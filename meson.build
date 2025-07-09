project('wayback', 'c',
  version: '0.1',
  license: 'MIT',
  meson_version: '>= 0.55',
  default_options: [
    'c_std=gnu11',
  ])

add_project_arguments('-DWLR_USE_UNSTABLE', language: 'c')

add_global_arguments('-DWAYBACK_COMPOSITOR_EXEC_PATH="@0@/wayback-compositor"'.format(get_option('prefix') /get_option('libexecdir')), language : 'c')

wayland_server = dependency('wayland-server')
wayland_client = dependency('wayland-client')
wayland_cursor = dependency('wayland-cursor')
wayland_egl    = dependency('wayland-egl')
wayland_protos = dependency('wayland-protocols', version: '>=1.14')
xkbcommon      = dependency('xkbcommon')
xwayland       = dependency('xwayland', version: '>=24.1')
wlroots        = dependency('wlroots-0.19', version: '>=0.19', required: false)

if not wlroots.found()
  wlroots = dependency('wlroots-0.18', version: '>=0.18')
endif

xwayland_executable_path = xwayland.get_variable(pkgconfig: 'xwayland')
add_global_arguments('-DXWAYLAND_EXEC_PATH="@0@"'.format(xwayland_executable_path), language : 'c')

subdir('protocol')
subdir('wayback-compositor')
subdir('wayback-session')
subdir('xwayback')
subdir('doc')
