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
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/signal.h>
#include <sys/types.h>
#include <wayland-client.h>
#include "xdg-output-unstable-v1-client-protocol.h"

struct xwayback {
	struct wl_display *display;

	char *wayland_socket;
	char *X_display;

	struct zxdg_output_manager_v1 *xdg_output_manager;

	struct xway_output *first_output;
	struct wl_list outputs;
};

struct xway_output {
	struct wl_list link;
	struct wl_output *output;
	struct zxdg_output_v1 *xdg_output;
	struct xwayback *xwayback;
	uint32_t wl_name;

	char *name;
	char *description;

	char *make;
	char *model;

	uint32_t height, width, x, y;
	int32_t physical_height, physical_width;
	int32_t subpixel;
	int32_t transform;
	int32_t scale;
	float refresh;
};

pid_t comp_pid;
pid_t xway_pid;

static void output_geometry(void *data, struct wl_output *wl_output, int32_t x,
		int32_t y, int32_t width_mm, int32_t height_mm, int32_t subpixel,
		const char *make, const char *model, int32_t transform) {
	struct xway_output *output = data;

	free (output->make);
	free (output->model);

	output->physical_height = height_mm;
	output->physical_width = width_mm;
	output->make = make != NULL ? strdup(make) : NULL;
	output->model = model != NULL ? strdup(model) : NULL;
	output->subpixel = subpixel;
	output->transform = transform;
	return;
}

static void output_mode(void *data, struct wl_output *wl_output, uint32_t flags,
		int32_t width, int32_t height, int32_t refresh) {
	struct xway_output *output = data;
	output->refresh = (float)refresh / 1000;
	output->width = width;
	output->height = height;
	return;
}

static void output_done(void *data, struct wl_output *wl_output) {
	// No extra output processing needed
	return;
}

static void output_scale(void *data, struct wl_output *wl_output, int32_t factor) {
	struct xway_output *output = data;
	output->scale = factor;
	return;
}

static const struct wl_output_listener output_listener = {
	.geometry = output_geometry,
	.mode = output_mode,
	.done = output_done,
	.scale = output_scale,
};

static void xdg_output_handle_logical_position(void *data,
		struct zxdg_output_v1 *xdg_output, int32_t x, int32_t y) {
	struct xway_output *output = data;
	output->x = x;
	output->y = y;
}

static void xdg_output_handle_logical_size(void *data,
		struct zxdg_output_v1 *xdg_output, int32_t width, int32_t height) {
	struct xway_output *output = data;
	output->height = height;
	output->width = width;
}

static void xdg_output_handle_done(void *data,
		struct zxdg_output_v1 *xdg_output) {
	// No extra processing is currently required.
	return;
}

static void xdg_output_handle_name(void *data,
		struct zxdg_output_v1 *xdg_output, const char *name) {
	struct xway_output *output = data;
	free(output->name);
	output->name = strdup(name);
}

static void xdg_output_handle_description(void *data,
		struct zxdg_output_v1 *xdg_output, const char *description) {
	struct xway_output *output = data;
	free(output->description);
	output->description = strdup(description);
}

static const struct zxdg_output_v1_listener xdg_output_listener = {
	.logical_position = xdg_output_handle_logical_position,
	.logical_size = xdg_output_handle_logical_size,
	.done = xdg_output_handle_done,
	.name = xdg_output_handle_name,
	.description = xdg_output_handle_description,
};

static void add_xdg_output(struct xway_output *output) {
	if (output->xdg_output != NULL) {
		return;
	}

	output->xdg_output = zxdg_output_manager_v1_get_xdg_output(output->xwayback->xdg_output_manager, output->output);
	zxdg_output_v1_add_listener(output->xdg_output, &xdg_output_listener, output);
}

static void handle_global(void *data, struct wl_registry *registry,
		uint32_t name, const char *interface, uint32_t version) {
	struct xwayback *xwayback = data;
	if (strcmp(interface, wl_output_interface.name) == 0) {
		struct xway_output *output = calloc(1, sizeof(struct xway_output));
		output->xwayback = xwayback;
		output->output = wl_registry_bind(registry, name, &wl_output_interface, 3);
		wl_output_add_listener(output->output, &output_listener, output);
		output->wl_name = name;
		wl_list_init(&output->link);
		wl_list_insert(&xwayback->outputs, &output->link);
		if (xwayback->first_output == NULL)
			xwayback->first_output = output;
		if (xwayback->xdg_output_manager != NULL) {
			add_xdg_output(output);
		}
	} else if (strcmp(interface, zxdg_output_manager_v1_interface.name) == 0) {
		xwayback->xdg_output_manager = wl_registry_bind(registry, name, &zxdg_output_manager_v1_interface, 2);
	}
}

static const struct wl_registry_listener registry_listener = {
	.global = handle_global,
	.global_remove = NULL, // TODO: handle_global_remove
};

void handle_exit(int sig) {
	pid_t pid = waitpid(-1, NULL, WNOHANG);
	if (pid == comp_pid|| pid == xway_pid) {
		if (pid == comp_pid && xway_pid > 0)
			kill(xway_pid, SIGKILL);
		if (pid == xway_pid && comp_pid > 0)
			kill(comp_pid, SIGKILL);
		exit(EXIT_SUCCESS);
	}
}

__attribute__((noreturn)) static void usage(char *binname) {
	fprintf(stderr, "usage: %s [-d :display]\n", binname);
	exit(EXIT_SUCCESS);
}

int main(int argc, char* argv[]) {
	struct xwayback *xwayback = malloc(sizeof(struct xwayback));
	int socket_xwayback[2];
	int socket_xwayland[2];
	char buffer[BUFSIZ];
	const char *x_display = ":0";
	char *displayfd = NULL;
	int opt;

	signal(SIGCHLD, handle_exit);

	// TODO: Implement all options from Xserver(1) and Xorg(1)
	static struct option long_options[] = {
		{"display", required_argument, 0, 'd'},
		{"displayfd", required_argument, 0, 'f'},
		{0, 0, 0, 0}
	};

	int option_index = 0;
	while ((opt = getopt_long(argc, argv, "d:f:", long_options, &option_index)) != -1) {
		switch (opt) {
			case 'd':
				x_display = optarg;
				break;
			case 'f':
				displayfd = optarg;
				break;
			default:
				usage(argv[0]);
		}
	}

	// displayfd takes priority
	// TODO: Check if this is also the case in Xserver(7)
	if (displayfd != NULL) {
		printf("Xwayback: using displayfd\n");
		x_display = NULL;
	}

	if (socketpair(AF_UNIX, SOCK_STREAM, 0, socket_xwayback) == -1) {
		fprintf(stderr, "ERROR: unable to create xwayback socket\n");
		exit(EXIT_FAILURE);
	}

	if (socketpair(AF_UNIX, SOCK_STREAM, 0, socket_xwayland) == -1) {
		fprintf(stderr, "ERROR: unable to create xwayland socket\n");
		exit(EXIT_FAILURE);
	}

	comp_pid = fork();
	if (comp_pid == 0) {
		char fd_xwayback[64];
		char fd_xwayland[64];
		snprintf(fd_xwayback, sizeof(fd_xwayback), "%d", socket_xwayback[0]);
		snprintf(fd_xwayland, sizeof(fd_xwayland), "%d", socket_xwayland[0]);

		printf("Passed descriptor xback: %s; xway: %s\n", fd_xwayback, fd_xwayland);
		close(socket_xwayback[1]);
		close(socket_xwayland[1]);
		execlp("wayback-compositor", "wayback-compositor", fd_xwayback, fd_xwayland, (void *)NULL);
		fprintf(stderr, "ERROR: failed to launch wayback-compositor\n");
		exit(EXIT_FAILURE);
	}

	close(socket_xwayback[0]);
	close(socket_xwayland[0]);

	char way_display[1024];
	snprintf(way_display, sizeof(way_display), "%d", socket_xwayback[1]);
	setenv("WAYLAND_SOCKET", way_display, true);

	xwayback->wayland_socket = strdup(buffer);
	if (x_display)
		xwayback->X_display = strdup(x_display);

	xwayback->display = wl_display_connect(NULL);
	if (!xwayback->display) {
		fprintf(stderr, "ERROR: unable to connect to wayback-compositor.\n");
		exit(EXIT_FAILURE);
	}

	wl_list_init(&xwayback->outputs);
	xwayback->first_output = NULL;
	struct wl_registry *registry = wl_display_get_registry(xwayback->display);
	wl_registry_add_listener(registry, &registry_listener, xwayback);
	wl_display_roundtrip(xwayback->display);
	wl_display_roundtrip(xwayback->display); // xdg-output requires two roundtrips

	xway_pid = fork();
	if (xway_pid == 0) {
		char way_display[64];
		snprintf(way_display, sizeof(way_display), "%d", socket_xwayland[1]);
		setenv("WAYLAND_SOCKET", way_display, true);

		char geometry[4096];
		snprintf(geometry, sizeof(geometry), "%dx%d", xwayback->first_output->width, xwayback->first_output->height);

		if (x_display)
			execlp("Xwayland", "Xwayland", x_display, "-fullscreen", "-retro", "-geometry", geometry, (void*)NULL);
		else {
			execlp("Xwayland", "Xwayland", "-displayfd", displayfd, "-fullscreen", "-retro", "-geometry", geometry, (void*)NULL);
		}
		fprintf(stderr, "ERROR: failed to launch Xwayland\n");
		exit(EXIT_FAILURE);
	}

	while (1)
		pause();
	return 0;
}
