project('wayback', 'c',
  version: '0.1',
  license: 'MIT',
  meson_version: '>= 0.50')

add_project_arguments('-DWLR_USE_UNSTABLE', language: 'c')

cc = meson.get_compiler('c')

wayland_server = dependency('wayland-server')
wayland_client = dependency('wayland-client')
wayland_cursor = dependency('wayland-cursor')
wayland_egl    = dependency('wayland-egl')
wayland_protos = dependency('wayland-protocols', version: '>=1.14')
xkbcommon      = dependency('xkbcommon')
wlroots        = dependency('wlroots-0.19')

subdir('protocol')

executable(
	'wayback',
	['wayback.c'],
	dependencies: [wayland_server, wayland_client, wayland_cursor, wayland_egl, wayland_protos, wlroots, xkbcommon, server_protos],
	install: true,
)
