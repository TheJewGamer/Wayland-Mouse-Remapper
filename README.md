# What is this
I recently switched to linux and needed a solution to program the side buttons on my Razer mouse. OpenRazer and other software did not work for me so I decided to create my own thing. This is that.

# Features
- Rebind mouse buttons on per window/application bias.
- Automatic profile switching based on active window
    - Mouse input does not stop while switching profiles either
- No noticeable delay caused to input because of the program
- Extra buttom mapping per configuration via layers (layershift)
- Allow rebinding of mouse scroll left and scroll right

# Available Keys
The list of keys that can be used can be seen in the bindings.c file.

# Configuration
Configurations files are stored at ~/.config/wayland-mouse-remapper/ and should be the name of the app/window you want to remap.

## Layershift
Layershift is an optional feature that you can use to have a second layer of mouse bindings. You can use layershift by assgined a key to one of the following triggers:
- ```LAYER_SHIFT_HOLD```
    - Enables layershift when held down
- ```LAYER_SHIFT_TOGGLE```
    - Toggles layershift on and off each time its pressed

Lastly use: ```[layershift]``` to define your layershift mapping section. Anything under this header will be read as layershift mappings. 

Note that when in layershift any unmapped key will have its normal mapping used even if it was mapped outside of of the layershift section.

# Requirements
- Linux
    - I used CachyOS so any arch derivative should work
- Wayland
- KDE Plasama
    - Spefically built on Plasma 6

## Other Window Managers
You can use this without KDE but the automatic profile switching will not work.

### Patching for other Window Mangers
You should be able to patch this program to work with other window manager's very easily as all you need to do is send the window name to the dbus. If people actually use this feel free to make a feature request and I will see what I can do.

# Building
- Copy the repo, extract it and run the setup.sh file. This handles the setup and can also be used to update the binary if needed.

# DBUS Commands
The script uses dbus for communication between the KWIN script and the acutal program. You can also send commands via this to set settings. 

## Script Commands
- Set Persistent Mode on or off. This prevents the program from switching profiles based on the current active window.
    - Enable
        - ```dbus-send --session --type=method_call --dest=org.mouse.remap /org/mouse/remap org.mouse.remap.SetPersistentMode boolean:true```
    - Disable
        - ```dbus-send --session --type=method_call --dest=org.mouse.remap /org/mouse/remap org.mouse.remap.SetPersistentMode boolean:false```
- Set current Configuration file to use. Note that if you have persitent mode off and are using the KWIN script this will get overwritten
    - ```dbus-send --session --type=method_call --dest=org.mouse.remap /org/mouse/remap org.mouse.remap.SetConfig string:"app"```
- Exit the program
    - ```dbus-send --session --type=method_call --dest=org.mouse.remap /org/mouse/remap org.mouse.remap.Stop```

## Logs
- You can view logs by using: ```journalctl --user -u wayland-mouse-remapper -f```
    - Note that logs are turned off by default expect on errors. You can enable them by setting the DEBUG variables in the vars.h file to 1. Then run the setup.sh file
- You can view the status of the program via: ```systemctl --user status wayland-mouse-remapper```
- You can view the status of the kwin script via: ```kreadconfig6 --file kwinrc --group Plugins --key "wayland-mouse-remapper-window-notifierEnabled"```

# Future Plans
Might make a simple UI to assist in creating configuration files. We shall see.

# LLM (AI) Notice
Fair warning cause some people care. I did use a(n) LLM/AI to help me program this. However I did review everything and wrote a fair bit of my own code. I would say 90% is my code along with 10% left over from the LLM/AI.

Either way its solid enough for me to use.