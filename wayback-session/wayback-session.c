/*
 * wayback-session launches the wayback compositor, XWayback/XWayland
 * and the given session executable
 *
 * SPDX-License-Identifier: MIT
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <getopt.h>
#include <unistd.h>
#include <dirent.h>
#include <ctype.h>
#include <sys/wait.h>
#include <sys/signal.h>

pid_t xwayback_pid;
pid_t session_pid;

int get_next_display() {
	const char *x11_socket_path = "/tmp/.X11-unix/";
	DIR *x11_socket_dir = opendir(x11_socket_path);
	struct dirent *x11_socket;
	int max_display = -1;

	if (!x11_socket_dir) {
		fprintf(stderr, "ERROR: failed to open /tmp/.X11-unix\n");
		return -1;
	}

	while ((x11_socket = readdir(x11_socket_dir)) != NULL) {
		if (x11_socket->d_name[0] == 'X') {
			char *num_str = x11_socket->d_name + 1;
			int valid = 1;
			for (int i = 0; num_str[i] != '\0'; ++i) {
				if (!isdigit((unsigned char)num_str[i])) {
					valid = 0;
					break;
				}
			}
			if (valid && strlen(num_str) > 0) {
				int display = atoi(num_str);
				if (display > max_display) {
					max_display = display;
				}
			}
		}
	}
	closedir(x11_socket_dir);

	return(max_display + 1);
}

char *get_xinitrc_path() {
	char *home_dir = getenv("HOME");
	char *xinitrc_path = malloc(strlen(home_dir)+strlen("/.xinitrc")+1);
	snprintf(xinitrc_path, strlen(home_dir)+strlen("/.xinitrc")+1, "%s/.xinitrc", home_dir);
	if (access(xinitrc_path, F_OK) == 0) {
		return xinitrc_path;
	}

	free(xinitrc_path);
	xinitrc_path = strdup("/etc/X11/xinit/xinitrc");
	if (access(xinitrc_path, F_OK) == 0) {
		return xinitrc_path;
	}

	free(xinitrc_path);
	fprintf(stderr, "ERROR: Unable to find xinitrc file.\n");
	exit(EXIT_FAILURE);
}

void handle_exit(int sig) {
	pid_t pid = waitpid(-1, NULL, WNOHANG);
	if (pid == session_pid|| pid == xwayback_pid) {
		if (pid == session_pid && xwayback_pid > 0)
			kill(xwayback_pid, SIGKILL);
		if (pid == xwayback_pid && session_pid > 0)
			kill(session_pid, SIGKILL);
		exit(EXIT_SUCCESS);
	}
}


__attribute__((noreturn)) static void usage(void) {
	printf("usage: wayback [-d :display] [-- <session launcher>]\n");
	exit(EXIT_SUCCESS);
}

int main(int argc, char* argv[]) {
	char **session_cmd;
	char *xinitrc_path = NULL;
	signal(SIGCHLD, handle_exit);
	char *x_display = NULL;
	int display_num;
	int opt;

	// TODO: Implement startx(1) compatibility
	static struct option long_options[] = {
		{"display", required_argument, 0, 'd'},
		{0, 0, 0, 0}
	};

	int option_index = 0;
	while ((opt = getopt_long(argc, argv, "d:", long_options, &option_index)) != -1) {
		switch (opt) {
			case 'd':
				x_display = optarg;
				display_num = atoi(optarg+1);
				break;
			default:
				usage();
		}
	}

	if (optind >= argc) {
		xinitrc_path = get_xinitrc_path();
	} else {
		session_cmd = &argv[optind];
	}

	if (x_display == NULL) {
		display_num = get_next_display();
		if (display_num == -1) {
			fprintf(stderr, "INFO: Unable to get next open display, assuming :0\n");
			display_num = 0;
		}

		char display[BUFSIZ];
		snprintf(display, BUFSIZ, ":%d", display_num);
		x_display = strdup(display);
	}

	xwayback_pid = fork();
	if (xwayback_pid == 0) {
		printf("Launching on display %s\n", x_display);
		execlp("Xwayback", "Xwayback", "--display", x_display, (void*)NULL);
		fprintf(stderr, "ERROR: Failed to launch Xwayback\n");
		exit(EXIT_FAILURE);
	}

	session_pid = fork();
	if (session_pid == 0) {
		char x11_socket_path[BUFSIZ];
		snprintf(x11_socket_path, BUFSIZ, "/tmp/.X11-unix/X%d", display_num);
		while (access(x11_socket_path, F_OK) != 0) {
			usleep(1000);
		}

		setenv("XDG_SESSION_TYPE", "x11", true);
		setenv("WAYLAND_DISPLAY", "", true);
		setenv("DISPLAY", x_display, true);
		if (xinitrc_path != NULL) {
			execlp("sh", "sh", xinitrc_path, (void*)NULL);
		} else if (session_cmd != NULL) {
			execvp(session_cmd[0], session_cmd);
		}
		fprintf(stderr, "ERROR: Failed to launch session\n");
		free(xinitrc_path);
		exit(EXIT_FAILURE);
	}

	while(1)
		pause();
	free(xinitrc_path);
	return 0;
}
