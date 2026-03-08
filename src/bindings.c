/* 
Author: TheJewGamer
Last Update: 3/8/2026
*/

//includes
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <linux/input.h>

//other files
#include "../headers/bindings.h"

//register mouse keys
int mouseKey(const char *keyName, int *type, int *scrollDirection) {
    //var
    *scrollDirection = 0;
    
    //mouse buttons
    if (strcmp(keyName, "BTN_LEFT") == 0)         { *type = EV_KEY; return BTN_LEFT; }
    if (strcmp(keyName, "BTN_RIGHT") == 0)        { *type = EV_KEY; return BTN_RIGHT; }
    if (strcmp(keyName, "BTN_MIDDLE") == 0)       { *type = EV_KEY; return BTN_MIDDLE; }
    if (strcmp(keyName, "BTN_SIDE") == 0)         { *type = EV_KEY; return BTN_SIDE; }
    if (strcmp(keyName, "BTN_EXTRA") == 0)        { *type = EV_KEY; return BTN_EXTRA; }

    //mouse wheel
    if (strcmp(keyName, "REL_HWHEEL_LEFT") == 0)  { *type = EV_REL; *scrollDirection = -1; return REL_HWHEEL; }
    if (strcmp(keyName, "REL_HWHEEL_RIGHT") == 0) { *type = EV_REL; *scrollDirection = 1;  return REL_HWHEEL; }
    if (strcmp(keyName, "REL_WHEEL_UP") == 0)     { *type = EV_REL; *scrollDirection = 1;  return REL_WHEEL; }
    if (strcmp(keyName, "REL_WHEEL_DOWN") == 0)   { *type = EV_REL; *scrollDirection = -1; return REL_WHEEL; }

    //error
    return -1;
}

//register keyboard keys
int keyboardKey(const char *keyName) {
    //special keys
    if (strcmp(keyName, "KEY_ESC") == 0) return KEY_ESC;
    if (strcmp(keyName, "KEY_BACKSPACE") == 0) return KEY_BACKSPACE;
    if (strcmp(keyName, "KEY_TAB") == 0) return KEY_TAB;
    if (strcmp(keyName, "KEY_LEFTBRACE") == 0) return KEY_LEFTBRACE;
    if (strcmp(keyName, "KEY_RIGHTBRACE") == 0) return KEY_RIGHTBRACE;
    if (strcmp(keyName, "KEY_ENTER") == 0) return KEY_ENTER;
    if (strcmp(keyName, "KEY_SEMICOLON") == 0) return KEY_SEMICOLON;
    if (strcmp(keyName, "KEY_APOSTROPHE") == 0) return KEY_APOSTROPHE;
    if (strcmp(keyName, "KEY_GRAVE") == 0) return KEY_GRAVE; // backtik: `
    if (strcmp(keyName, "KEY_BACKSLASH") == 0) return KEY_BACKSLASH;
    if (strcmp(keyName, "KEY_COMMA") == 0) return KEY_COMMA;
    if (strcmp(keyName, "KEY_DOT") == 0) return KEY_DOT;
    if (strcmp(keyName, "KEY_SLASH") == 0) return KEY_SLASH;
    if (strcmp(keyName, "KEY_SPACE") == 0) return KEY_SPACE;
    if (strcmp(keyName, "KEY_HOME") == 0) return KEY_HOME;
    if (strcmp(keyName, "KEY_PAGEUP") == 0) return KEY_PAGEUP;
    if (strcmp(keyName, "KEY_END") == 0) return KEY_END;
    if (strcmp(keyName, "KEY_PAGEDOWN") == 0) return KEY_PAGEDOWN;
    if (strcmp(keyName, "KEY_INSERT") == 0) return KEY_INSERT;
    if (strcmp(keyName, "KEY_DELETE") == 0) return KEY_DELETE;

    //modifier keys
    if (strcmp(keyName, "KEY_CAPSLOCK") == 0) return KEY_CAPSLOCK;
    if (strcmp(keyName, "KEY_NUMLOCK") == 0) return KEY_NUMLOCK;
    if (strcmp(keyName, "KEY_SCROLLLOCK") == 0) return KEY_SCROLLLOCK;
    if (strcmp(keyName, "KEY_LEFTSHIFT") == 0) return KEY_LEFTSHIFT;
    if (strcmp(keyName, "KEY_RIGHTSHIFT") == 0) return KEY_RIGHTSHIFT;
    if (strcmp(keyName, "KEY_LEFTCTRL") == 0) return KEY_LEFTCTRL;
    if (strcmp(keyName, "KEY_RIGHTCTRL") == 0) return KEY_RIGHTCTRL;
    if (strcmp(keyName, "KEY_LEFTALT") == 0) return KEY_LEFTALT;
    if (strcmp(keyName, "KEY_RIGHTALT") == 0) return KEY_RIGHTALT;
    if (strcmp(keyName, "KEY_LEFTMETA") == 0) return KEY_LEFTMETA; //windows key
    if (strcmp(keyName, "KEY_RIGHTMETA") == 0) return KEY_RIGHTMETA; //menu key

    //number row
    if (strcmp(keyName, "KEY_1") == 0) return KEY_1;
    if (strcmp(keyName, "KEY_2") == 0) return KEY_2;
    if (strcmp(keyName, "KEY_3") == 0) return KEY_3;
    if (strcmp(keyName, "KEY_4") == 0) return KEY_4;
    if (strcmp(keyName, "KEY_5") == 0) return KEY_5;
    if (strcmp(keyName, "KEY_6") == 0) return KEY_6;
    if (strcmp(keyName, "KEY_7") == 0) return KEY_7;
    if (strcmp(keyName, "KEY_8") == 0) return KEY_8;
    if (strcmp(keyName, "KEY_9") == 0) return KEY_9;
    if (strcmp(keyName, "KEY_0") == 0) return KEY_0;
    if (strcmp(keyName, "KEY_MINUS") == 0) return KEY_MINUS;
    if (strcmp(keyName, "KEY_EQUAL") == 0) return KEY_EQUAL;

    //letters
    if (strcmp(keyName, "KEY_Q") == 0) return KEY_Q;
    if (strcmp(keyName, "KEY_W") == 0) return KEY_W;
    if (strcmp(keyName, "KEY_E") == 0) return KEY_E;
    if (strcmp(keyName, "KEY_R") == 0) return KEY_R;
    if (strcmp(keyName, "KEY_T") == 0) return KEY_T;
    if (strcmp(keyName, "KEY_Y") == 0) return KEY_Y;
    if (strcmp(keyName, "KEY_U") == 0) return KEY_U;
    if (strcmp(keyName, "KEY_I") == 0) return KEY_I;
    if (strcmp(keyName, "KEY_O") == 0) return KEY_O;
    if (strcmp(keyName, "KEY_P") == 0) return KEY_P;
    if (strcmp(keyName, "KEY_A") == 0) return KEY_A;
    if (strcmp(keyName, "KEY_S") == 0) return KEY_S;
    if (strcmp(keyName, "KEY_D") == 0) return KEY_D;
    if (strcmp(keyName, "KEY_F") == 0) return KEY_F;
    if (strcmp(keyName, "KEY_G") == 0) return KEY_G;
    if (strcmp(keyName, "KEY_H") == 0) return KEY_H;
    if (strcmp(keyName, "KEY_J") == 0) return KEY_J;
    if (strcmp(keyName, "KEY_K") == 0) return KEY_K;
    if (strcmp(keyName, "KEY_L") == 0) return KEY_L;
    if (strcmp(keyName, "KEY_Z") == 0) return KEY_Z;
    if (strcmp(keyName, "KEY_X") == 0) return KEY_X;
    if (strcmp(keyName, "KEY_C") == 0) return KEY_C;
    if (strcmp(keyName, "KEY_V") == 0) return KEY_V;
    if (strcmp(keyName, "KEY_B") == 0) return KEY_B;
    if (strcmp(keyName, "KEY_N") == 0) return KEY_N;
    if (strcmp(keyName, "KEY_M") == 0) return KEY_M;

    //function keys
    if (strcmp(keyName, "KEY_F1") == 0) return KEY_F1;
    if (strcmp(keyName, "KEY_F2") == 0) return KEY_F2;
    if (strcmp(keyName, "KEY_F3") == 0) return KEY_F3;
    if (strcmp(keyName, "KEY_F4") == 0) return KEY_F4;
    if (strcmp(keyName, "KEY_F5") == 0) return KEY_F5;
    if (strcmp(keyName, "KEY_F6") == 0) return KEY_F6;
    if (strcmp(keyName, "KEY_F7") == 0) return KEY_F7;
    if (strcmp(keyName, "KEY_F8") == 0) return KEY_F8;
    if (strcmp(keyName, "KEY_F9") == 0) return KEY_F9;
    if (strcmp(keyName, "KEY_F10") == 0) return KEY_F10;
    if (strcmp(keyName, "KEY_F11") == 0) return KEY_F11;
    if (strcmp(keyName, "KEY_F12") == 0) return KEY_F12;

    //extra function keys (these are added so extra mouse buttons can be used. By default mouses can only have two side buttons so can mark extra buttons as these)
    if (strcmp(keyName, "KEY_F13") == 0) return KEY_F13;
    if (strcmp(keyName, "KEY_F14") == 0) return KEY_F14;
    if (strcmp(keyName, "KEY_F15") == 0) return KEY_F15;
    if (strcmp(keyName, "KEY_F16") == 0) return KEY_F16;
    if (strcmp(keyName, "KEY_F17") == 0) return KEY_F17;
    if (strcmp(keyName, "KEY_F18") == 0) return KEY_F18;
    if (strcmp(keyName, "KEY_F19") == 0) return KEY_F19;
    if (strcmp(keyName, "KEY_F20") == 0) return KEY_F20;
    if (strcmp(keyName, "KEY_F21") == 0) return KEY_F21;
    if (strcmp(keyName, "KEY_F22") == 0) return KEY_F22;
    if (strcmp(keyName, "KEY_F23") == 0) return KEY_F23;
    if (strcmp(keyName, "KEY_F24") == 0) return KEY_F24;

    //numpad
    if (strcmp(keyName, "KEY_KP0") == 0) return KEY_KP0;
    if (strcmp(keyName, "KEY_KP1") == 0) return KEY_KP1;
    if (strcmp(keyName, "KEY_KP2") == 0) return KEY_KP2;
    if (strcmp(keyName, "KEY_KP3") == 0) return KEY_KP3;
    if (strcmp(keyName, "KEY_KP4") == 0) return KEY_KP4;
    if (strcmp(keyName, "KEY_KP5") == 0) return KEY_KP5;
    if (strcmp(keyName, "KEY_KP6") == 0) return KEY_KP6;
    if (strcmp(keyName, "KEY_KP7") == 0) return KEY_KP7;
    if (strcmp(keyName, "KEY_KP8") == 0) return KEY_KP8;
    if (strcmp(keyName, "KEY_KP9") == 0) return KEY_KP9;
    if (strcmp(keyName, "KEY_KPMINUS") == 0) return KEY_KPMINUS;
    if (strcmp(keyName, "KEY_KPPLUS") == 0) return KEY_KPPLUS;
    if (strcmp(keyName, "KEY_KPDOT") == 0) return KEY_KPDOT;
    if (strcmp(keyName, "KEY_KPENTER") == 0) return KEY_KPENTER;
    if (strcmp(keyName, "KEY_KPSLASH") == 0) return KEY_KPSLASH;
    if (strcmp(keyName, "KEY_KPASTERISK") == 0) return KEY_KPASTERISK;

    //arrow keys
    if (strcmp(keyName, "KEY_UP") == 0) return KEY_UP;
    if (strcmp(keyName, "KEY_LEFT") == 0) return KEY_LEFT;
    if (strcmp(keyName, "KEY_RIGHT") == 0) return KEY_RIGHT;
    if (strcmp(keyName, "KEY_DOWN") == 0) return KEY_DOWN;

    //macros actions
    if (strcmp(keyName, "COPY_MACRO") == 0) return -2;
    if (strcmp(keyName, "PASTE_MACRO") == 0) return -3;

    //error
    return -1;
}

//method to figure out if a mouse or keyboard key
int parseFromKey(const char *keyName, int *type, int *scrollDirection)
{
    // check to see if a mouse key
    int keyCode = mouseKey(keyName, type, scrollDirection);
    
    if (keyCode != -1) 
    {
        //is mouse key
        return keyCode;
    }
    //not a mouse key
    else
    {
        // check to see if a keyboard key
        keyCode = keyboardKey(keyName);
        
        if (keyCode != -1)
        {
            //set vars
            *type = EV_KEY;
            *scrollDirection = 0;
            
            //is keyboard key
            return keyCode;
        }
    }

    //error
    return -1;
}
