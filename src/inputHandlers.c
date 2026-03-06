/* 
Author: TheJewGamer
Last Update: 3/6/2026
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