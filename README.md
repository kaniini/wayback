# wayback

Wayback is an experimental X compatibility layer which allows for running
full X desktop environments using Wayland components.  It is essentially
a stub compositor which provides just enough Wayland capabilities to host
a rootful Xwayland server.

It is intended to eventually replace the classic X.org server in Alpine,
thus reducing maintenance burden of X applications in Alpine, but a lot
of work needs to be done first.

Wayback is an experimental state: expect breaking changes, and lots of
bugs.  Please submit pull requests fixing bugs instead of bug reports
if you are able.

## Installation

Dependencies:
- wayland (wayland-server, wayland-client, wayland-cursor, wayland-egl)
- wayland-protocol >=1.14
- xkbcommon
- wlroots-0.19

Building:
```
meson setup _build
cd _build
meson compile
```

Installing:
```
meson install
```

## Testing / Usage

Run `Xwayback xterm` or `Xwayback startplasma-x11`. Exit with <key>Ctrl</key>+<key>Alt</key>+<key>Backspace</key>, or by terminating every client.

If you're running `Xwayback` out of the build directory, wake sure `wayland-compositor` is on your PATH.

If you're running wayback from an existing desktop session, allocate a new display number with the `-d :1` option.

## Discussion

irc.libera.chat #wayback
