#
# Author: TheJewGamer
# Last Update: 3/8/2026
#

#!/bin/bash

# colours for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

# vars
BINARY_NAME="wayland-mouse-remapper"
BINARY_PATH="/usr/local/bin/$BINARY_NAME"
CONFIG_DIR="$HOME/.config/wayland-mouse-remapper"
SERVICE_DIR="$HOME/.config/systemd/user"
UDEV_RULES_PATH="/etc/udev/rules.d/99-wayland-mouse-remapper.rules"
USER_ID=$(id -u)
KWIN_SCRIPT_DIR="$HOME/.local/share/kwin/scripts"
KWIN_SCRIPT_NAME="wayland-mouse-remapper-window-notifier"

# -----------------------------------------------
# helper functions
# -----------------------------------------------

log_info() { echo -e "${GREEN}[INFO]${NC} $1"; }
log_warn() { echo -e "${YELLOW}[WARN]${NC} $1"; }
log_error() { echo -e "${RED}[ERROR]${NC} $1"; }


# Welcome message
echo "==============================="
echo " Wayland Mouse Remapper Setup"
echo " By      TheJewGamer"
echo "==============================="
echo ""

#feedback
echo "Welcome to the Wayland Mouse Remapper setup script. This script will do the following:"
echo "1. Create the wayland-mouse-reampper binary file"
echo "2. Copy the binary file to the install directory (/usr/local/bin/)"
echo "3. Setup needed udev rules for the script if not already present"
echo "4. Setup the configuration directory if not already present"
echo "5. Create a default.config if not already present"
echo "6. Create a settings.ini file if not already present"
echo "7. Create the user systemd service if not already present"
echo "8. Installs the KWIN script that is used to notify of windows changes if on KDE"
echo "Please note this script will request your password to run as sudo."
read -rp "Proceed? (Y/n): " CONFIRM
if [[ ! $CONFIRM =~ ^[Yy]$ ]]; then
    log_info "Said no. Canceling setup"
    exit 0
fi

#new line
echo ""

# -----------------------------------------------
# create binary file
# -----------------------------------------------

# feedback
log_info "Creating binary"

#run make command
make

#check to see if binary was created
if [ $? -ne 0 ]; then

    #logging
    log_error "Build failed"

    #end script
    exit 1
fi

#logging
log_info "Binary built successfully"

#new line
echo ""

# -----------------------------------------------
# install binary
# -----------------------------------------------

#logging
log_info "Installing binary to $BINARY_PATH"

# stop service if running before installing binary
if systemctl --user is-active --quiet wayland-mouse-remapper; then
    #feedback
    log_info "Stopping existing wayland-mouse-remapper service"
    
    systemctl --user stop wayland-mouse-remapper
fi

#copy binary to install location
sudo cp "bin/$BINARY_NAME" "$BINARY_PATH"

#confirm binary was copied correctly
if [ $? -ne 0 ]; then
    
    #logging
    log_error "Failed to install binary."
    
    #end script
    exit 1
fi

#logging
log_info "Binary installed"

#new line
echo ""

# -----------------------------------------------
# create udev rules
# -----------------------------------------------

#feedback
log_info "Checking if udev rules file is present"

#check to see if udev rule already exists
if sudo test -f "$UDEV_RULES_PATH"; then
    log_info "udev rules already present"
else
    log_info "udev rules not present creating udev rules at $UDEV_RULES_PATH"

    # prompt user for mouse vendor and product ID
    echo ""
    echo "Getting input devices."
    echo ""

    # get list of input devices by name along with phys path
    mapfile -t DEVICE_NAMES < <(cat /proc/bus/input/devices | grep "^N: Name=" | sed 's/N: Name=//' | tr -d '"')
    mapfile -t VENDOR_IDS < <(cat /proc/bus/input/devices | grep "^I:" | sed 's/.*Vendor=\([0-9a-f]*\).*/\1/')
    mapfile -t PRODUCT_IDS < <(cat /proc/bus/input/devices | grep "^I:" | sed 's/.*Product=\([0-9a-f]*\).*/\1/')
    mapfile -t PHYS_PATHS < <(cat /proc/bus/input/devices | grep "^P: Phys=" | sed 's/P: Phys=//')

    #feedback
    echo "Available input devices:"
    echo ""

    # print all found devices
    for i in "${!DEVICE_NAMES[@]}"; do
        echo "  $((i+1))) ${DEVICE_NAMES[$i]} / ${PHYS_PATHS[$i]}"
    done
    echo ""

    # prompt for input
    read -p "Select your mouse (1-${#DEVICE_NAMES[@]}): " SELECTION
    SELECTION=$((SELECTION-1))

    # confirm valid input
    if [ $SELECTION -lt 0 ] || [ $SELECTION -ge ${#DEVICE_NAMES[@]} ]; then
        #logging
        log_error "Invalid selection"
        
        #end script
        exit 1
    fi

    # update vars
    VENDOR_ID="${VENDOR_IDS[$SELECTION]}"
    PRODUCT_ID="${PRODUCT_IDS[$SELECTION]}"
    MOUSE_NAME="${DEVICE_NAMES[$SELECTION]}"

    #feedback
    log_info "Selected: $MOUSE_NAME (Vendor: $VENDOR_ID Product: $PRODUCT_ID)"
    echo ""

    # create udev file
    sudo tee "$UDEV_RULES_PATH" > /dev/null << EOF
KERNEL=="uinput", MODE="0660", TAG+="uaccess"
SUBSYSTEM=="input", ATTRS{idVendor}=="$VENDOR_ID", ATTRS{idProduct}=="$PRODUCT_ID", MODE="0660", TAG+="uaccess"
SUBSYSTEM=="usb", ATTRS{idVendor}=="$VENDOR_ID", ATTRS{idProduct}=="$PRODUCT_ID", MODE="0660", TAG+="uaccess"
EOF

    # check to see if udev file was created correctly 
    if [ $? -ne 0 ]; then

        #feedback
        log_error "Failed to create udev rules."
        
        #emd script
        exit 1
    fi
    #feedback
    log_info "udev rules created"

    # reload udev rules
    log_info "Reloading udev rules"
    sudo udevadm control --reload-rules && sudo udevadm trigger
    log_info "udev rules reloaded"

fi

# new line
echo ""

# -----------------------------------------------
# create config directory
# -----------------------------------------------

#feedback
log_info "checking if config directory is present or not"

#check to see if config directory exists or not
if [ -d "$CONFIG_DIR" ]; then
    #feedback
    log_info "Config directory already exists"
#config directory does not exist
else
    #logging
    log_info "Creating config directory at $CONFIG_DIR"
    
    #create directory
    mkdir -p "$CONFIG_DIR"

    #logging
    log_info "Config directory created"
fi

# new line
echo ""

# -----------------------------------------------
# create default.conf if it doesn't exist
# -----------------------------------------------

#feedback
log_info "Checking to see if default configuration file is present"

#check to see if default configuration is present or not
if [ ! -f "$CONFIG_DIR/default.conf" ]; then

    #feedback
    log_info "Creating default configuration file"
    
    #create default configuration
    cat > "$CONFIG_DIR/default.conf" << EOF
# wayland-mouse-remapper default configuration example

# format: FROM_BUTTON=TO_KEY

# example mappings
BTN_SIDE=KEY_Q
BTN_EXTRA=KEY_E

# horizontal scroll mappings
REL_HWHEEL_LEFT=KEY_LEFT
REL_HWHEEL_RIGHT=KEY_RIGHT

# macro actions
# BTN_SIDE=COPY_MACRO
# BTN_EXTRA=PASTE_MACRO

# layer shift - use one or the other
# LAYER_SHIFT_TOGGLE=BTN_SIDE
# LAYER_SHIFT_HOLD=BTN_EXTRA

# layer shift mappings
# [layershift]
# BTN_SIDE=KEY_E
EOF

    #feedback
    log_info "default configuration file created"

#default configuration file already exists 
else
    #feedback
    log_info "default configuration file already present"
fi

#new line
echo ""

# -----------------------------------------------
# create settings.ini
# -----------------------------------------------

#feedback
log_info "checking to see if settings file is present"

# check to see if settings files already exists
if [ ! -f "$CONFIG_DIR/settings.ini" ]; then

    #feedback
    log_info "Creating settings file"

    # get list of input devices by name
    mapfile -t DEVICE_NAMES < <(cat /proc/bus/input/devices | grep "^N: Name=" | sed 's/N: Name=//' | tr -d '"')
    mapfile -t VENDOR_IDS < <(cat /proc/bus/input/devices | grep "^I:" | sed 's/.*Vendor=\([0-9a-f]*\).*/\1/')
    mapfile -t PRODUCT_IDS < <(cat /proc/bus/input/devices | grep "^I:" | sed 's/.*Product=\([0-9a-f]*\).*/\1/')
    mapfile -t PHYS_PATHS < <(cat /proc/bus/input/devices | grep "^P: Phys=" | sed 's/P: Phys=//')

    #feedback
    echo "Available input devices:"
    echo ""

    # print all found devices
    for i in "${!DEVICE_NAMES[@]}"; do
        echo "  $((i+1))) ${DEVICE_NAMES[$i]} / ${PHYS_PATHS[$i]}"
    done
    echo ""

    # prompt for input
    read -p "Select your mouse (1-${#DEVICE_NAMES[@]}): " SELECTION
    SELECTION=$((SELECTION-1))

    # confirm valid input
    if [ $SELECTION -lt 0 ] || [ $SELECTION -ge ${#DEVICE_NAMES[@]} ]; then
        #logging
        log_error "Invalid selection"
        
        #end script
        exit 1
    fi

    #update mouse phys var
    MOUSE_PHYS="${PHYS_PATHS[$SELECTION]}"

    #prompt for keyboard device
    echo ""
    echo "Select the keyboard interface of your mouse (for extra buttons):"
    echo "Leave blank if not needed"
    echo ""

    #list all input devices
    for i in "${!DEVICE_NAMES[@]}"; do
        # skip already selected mouse device
        if [ $i -eq $SELECTION ]; then
            continue
        fi
        echo "  $((i+1))) ${DEVICE_NAMES[$i]} / ${PHYS_PATHS[$i]}"
    done

    #new line
    echo ""

    # get input
    read -rp "Select keyboard device (1-${#DEVICE_NAMES[@]}) or press enter to skip: " KB_SELECTION

    #check to see if input is empty
    if [ -z "$KB_SELECTION" ]; then
        #just set as nothing as skipped
        MOUSE_KEYBOARD_PHYS=""
    #not empty input
    else
        #get actually array value
        KB_SELECTION=$((KB_SELECTION-1))

        # confirm valid input and not same as previous selection
        if [ $KB_SELECTION -lt 0 ] || [ $KB_SELECTION -ge ${#DEVICE_NAMES[@]} ] || [ $KB_SELECTION -eq $SELECTION ]; then
            log_error "Invalid selection"
            exit 1
        fi
        MOUSE_KEYBOARD_PHYS="${PHYS_PATHS[$KB_SELECTION]}"
    fi

    #create file
    cat > "$CONFIG_DIR/settings.ini" << EOF
MOUSE_PHYS=$MOUSE_PHYS
MOUSE_KEYBOARD_PHYS=$MOUSE_KEYBOARD_PHYS
PERSISTENT_MODE=0
PERSISTENT_PROFILE=default
EOF

    #feedback
    log_info "settings file created"

# settings file already exists
else
    #feedback
    log_info "settings file already present"
fi

#new line
echo ""

# -----------------------------------------------
# create systemd user service
# -----------------------------------------------

#feedback
log_info "Checking for systemd service"

#check to see if service already exists
if [ -f "$SERVICE_DIR/wayland-mouse-remapper.service" ]; then
    #feedback
    log_info "Systemd service already exists"
else

    #feedback
    log_info "Creating systemd user service"

    #create service file
    cat > "$SERVICE_DIR/wayland-mouse-remapper.service" << EOF
[Unit]
Description=Wayland Mouse Remapper
After=graphical-session.target

[Service]
Type=simple
ExecStart=$BINARY_PATH
Environment=DBUS_SESSION_BUS_ADDRESS=unix:path=/run/user/$USER_ID/bus

[Install]
WantedBy=graphical-session.target
EOF

    #feedback
    log_info "Systemd user service created"

    # enable and start service
    log_info "Enabling and starting service"
    systemctl --user daemon-reload
    systemctl --user enable wayland-mouse-remapper
    systemctl --user start wayland-mouse-remapper

    # check to see if service started
    if systemctl --user is-active --quiet wayland-mouse-remapper; then

        #feedback
        log_info "Service started successfully"
    
    # service did not start
    else
        #feedback
        log_error "Service failed to start. Check logs with: journalctl --user -u wayland-mouse-remapper"
        
        # end program
        exit 1
    fi
fi

# new line
echo ""

# -----------------------------------------------
# create KWIN script
# -----------------------------------------------

log_info "Trying to install KWIN script"

#check to see if on KDE
if [ "$XDG_CURRENT_DESKTOP" == "KDE" ]; then

    #check to see if kwin script is already installed
    if [ -d "$KWIN_SCRIPT_DIR/$KWIN_SCRIPT_NAME" ]; then
        # feedback
        log_info "KWin script already installed"

    # not installed
    else
        #feedback
        log_info "Installing KWin script..."
        
        #copy folder to kwin scripts location
        cp -a $KWIN_SCRIPT_NAME "$KWIN_SCRIPT_DIR/$KWIN_SCRIPT_NAME/"
        
        # check to see if folder now exists
        if [ $? -ne 0 ]; then
            #feedback
            log_error "Failed to install KWin script"
            #end script
            exit 1
        fi

        #feedback
        log_info "KWin script installed"
    fi
# not on KDE
else
    #feedback
    log_info "Not on KDE skipping KWIN script install"
fi

# done
echo ""
echo "==============================="
echo -e "${GREEN} Setup complete${NC}"
echo "==============================="
echo ""
echo "Config directory: $CONFIG_DIR"
ehco "Confirm program is running via: systemctl --user status wayland-mouse-remapper"
echo ""