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
## Usage

To run an X11 desktop environment/window manager inside Wayback, simply use the command `wayback <desktop environment>`

To simplify this, you can create a Session file, which you can choose when you log in, like you would choose any other desktop environment.  
For this example, I will be using BSPWM.

1. Create a session file in `/usr/share/wayland-sessions/`, I'll call it `bspwm-wayland.desktop`. Make sure it has the `.desktop` extension. For example, `sudo touch /usr/share/wayland-sessions/bspwm-wayland.desktop`
2. Open the file with your favourite text editor (make sure you run the editor as `sudo`, else you won't have the permissions.)
3. Inside the file, write the following:
```ini
[Desktop Entry]
Name=bspwm (Wayland)
Exec=wayback bspwm
Type=application
```
4. Replace the `Name` field with your WM/DE's name, and the `Exec` field with `Exec=wayback <your window manager/desktop>`
5. Mark the file as executable with `sudo chmod +x /usr/share/wayland-sessions/<your_wm>.desktop`
6. Restart your display manager (or just reboot) and select the session!

## Discussion

irc.libera.chat #wayback
