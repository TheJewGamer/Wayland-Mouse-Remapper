/* 
Author: TheJewGamer
Last Update: 3/6/2026
*/

//includes
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <linux/input.h>

//other files
#include "../headers/bindings.h"

//register mouse keys
int mouseKey(const char *name, int *type, int *scrollDirection) {
    //var
    *scrollDirection = 0;
    
    //mouse buttons
    if (strcmp(name, "BTN_LEFT") == 0)         { *type = EV_KEY; return BTN_LEFT; }
    if (strcmp(name, "BTN_RIGHT") == 0)        { *type = EV_KEY; return BTN_RIGHT; }
    if (strcmp(name, "BTN_MIDDLE") == 0)       { *type = EV_KEY; return BTN_MIDDLE; }
    if (strcmp(name, "BTN_SIDE") == 0)         { *type = EV_KEY; return BTN_SIDE; }
    if (strcmp(name, "BTN_EXTRA") == 0)        { *type = EV_KEY; return BTN_EXTRA; }

    //mouse wheel
    if (strcmp(name, "REL_HWHEEL_LEFT") == 0)  { *type = EV_REL; *scrollDirection = -1; return REL_HWHEEL; }
    if (strcmp(name, "REL_HWHEEL_RIGHT") == 0) { *type = EV_REL; *scrollDirection = 1;  return REL_HWHEEL; }
    if (strcmp(name, "REL_WHEEL_UP") == 0)     { *type = EV_REL; *scrollDirection = 1;  return REL_WHEEL; }
    if (strcmp(name, "REL_WHEEL_DOWN") == 0)   { *type = EV_REL; *scrollDirection = -1; return REL_WHEEL; }

    //error
    return -1;
}

//register keyboard keys
int keyboardKey(const char *name) {
    //special keys
    if (strcmp(name, "KEY_ESC") == 0) return KEY_ESC;
    if (strcmp(name, "KEY_BACKSPACE") == 0) return KEY_BACKSPACE;
    if (strcmp(name, "KEY_TAB") == 0) return KEY_TAB;
    if (strcmp(name, "KEY_LEFTBRACE") == 0) return KEY_LEFTBRACE;
    if (strcmp(name, "KEY_RIGHTBRACE") == 0) return KEY_RIGHTBRACE;
    if (strcmp(name, "KEY_ENTER") == 0) return KEY_ENTER;
    if (strcmp(name, "KEY_SEMICOLON") == 0) return KEY_SEMICOLON;
    if (strcmp(name, "KEY_APOSTROPHE") == 0) return KEY_APOSTROPHE;
    if (strcmp(name, "KEY_GRAVE") == 0) return KEY_GRAVE; // backtik: `
    if (strcmp(name, "KEY_BACKSLASH") == 0) return KEY_BACKSLASH;
    if (strcmp(name, "KEY_COMMA") == 0) return KEY_COMMA;
    if (strcmp(name, "KEY_DOT") == 0) return KEY_DOT;
    if (strcmp(name, "KEY_SLASH") == 0) return KEY_SLASH;
    if (strcmp(name, "KEY_SPACE") == 0) return KEY_SPACE;
    if (strcmp(name, "KEY_HOME") == 0) return KEY_HOME;
    if (strcmp(name, "KEY_PAGEUP") == 0) return KEY_PAGEUP;
    if (strcmp(name, "KEY_END") == 0) return KEY_END;
    if (strcmp(name, "KEY_PAGEDOWN") == 0) return KEY_PAGEDOWN;
    if (strcmp(name, "KEY_INSERT") == 0) return KEY_INSERT;
    if (strcmp(name, "KEY_DELETE") == 0) return KEY_DELETE;

    //modifier keys
    if (strcmp(name, "KEY_CAPSLOCK") == 0) return KEY_CAPSLOCK;
    if (strcmp(name, "KEY_NUMLOCK") == 0) return KEY_NUMLOCK;
    if (strcmp(name, "KEY_SCROLLLOCK") == 0) return KEY_SCROLLLOCK;
    if (strcmp(name, "KEY_LEFTSHIFT") == 0) return KEY_LEFTSHIFT;
    if (strcmp(name, "KEY_RIGHTSHIFT") == 0) return KEY_RIGHTSHIFT;
    if (strcmp(name, "KEY_LEFTCTRL") == 0) return KEY_LEFTCTRL;
    if (strcmp(name, "KEY_RIGHTCTRL") == 0) return KEY_RIGHTCTRL;
    if (strcmp(name, "KEY_LEFTALT") == 0) return KEY_LEFTALT;
    if (strcmp(name, "KEY_RIGHTALT") == 0) return KEY_RIGHTALT;
    if (strcmp(name, "KEY_LEFTMETA") == 0) return KEY_LEFTMETA; //windows key
    if (strcmp(name, "KEY_RIGHTMETA") == 0) return KEY_RIGHTMETA; //menu key

    //number row
    if (strcmp(name, "KEY_1") == 0) return KEY_1;
    if (strcmp(name, "KEY_2") == 0) return KEY_2;
    if (strcmp(name, "KEY_3") == 0) return KEY_3;
    if (strcmp(name, "KEY_4") == 0) return KEY_4;
    if (strcmp(name, "KEY_5") == 0) return KEY_5;
    if (strcmp(name, "KEY_6") == 0) return KEY_6;
    if (strcmp(name, "KEY_7") == 0) return KEY_7;
    if (strcmp(name, "KEY_8") == 0) return KEY_8;
    if (strcmp(name, "KEY_9") == 0) return KEY_9;
    if (strcmp(name, "KEY_0") == 0) return KEY_0;
    if (strcmp(name, "KEY_MINUS") == 0) return KEY_MINUS;
    if (strcmp(name, "KEY_EQUAL") == 0) return KEY_EQUAL;

    //letters
    if (strcmp(name, "KEY_Q") == 0) return KEY_Q;
    if (strcmp(name, "KEY_W") == 0) return KEY_W;
    if (strcmp(name, "KEY_E") == 0) return KEY_E;
    if (strcmp(name, "KEY_R") == 0) return KEY_R;
    if (strcmp(name, "KEY_T") == 0) return KEY_T;
    if (strcmp(name, "KEY_Y") == 0) return KEY_Y;
    if (strcmp(name, "KEY_U") == 0) return KEY_U;
    if (strcmp(name, "KEY_I") == 0) return KEY_I;
    if (strcmp(name, "KEY_O") == 0) return KEY_O;
    if (strcmp(name, "KEY_P") == 0) return KEY_P;
    if (strcmp(name, "KEY_A") == 0) return KEY_A;
    if (strcmp(name, "KEY_S") == 0) return KEY_S;
    if (strcmp(name, "KEY_D") == 0) return KEY_D;
    if (strcmp(name, "KEY_F") == 0) return KEY_F;
    if (strcmp(name, "KEY_G") == 0) return KEY_G;
    if (strcmp(name, "KEY_H") == 0) return KEY_H;
    if (strcmp(name, "KEY_J") == 0) return KEY_J;
    if (strcmp(name, "KEY_K") == 0) return KEY_K;
    if (strcmp(name, "KEY_L") == 0) return KEY_L;
    if (strcmp(name, "KEY_Z") == 0) return KEY_Z;
    if (strcmp(name, "KEY_X") == 0) return KEY_X;
    if (strcmp(name, "KEY_C") == 0) return KEY_C;
    if (strcmp(name, "KEY_V") == 0) return KEY_V;
    if (strcmp(name, "KEY_B") == 0) return KEY_B;
    if (strcmp(name, "KEY_N") == 0) return KEY_N;
    if (strcmp(name, "KEY_M") == 0) return KEY_M;

    //function keys
    if (strcmp(name, "KEY_F1") == 0) return KEY_F1;
    if (strcmp(name, "KEY_F2") == 0) return KEY_F2;
    if (strcmp(name, "KEY_F3") == 0) return KEY_F3;
    if (strcmp(name, "KEY_F4") == 0) return KEY_F4;
    if (strcmp(name, "KEY_F5") == 0) return KEY_F5;
    if (strcmp(name, "KEY_F6") == 0) return KEY_F6;
    if (strcmp(name, "KEY_F7") == 0) return KEY_F7;
    if (strcmp(name, "KEY_F8") == 0) return KEY_F8;
    if (strcmp(name, "KEY_F9") == 0) return KEY_F9;
    if (strcmp(name, "KEY_F10") == 0) return KEY_F10;
    if (strcmp(name, "KEY_F11") == 0) return KEY_F11;
    if (strcmp(name, "KEY_F12") == 0) return KEY_F12;

    //extra function keys (these are added so extra mouse buttons can be used. By default mouses can only have two side buttons so can mark extra buttons as these)
    if (strcmp(name, "KEY_F13") == 0) return KEY_F13;
    if (strcmp(name, "KEY_F14") == 0) return KEY_F14;
    if (strcmp(name, "KEY_F15") == 0) return KEY_F15;
    if (strcmp(name, "KEY_F16") == 0) return KEY_F16;
    if (strcmp(name, "KEY_F17") == 0) return KEY_F17;
    if (strcmp(name, "KEY_F18") == 0) return KEY_F18;
    if (strcmp(name, "KEY_F19") == 0) return KEY_F19;
    if (strcmp(name, "KEY_F20") == 0) return KEY_F20;
    if (strcmp(name, "KEY_F21") == 0) return KEY_F21;
    if (strcmp(name, "KEY_F22") == 0) return KEY_F22;
    if (strcmp(name, "KEY_F23") == 0) return KEY_F23;
    if (strcmp(name, "KEY_F24") == 0) return KEY_F24;

    //numpad
    if (strcmp(name, "KEY_KP0") == 0) return KEY_KP0;
    if (strcmp(name, "KEY_KP1") == 0) return KEY_KP1;
    if (strcmp(name, "KEY_KP2") == 0) return KEY_KP2;
    if (strcmp(name, "KEY_KP3") == 0) return KEY_KP3;
    if (strcmp(name, "KEY_KP4") == 0) return KEY_KP4;
    if (strcmp(name, "KEY_KP5") == 0) return KEY_KP5;
    if (strcmp(name, "KEY_KP6") == 0) return KEY_KP6;
    if (strcmp(name, "KEY_KP7") == 0) return KEY_KP7;
    if (strcmp(name, "KEY_KP8") == 0) return KEY_KP8;
    if (strcmp(name, "KEY_KP9") == 0) return KEY_KP9;
    if (strcmp(name, "KEY_KPMINUS") == 0) return KEY_KPMINUS;
    if (strcmp(name, "KEY_KPPLUS") == 0) return KEY_KPPLUS;
    if (strcmp(name, "KEY_KPDOT") == 0) return KEY_KPDOT;
    if (strcmp(name, "KEY_KPENTER") == 0) return KEY_KPENTER;
    if (strcmp(name, "KEY_KPSLASH") == 0) return KEY_KPSLASH;
    if (strcmp(name, "KEY_KPASTERISK") == 0) return KEY_KPASTERISK;

    //arrow keys
    if (strcmp(name, "KEY_UP") == 0) return KEY_UP;
    if (strcmp(name, "KEY_LEFT") == 0) return KEY_LEFT;
    if (strcmp(name, "KEY_RIGHT") == 0) return KEY_RIGHT;
    if (strcmp(name, "KEY_DOWN") == 0) return KEY_DOWN;

    //macros actions
    if (strcmp(name, "COPY_MACRO") == 0) return -2;
    if (strcmp(name, "PASTE_MACRO") == 0) return -3;

    //error
    return -1;
}
