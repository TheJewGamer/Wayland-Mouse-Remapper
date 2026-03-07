/* 
Author: TheJewGamer
Last Update: 3/6/2026
*/

//for reading configurations file
void readConfig(FILE *configurationFileData);

//for getting configuration files
void loadConfig(const char *appName); 

//Note arg is needed here despite not being used.
void *windowListener(void *arg);