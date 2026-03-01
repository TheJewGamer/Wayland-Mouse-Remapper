# Warning
Currently still working on this and I did use AI to help write this. I did review everything and rewrite basically everything though as claude wrote some garbage.
So basically this is now my garabage. Just with less loops that run every millisecond

# Building
gcc -o mouse-remap mouse-remap.c -lpthread $(pkg-config --libs --cflags dbus-1)

# Running
sudo DBUS_SESSION_BUS_ADDRESS=unix:path=/run/user/UserID/bus ./mouse-remap
