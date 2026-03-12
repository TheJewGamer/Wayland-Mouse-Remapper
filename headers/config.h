/* 
Author: TheJewGamer
Last Update: 3/11/2026
*/

//includes
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <pthread.h>
#include <dbus/dbus.h> 
#include <sys/ioctl.h>
#include <unistd.h>
#include <linux/uinput.h>

//other files
#include "settings.h"
#include "bindings.h"
#include "vars.h"
#include "helpers.h"

//for reading configurations file
void readConfig(FILE *configurationFileData);

//for getting configuration files
void loadConfig(const char *appName); 

//Note arg is needed here despite not being used.
void *windowListener(void *arg);