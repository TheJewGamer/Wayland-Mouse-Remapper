/* 
Author: TheJewGamer
Last Update: 3/8/2026
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
#include <string.h>

//file imports
#include "../headers/vars.h"
#include "../headers/config.h"
#include "../headers/helpers.h"
#include "../headers/inputHandlers.h"
#include "../headers/mouse.h"
#include "../headers/settings.h"

//main
int main() 
{
    //inital preq check
    setupCheck();

    //check to see if presistant mode is enabled
    if(PERSISTENT_MODE == 1)
    {
        
        #if (DEBUG)
            //logging
            printf("loading last used profile\n");
        #endif

        //load last used persistent profile
        loadConfig(PERSISTENT_PROFILE);
    }
    //not enabled
    else
    {
        #if (DEBUG)
            //logging
            printf("loading default profile\n");
        #endif

        //load default config
        loadConfig("default");
    }

    //open/grab mouse as root
    char *mouseDevice = getMouseEventID(MOUSE_PHYS); //get mouse by name
    MOUSEDEVICEFILE = open(mouseDevice, O_RDONLY); //open mouse
    
    #if(DEBUG)
        //logging
        printf("found mouse at %s\n", mouseDevice);
    #endif
    
    //confirm mouse was opened correctly
    if (MOUSEDEVICEFILE < 0) 
    {
        //logging
        fprintf(stderr, "ERROR: Could not access mouse. Exiting\n");

        //stop script
        exit(1);
    } 
    //grab the mouse and prevent events from going to the actually device. (Prevents issue with double input)
    if (ioctl(MOUSEDEVICEFILE, EVIOCGRAB, 1) < 0) 
    {
        //logging
        fprintf(stderr, "ERROR: Could not grab mouse. Exiting\n");

        //stop script
        exit(1); 
    }

    //get mouse keyboard if needed
    int MOUSEKEYBOARDFILE = -1;

    //check to see if mouse keyboard setting is filled out
    if(strlen(MOUSE_KEYBOARD_PHYS) > 0)
    {
        #if(DEBUG)
            //logging
            printf("Mouse keyboard setting set starting setup\n");
        #endif

            //vars
            char *mouseKeyboardDevice = getMouseEventID(MOUSE_KEYBOARD_PHYS); //get mouse keyboard by name
            MOUSEKEYBOARDFILE = open(mouseKeyboardDevice, O_RDONLY); //open mouse keyboard

            #if(DEBUG)
                //logging
                printf("found mouse keyboard at %s\n", mouseKeyboardDevice);
            #endif

            //confirm mouse keyboard was opened correctly
            if (MOUSEKEYBOARDFILE < 0) 
            {
                //logging
                fprintf(stderr, "ERROR: Could not access mouse keyboard. Exiting\n");

                //stop script
                exit(1);
            } 
            //grab the mouse and prevent events from going to the actually device. (Prevents issue with double input)
            if (ioctl(MOUSEKEYBOARDFILE, EVIOCGRAB, 1) < 0) 
            {
                //logging
                fprintf(stderr, "ERROR: Could not grab mouse keyboard. Exiting\n");

                //stop script
                exit(1); 
            }
    }
    //mouse keyboard setting not filled out
    else
    {
        #if(DEBUG)
        //logging
            printf("Mouse Keyboard setting not filled out. Skipping mouse keyboard setup.");
        #endif
    }

    //set up virtual device as root
    VIRTUALMOUSE = setupVirtualMouse();

    //start dubs thread for window listening as  user
    pthread_t windowListenerThread;
    pthread_create(&windowListenerThread, NULL, windowListener, NULL);

    #if (DEBUG)
        //logging
        printf("Starting standard mouse thread\n");
    #endif

    //start standard mouse thread
    pthread_t mouseThread;
    pthread_create(&mouseThread, NULL, inputReader, &MOUSEDEVICEFILE);

    //check to see if keyboard file is loaded
    if (MOUSEKEYBOARDFILE != -1)
    {
        #if (DEBUG)
            //logging
            printf("mouse keyboard present. Starting mouse keyboard thread.\n");
        #endif

        //file is present so start thread for mouse keyboard as well
        pthread_t mouseKeyboardThread;
        pthread_create(&mouseKeyboardThread, NULL, inputReader, &MOUSEKEYBOARDFILE);
    }
    else
    {
        #if (DEBUG)
        //logging
            printf("mouse keyboard not present.\n");
        #endif
    }

    //wait for mouse reader thread to exit
    pthread_join(mouseThread, NULL);

    //clean up program and exit if input device is lost
    ioctl(VIRTUALMOUSE, UI_DEV_DESTROY);
    close(VIRTUALMOUSE);
    close(MOUSEDEVICEFILE);
    close(MOUSEKEYBOARDFILE);
    freeMappings();
    return 0;
}
