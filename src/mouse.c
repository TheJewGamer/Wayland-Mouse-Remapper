/* 
Author: TheJewGamer
Last Update: 3/8/2026
*/

//standard imports
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>
#include <linux/uinput.h>

//file imports
#include "../headers/vars.h"

//get mouse event id by name. Needed as event ids can change on reboot
char* getMouseEventID(const char *physPath)
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
        //check to see if current line contains physPath
        if (strstr(currentLine, physPath))
        {
            //it did update var
            foundMouse = 1;
        }

        //check to see if we found the physPath line
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

    //logging
    fprintf(stderr, "ERROR: could not find mouse device: %s\n", physPath);

    //stop script
    exit(1);
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