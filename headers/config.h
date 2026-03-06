/* 
Author: TheJewGamer
Last Update: 3/6/2026
*/

#ifndef CONFIG_H
#define CONFIG_H

//for reading configurations file
void readConfig(FILE *configurationFileData);

//for getting configuration files
void loadConfig(const char *appName); 

//Note arg is needed here despite not being used.
void *windowListener(void *arg);

#endif