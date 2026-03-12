/* 
Author: TheJewGamer
Last Update: 3/11/2026
*/

//standard imports
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>
#include <linux/uinput.h>

//other files
#include "vars.h"

//get mouse event id by name. Needed as event ids can change on reboot
char* getMouseEventID(const char *physFilter);

// sets up the virutal mouse
int setupVirtualMouse();