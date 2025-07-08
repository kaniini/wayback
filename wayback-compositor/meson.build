executable(
	'wayback-compositor',
	['wayback-compositor.c'],
	dependencies: [wayland_server, wayland_client, wayland_cursor, wayland_egl, wayland_protos, wlroots, xkbcommon, server_protos],
	install: true,
	install_dir: get_option('libexecdir'),
)
