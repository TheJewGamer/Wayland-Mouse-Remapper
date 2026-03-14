/* 
Author: TheJewGamer
Last Update: 3/14/2026
*/

//header file
#include "../headers/inputHandlers.h"

//method to send inputs to virtual device
void sendInput(int deviceFile, int eventType, int keyCode, int keyState) 
{
    struct input_event ev = {0};
    ev.type = eventType;
    ev.code = keyCode;
    ev.value = keyState;

    //write to device
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
        #if (DEBUG)
            //logging
            printf("Layershift toggle button pushed\n");
        #endif

        //key down only
        if (ev.value == 1)
        {
            #if (DEBUG)
            //logging
                printf("Toggling layershift.\n");
            #endif

            //invert var
            LAYER_SHIFT_ACTIVE = !LAYER_SHIFT_ACTIVE;

            #if (DEBUG)
                //logging
                printf("Layershift is now: %d\n", LAYER_SHIFT_ACTIVE);
            #endif
        }
        return;
    }
    //check to see if layershift hold button is pressed
    else if (ev.type == EV_KEY && ev.code == LAYER_HOLD_BUTTON)
    {
        //ignore held state as no change
        if(ev.value == 2)
        {
            #if (DEBUG)
            //logging
            printf("Layershift hold button currently held\n");
        #endif
            
            //end this run here
            return;
        }

        #if (DEBUG)
            //logging
            printf("Layershift hold button pressed. Current state: %d\n", ev.value);
        #endif

        //active while held
        LAYER_SHIFT_ACTIVE = ev.value;
        return;
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
            //go to next run of for loop
            continue;
        }

        //check to see if remapped key is being pushed
        if (ev.type == EV_KEY && currentMapping->from_type == EV_KEY && ev.code == currentMapping->from_code)
        {
            #if (DEBUG)
                //logging
                printf("Remapped Key Pushed\n");
            #endif

            //check to see if not a macro (marcos are negative key codes)
            if (currentMapping->to_key >= 0)
            {
                //key is down
                if (ev.value == 1)
                {
                    #if (DEBUG)
                        //logging
                        printf("Non Macro Key down\n");
                    #endif

                    //store currently held down key
                    CURRENT_DOWN_REMAP_BUTTONS[ev.code] = currentMapping->to_key;

                    //send input
                    sendInput(VIRTUALMOUSE, EV_KEY, currentMapping->to_key, ev.value);
                    sendInput(VIRTUALMOUSE, EV_SYN, SYN_REPORT, 0);
                }
                //key is up
                else if (ev.value == 0)
                {
                    #if (DEBUG)
                        //logging
                        printf("Non Macro Key up\n");
                    #endif

                    //check to see if released button is remapped
                    if (CURRENT_DOWN_REMAP_BUTTONS[ev.code] != -1)
                    {
                        //send key up event for remppaed key that was stored when button was pressed (present issues with keys getting stuck while holding a key and doing release or activing layershift)
                        sendInput(VIRTUALMOUSE, EV_KEY, CURRENT_DOWN_REMAP_BUTTONS[ev.code], 0);
                        sendInput(VIRTUALMOUSE, EV_SYN, SYN_REPORT, 0);
                        
                        //reset keycode to inactive
                        CURRENT_DOWN_REMAP_BUTTONS[ev.code] = -1;
                        
                        //update var
                        remapped = 1;
                    }
                }
                //held down or error
                else
                {
                    #if (DEBUG)
                        //check to see if just held down
                        if(ev.value == 2)
                        {
                            //logging
                            printf("Key is held down\n");
                        }
                        else
                        {
                            //error
                            fprintf(stderr, "ERROR: on input handling\n");
                        }
                    #endif
                }

            }
            //macro so only run on key down
            else if (ev.value == 1)
            {
                #if (DEBUG)
                    //logging
                    printf("Macro Key down\n");
                #endif

                //send macro
                doMacro(VIRTUALMOUSE, currentMapping->to_key);
            }
            //update var
            remapped = 1;

            //end for loop
            break;
        }

        //check to see if scroll event and is currently remapped
        if (ev.type == EV_REL && currentMapping->from_type == EV_REL && ev.code == currentMapping->from_code && ev.value == currentMapping->from_value)
        {
            #if (DEBUG)
                //logging
                printf("Scroll Event fired and remapped\n");
            #endif

            //send input
            send_key(VIRTUALMOUSE, currentMapping->to_key);
            
            //update var
            remapped = 1;

            //end for loop
            break;
        }
    }

    //unlock configuration can be swapped
    pthread_mutex_unlock(&BUTTON_MAPPINGS_MUTEX);

    //check to see if key was remapped or not
    if (!remapped)
    {
        // #if (DEBUG)
        //     //logging
        //     printf("Input not remapped send as raw");
        // #endif

        //not remapped key so just send input as normal
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