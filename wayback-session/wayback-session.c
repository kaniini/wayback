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

int main(int argc, char* argv[]) {
	char **session_cmd;
	char *xinitrc_path = NULL;
	signal(SIGCHLD, handle_exit);

	if (argc == 1) {
		xinitrc_path = get_xinitrc_path();
	} else {
		session_cmd = &argv[optind];
	}

	int fd[2];
	if (pipe(fd) == -1) {
		fprintf(stderr, "ERROR: Failed to create pipe\n");
		exit(EXIT_FAILURE);
	}

	xwayback_pid = fork();
	if (xwayback_pid == 0) {
		close(fd[0]);
		printf("Launching with fd %d\n", fd[1]);
		char fd_str[BUFSIZ];
		snprintf(fd_str, BUFSIZ, "%d", fd[1]);
		execlp("Xwayback", "Xwayback", "--displayfd", fd_str, (void*)NULL);
		fprintf(stderr, "ERROR: Failed to launch Xwayback\n");
		exit(EXIT_FAILURE);
	}

	char buffer[BUFSIZ-1];
	close(fd[1]);
	ssize_t n = read(fd[0], buffer, sizeof(buffer)-1);
	if (n > 0) {
		buffer[n-1] = '\0'; // Convert from newline-terminated to null-terminated string
		printf("Received display %s\n", buffer);
	}

	char x_display[BUFSIZ];
	snprintf(x_display, BUFSIZ, ":%s", buffer);

	session_pid = fork();
	if (session_pid == 0) {
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
