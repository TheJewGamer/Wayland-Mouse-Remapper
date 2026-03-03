#ifndef CONFIG_H
#define CONFIG_H

//includes
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

//functions
void readConfig(FILE *configurationFileData);
void loadConfig(const char *appName);

#endif