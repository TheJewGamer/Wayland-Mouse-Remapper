/* 
Author: TheJewGamer
Last Update: 3/8/2026
*/

//standard includes
#include <unistd.h>
#include <pthread.h>
#include <linux/input.h>
#include <linux/uinput.h>

//file imports
#include "../headers/inputHandlers.h"
#include "../headers/bindings.h"
#include "../headers/vars.h"

//method to send inputs to virtual device
void sendInput(int deviceFile, int eventType, int keyCode, int keyState) 
{
    struct input_event ev = {0};
    ev.type = eventType;
    ev.code = keyCode;
    ev.value = keyState;
    write(deviceFile, &ev, sizeof(ev));
}

//method to simulate key presses
void send_key(int deviceFile, int keyCode) 
{
    sendInput(deviceFile, EV_KEY, keyCode, 1); //key down
    sendInput(deviceFile, EV_SYN, SYN_REPORT, 0); //key down complete
    sendInput(deviceFile, EV_KEY, keyCode, 0); //key up
    sendInput(deviceFile, EV_SYN, SYN_REPORT, 0); //key up complete
}

//method to send macros
void doMacro(int deviceFile, int macroInput)
{
    //copy macro
    if (macroInput == -2)
    {
        sendInput(deviceFile, EV_KEY, KEY_LEFTCTRL, 1);
        sendInput(deviceFile, EV_KEY, KEY_C, 1);
        sendInput(deviceFile, EV_SYN, SYN_REPORT, 0);
        sendInput(deviceFile, EV_KEY, KEY_C, 0);
        sendInput(deviceFile, EV_KEY, KEY_LEFTCTRL, 0);
        sendInput(deviceFile, EV_SYN, SYN_REPORT, 0);
    }
    //paste macro
    else if (macroInput == -3)
    {
        sendInput(deviceFile, EV_KEY, KEY_LEFTCTRL, 1);
        sendInput(deviceFile, EV_KEY, KEY_V, 1);
        sendInput(deviceFile, EV_SYN, SYN_REPORT, 0);
        sendInput(deviceFile, EV_KEY, KEY_V, 0);
        sendInput(deviceFile, EV_KEY, KEY_LEFTCTRL, 0);
        sendInput(deviceFile, EV_SYN, SYN_REPORT, 0);
    }
}

//method to process an input event
void processEvent(struct input_event ev)
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
        return;
    }
    //check to see if layershift hold button is pressed
    else if (ev.type == EV_KEY && ev.code == LAYER_HOLD_BUTTON)
    {
        //active while held
        LAYER_SHIFT_ACTIVE = ev.value;
        return;
    }

    //lock so configuration cannot be swapped
    pthread_mutex_lock(&BUTTON_MAPPINGS_MUTEX);

    //handle key release even if layer has shifted
    if (ev.type == EV_KEY && ev.value == 0 && HELD_KEY != -1)
    {
        sendInput(VIRTUALMOUSE, EV_KEY, HELD_KEY, 0);
        sendInput(VIRTUALMOUSE, EV_SYN, SYN_REPORT, 0);
        HELD_KEY = -1;
        remapped = 1;
    }

    //loop through all active buttonMappings
    for (int i = 0; i < BUTTON_MAPPINGS_AMOUNT; i++)
    {
        struct buttonMapping *currentMapping = &BUTTON_MAPPINGS[i];

        //skip buttonMappings that are not part of current layer
        if (currentMapping->layer_shifted != LAYER_SHIFT_ACTIVE) continue;

        //check to see if remapped key is being pushed
        if (ev.type == EV_KEY && currentMapping->from_type == EV_KEY && ev.code == currentMapping->from_code)
        {
            //check to see if not a macro
            if (currentMapping->to_key >= 0)
            {
                if (ev.value == 1)
                    HELD_KEY = currentMapping->to_key;
                else if (ev.value == 0)
                    HELD_KEY = -1;

                sendInput(VIRTUALMOUSE, EV_KEY, currentMapping->to_key, ev.value);
                sendInput(VIRTUALMOUSE, EV_SYN, SYN_REPORT, 0);
            }
            //macro so only run on key down
            else if (ev.value == 1)
            {
                doMacro(VIRTUALMOUSE, currentMapping->to_key);
            }
            remapped = 1;
            break;
        }

        //check to see if scroll event and is currently remapped
        if (ev.type == EV_REL && currentMapping->from_type == EV_REL && ev.code == currentMapping->from_code && ev.value == currentMapping->from_value)
        {
            send_key(VIRTUALMOUSE, currentMapping->to_key);
            remapped = 1;
            break;
        }
    }

    //unlock configuration can be swapped
    pthread_mutex_unlock(&BUTTON_MAPPINGS_MUTEX);

    //check to see if key was remapped or not
    if (!remapped)
    {
        write(VIRTUALMOUSE, &ev, sizeof(ev));
    }
}

//thread to read input
void *inputReader(void *deviceFileInput)
{
    //vars
    int deviceFile = *(int *)deviceFileInput;
    struct input_event ev;

    //loop while getting input from device. Sleeps when no new event
    while (read(deviceFile, &ev, sizeof(ev)) == sizeof(ev))
    {
        //send key event to processing method
        processEvent(ev);
    }

    //done
    return NULL;
}