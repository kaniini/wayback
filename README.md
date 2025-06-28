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
