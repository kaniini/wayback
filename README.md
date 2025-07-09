# ARCHIVED AND MOVED TO FREEDESKTOP GITLAB

This project has moved to: https://gitlab.freedesktop.org/wayback/wayback

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
- xwayland >= 24.1

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

## Discussion

irc.libera.chat #wayback
