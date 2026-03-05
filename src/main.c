/* 
Author: TheJewGamer
Last Update: 3/5/2026
*/

//includes
#include <stdio.h> //printing
#include <stdlib.h> //exit/free
#include <string.h> //strings
#include <fcntl.h> //reading/open
#include <unistd.h> //file reading
#include <dirent.h> //directory reading
#include <pwd.h> //user info
#include <grp.h> //groups
#include <time.h> //timing stuff
#include <dbus/dbus.h> //dbus
#include <linux/uinput.h> //input

//file imports
#include "../headers/vars.h"
#include "../headers/config.h"
#include "../headers/helpers.h"
#include "../headers/inputHandlers.h"

//get mouse event id by name. Needed as event ids can change on reboot
char* getMouseEventID(const char *mouseName)
{
    //var
    static char mouseFilePath[64];

    //open input devices file
    FILE *inputDevicesFile = fopen("/proc/bus/input/devices", "r");

    //confirm that file was opened successfully
    if (!inputDevicesFile)
    { 
        //logging
        perror("open /proc/bus/input/devices"); 
        
        //stop script
        exit(1); 
    }

    //vars
    char currentLine[256];
    int foundMouse = 0; //bool

    //loop through each line in file
    while (fgets(currentLine, sizeof(currentLine), inputDevicesFile))
    {
        //check to see if current line contains mouseName
        if (strstr(currentLine, mouseName))
        {
            //it did update var
            foundMouse = 1;
        }

        //check to see if we found the mouseName line
        if (foundMouse)
        {
            //find the handler line
            if(strstr(currentLine, "Handlers="))
            {
                //find the event line and extract device path. IE: /dev/input/eventX
                char *event = strstr(currentLine, "event");

                //var to hold event number
                int eventNum;
                
                //extract number after the word 'event'
                sscanf(event, "event%d", &eventNum);

                //build full device path
                snprintf(mouseFilePath, sizeof(mouseFilePath), "/dev/input/event%d", eventNum);
                
                //close device file
                fclose(inputDevicesFile);

                //return device path
                return mouseFilePath;
            }
        }
    }

    //Did not find mouse based on name
    fclose(inputDevicesFile); //close file
    fprintf(stderr, "ERROR: could not find mouse device: %s\n", mouseName); //Loggins
    exit(1); //stop script
}

//dbus listener method for KWIN script
void *windowListener(void *arg) //Note arg is needed here despite not being used.
{
    //vars
    DBusError err;
    dbus_error_init(&err);

    //connect to user dbus session
    DBusConnection *windowListenerConnection = dbus_bus_get(DBUS_BUS_SESSION, &err);

    //check to see if connection is successful
    if (dbus_error_is_set(&err)) 
    {
        //errror on dbus connection

        //logging
        fprintf(stderr, "dbus connection error: %s\n", err.message);
        dbus_error_free(&err);
        return NULL;
    }

    //register custom dbus service
    dbus_bus_request_name(windowListenerConnection, DBUS_SERVICE, DBUS_NAME_FLAG_REPLACE_EXISTING, &err);
    
    //check to see if registeration was successful
    if (dbus_error_is_set(&err)) 
    {
        //error

        //logging
        fprintf(stderr, "dbus name error: %s\n", err.message);
        dbus_error_free(&err);
        return NULL;
    }

    //infinite loop but sleeps until dbus gets a message. Will exit if dbus connection is lost though
    while (dbus_connection_read_write_dispatch(windowListenerConnection, -1)) 
    {
        //get first message from the dbus by popping it
        DBusMessage *windowListenerInput = dbus_connection_pop_message(windowListenerConnection);

        //check to see if message was returned (needed to prevent crash in extreme cases)
        if (!windowListenerInput) 
        {
            continue;
        }

        //confirm message is on the correct interface
        if (dbus_message_is_method_call(windowListenerInput, DBUS_IFACE, "SetApp")) 
        {
            //var
            const char *appName = NULL;

            //extract data from dbus message
            if (dbus_message_get_args(windowListenerInput, &err, DBUS_TYPE_STRING, &appName, DBUS_TYPE_INVALID)) 
            {
                //logging
                printf("switching to app: %s\n", appName);

                //lock mapping array when changing things
                pthread_mutex_lock(&mappings_mutex);
                
                //load the new configuration
                freeMappings();
                loadConfig(appName);

                //unlock the mapping array as done
                pthread_mutex_unlock(&mappings_mutex);
            }
        }
        //remove the current message garabge collection
        dbus_message_unref(windowListenerInput);
    }

    return NULL;
}

// sets up the virutal mouse
int setupVirtualMouse() 
{
    //vars for virtual mouse device
    int uinputFile = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    if (uinputFile < 0)
    { 
        //logging ERROR
        fprintf(stderr, "could not open /dev/uinput\n");
        
        //stop script
        exit(1); 
    }

    //register event types
    ioctl(uinputFile, UI_SET_EVBIT, EV_KEY); //button presses
    ioctl(uinputFile, UI_SET_EVBIT, EV_REL); //movement
    ioctl(uinputFile, UI_SET_EVBIT, EV_SYN); //sync events

    //register standard mouse buttons
    ioctl(uinputFile, UI_SET_KEYBIT, BTN_LEFT);
    ioctl(uinputFile, UI_SET_KEYBIT, BTN_RIGHT);
    ioctl(uinputFile, UI_SET_KEYBIT, BTN_MIDDLE);
    ioctl(uinputFile, UI_SET_KEYBIT, BTN_SIDE);
    ioctl(uinputFile, UI_SET_KEYBIT, BTN_EXTRA);

    //register all keyboard keys
    for (int i = KEY_ESC; i < KEY_MAX; i++)
    {
        //add key
        ioctl(uinputFile, UI_SET_KEYBIT, i);
    }

    //register mouse wheel and relvant axies
    ioctl(uinputFile, UI_SET_RELBIT, REL_X);
    ioctl(uinputFile, UI_SET_RELBIT, REL_Y);
    ioctl(uinputFile, UI_SET_RELBIT, REL_WHEEL);
    ioctl(uinputFile, UI_SET_RELBIT, REL_HWHEEL);
    
    //set as virtual pointer device
    ioctl(uinputFile, UI_SET_PROPBIT, INPUT_PROP_POINTER);

    //create virtual mouse
    struct uinput_setup usetup = {0};
    usetup.id.bustype = BUS_VIRTUAL;
    usetup.id.vendor  = 0x1234;
    usetup.id.product = 0x5678;
    strcpy(usetup.name, "virtual-mouse");

    ioctl(uinputFile, UI_DEV_SETUP, &usetup);
    ioctl(uinputFile, UI_DEV_CREATE);

    //done
    return uinputFile;
}

//main
int main() 
{
    //inital preq check
    setupCheck();

    //load default config
    loadConfig("default");

    //open/grab mouse as root
    char *mouseDevice = getMouseEventID(MOUSE_NAME); //get mouse by name
    printf("found mouse at %s\n", mouseDevice); //logging
    int mouseDeviceFile = open(mouseDevice, O_RDONLY); //open mouse
    
    //confirm mouse was opened correctly
    if (mouseDeviceFile < 0) 
    { 
        //logging
        fprintf(stderr, "ERROR: Could not access mouse. Exiting\n");
        exit(1);
    } 
    //grab the mouse and prevent events from going to the actually device. (Prevents issue with double input)
    if (ioctl(mouseDeviceFile, EVIOCGRAB, 1) < 0) 
    {
        fprintf(stderr, "ERROR: Could not grab mouse. Exiting\n");
        exit(1); 
    }

    //set up virtual device as root
    int virtualMouse = setupVirtualMouse();

    //drop privileges from sudo to real user
    dropPrivileges();

    //start dubs thread for window listening as  user
    pthread_t windoListenerTid;
    pthread_create(&windoListenerTid, NULL, windowListener, NULL);

    //holds incoming input
    struct input_event ev;

    //read input from actual mouse
    while (read(mouseDeviceFile, &ev, sizeof(ev)) == sizeof(ev)) 
    {
        //var for checking if current key is remapped or not
        int remapped = 0;

        //check to see if layershift toggle button is pressed
        if (ev.type == EV_KEY && ev.code == layer_toggle_button)
        {
            //key down only
            if (ev.value == 1)
            {
                //invert var
                layerShiftActive = !layerShiftActive;
            }
            
            //end this loop run
            continue;
        }

        //check to see if layershift hold button is pressed
        if (ev.type == EV_KEY && ev.code == layer_hold_button)
        {
            //active while held
            layerShiftActive = ev.value;

            //end this loop run here
            continue;
        }

        //lock so configuration cannot be swapped
        pthread_mutex_lock(&mappings_mutex);

        //loop through all active mappings
        for (int i = 0; i < mapping_count; i++) 
        {
            //var
            struct mapping *currentMapping = &mappings[i];

            //skip mappings that are not part of current layer
            if (currentMapping->layer_shifted != layerShiftActive) 
            {
                //skip to next for loop run
                continue;
            }

            //check to see if rempped key is being pushed
            if (ev.type == EV_KEY && currentMapping->from_type == EV_KEY && ev.code == currentMapping->from_code)
            {
                //check to see if not a macro
                if (currentMapping->to_key >= 0)
                {
                    //send key along with currnet key state
                    sendInput(virtualMouse, EV_KEY, currentMapping->to_key, ev.value);
                    sendInput(virtualMouse, EV_SYN, SYN_REPORT, 0);
                }
                //macro so only run on key down
                else if (ev.value == 1)
                {
                    //run macro on key down only
                    doMacro(virtualMouse, currentMapping->to_key);
                }
                //update var to prevent sending orignal key
                remapped = 1;

                //end for loop
                break;
            }

            //check to see if scroll event and is currently remapped
            if (ev.type == EV_REL && currentMapping->from_type == EV_REL && ev.code == currentMapping->from_code && ev.value == currentMapping->from_value) 
            {
                //matches so get remap key and send it
                send_key(virtualMouse, currentMapping->to_key);
                remapped = 1; //update var to prevent sending orignal key
                break; //end for loop
            }
        }
        //unlock configuration can be swapped
        pthread_mutex_unlock(&mappings_mutex);

        //check to see if key was remapped or not
        if (!remapped)
        {
            //was not remapped so just send the key event as normal to the virtual device
            write(virtualMouse, &ev, sizeof(ev));
        }
    }

    //clean up program and exit if input device is lost
    ioctl(virtualMouse, UI_DEV_DESTROY);
    close(virtualMouse);
    close(mouseDeviceFile);
    freeMappings();
    return 0;
}
