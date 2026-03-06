# Warning
Currently still working on this and I did use AI to help write this. I did review everything and rewrite basically everything though as claude wrote some garbage.
So basically this is now my garabage. Just with less loops that run every millisecond

# Building
gcc -o mouse-remap mouse-remap.c -lpthread $(pkg-config --libs --cflags dbus-1)

# Running
sudo DBUS_SESSION_BUS_ADDRESS=unix:path=/run/user/1000/bus ./mouse-remap

# DBUS commands
dbus-send --session --type=method_call --dest=org.mouse.remap /org/mouse/remap org.mouse.remap.SetPersistentMode boolean:true
dbus-send --session --type=method_call --dest=org.mouse.remap /org/mouse/remap org.mouse.remap.SetConfig string:"app"