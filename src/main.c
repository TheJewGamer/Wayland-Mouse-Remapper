/* 
Author: TheJewGamer
Last Update: 3/6/2026
*/

//includes
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <linux/input.h>
#include <linux/uinput.h>
#include <sys/ioctl.h>

//file imports
#include "../headers/vars.h"
#include "../headers/config.h"
#include "../headers/helpers.h"
#include "../headers/inputHandlers.h"
#include "../headers/mouse.h"

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
        if (ev.type == EV_KEY && ev.code == LAYER_TOGGLE_BUTTON)
        {
            //key down only
            if (ev.value == 1)
            {
                //invert var
                LAYER_SHIFT_ACTIVE = !LAYER_SHIFT_ACTIVE;
            }
            
            //end this loop run
            continue;
        }
        //check to see if layershift hold button is pressed
        else if (ev.type == EV_KEY && ev.code == LAYER_HOLD_BUTTON)
        {
            //active while held
            LAYER_SHIFT_ACTIVE = ev.value;

            //end this loop run here
            continue;
        }

        //handle key release even if layer has shifted
        if (ev.type == EV_KEY && ev.value == 0 && HELD_KEY != -1)
        {
            //send stored held key along with current state
            sendInput(virtualMouse, EV_KEY, HELD_KEY, 0);
            sendInput(virtualMouse, EV_SYN, SYN_REPORT, 0);
            
            //reset var
            HELD_KEY = -1;

            //update var
            remapped = 1;
        }

        //lock so configuration cannot be swapped
        pthread_mutex_lock(&BUTTON_MAPPINGS_MUTEX);

        //loop through all active buttonMappings
        for (int i = 0; i < BUTTON_MAPPINGS_AMOUNT; i++) 
        {
            //var
            struct buttonMapping *currentMapping = &BUTTON_MAPPINGS[i];

            //skip buttonMappings that are not part of current layer
            if (currentMapping->layer_shifted != LAYER_SHIFT_ACTIVE) 
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
                    //tracking held key to prevent layer shift issues
                    if (ev.value == 1)
                    {
                        //set var
                        HELD_KEY = currentMapping->to_key;
                    }
                    else if (ev.value == 0)
                    {  
                        // reset var
                        HELD_KEY = -1;
                    }

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
        pthread_mutex_unlock(&BUTTON_MAPPINGS_MUTEX);

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
