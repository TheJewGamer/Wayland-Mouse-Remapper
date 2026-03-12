/* 
Author: TheJewGamer
Last Update: 3/11/2026
*/

//standard includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//other files
#include "../headers/settings.h"

void saveSettings()
{
    //vars
    char settingsFilePath[512];

    //get settings file path
    snprintf(settingsFilePath, sizeof(settingsFilePath), "%s/.config/wayland-mouse-remapper/settings.ini", HOMEPATH);

    //open settings file
    FILE *settingsFile = fopen(settingsFilePath, "w");

    //confirm not null
    if (!settingsFile)
    {
        //logging
        fprintf(stderr, "ERROR: could not save settings file\n");

        //exit program
        exit(1);
    }

    //write new vars to settings file
    fprintf(settingsFile, "PERSISTENT_MODE=%d\n", PERSISTENT_MODE);
    fprintf(settingsFile, "PERSISTENT_PROFILE=%s\n", PERSISTENT_PROFILE);

    //close settings file
    fclose(settingsFile);

    //logging
    #if(DEBUG)
    {
        printf("settings saved\n");
    }
    #endif
}

void loadSettings()
{
    //vars
    char settingsFilePath[512];

    //get settings file Path
    snprintf(settingsFilePath, sizeof(settingsFilePath), "%s/.config/wayland-mouse-remapper/settings.ini", HOMEPATH);

    //open settings file
    FILE *settingsFile = fopen(settingsFilePath, "r");

    //confirm not null
    if (!settingsFile)
    {
        #if (DEBUG)
            //logging
            fprintf(stderr, "No settings file found exiting\n");
        #endif

        //exit program
        exit(1);
    }

    //var
    char currentLine[256];

    //lopp through every line in the settings file
    while (fgets(currentLine, sizeof(currentLine), settingsFile))
    {
        //confirm not comment or new line
        if (currentLine[0] == '#' || currentLine[0] == '\n')
        {
            //go to next run of while loop
            continue;
        }
        else
        {
            //remove end line from currentLine
            currentLine[strcspn(currentLine, "\n")] = 0;
        }

        //vars
        char settingName[64];
        char settingValue[256];

        //try to split via the = sign
        if (sscanf(currentLine, "%63[^=]=%255s", settingName, settingValue) != 2)
        {
            //go to next while loop run if cannot split
            continue;
        }
        //check to see if mouse phys path setting
        if (strcmp(settingName, "MOUSE_PHYS") == 0)
        {
            //set global var
            strncpy(MOUSE_PHYS, settingValue, sizeof(MOUSE_PHYS));
        }
        //check to see if mouse keyboard phys path setting
        if (strcmp(settingName, "MOUSE_KEYBOARD_PHYS") == 0)
        {
            //set global var
            strncpy(MOUSE_KEYBOARD_PHYS, settingValue, sizeof(MOUSE_KEYBOARD_PHYS));
        }
        //check to see if PERSISTENT_MODE setting
        else if (strcmp(settingName, "PERSISTENT_MODE") == 0)
        {
            //set global var
            PERSISTENT_MODE = atoi(settingValue);
        }
        //check to see if PERSISTENT_PROFILE setting     
        else if (strcmp(settingName, "PERSISTENT_PROFILE") == 0)
        {
            //set global var
            strncpy(PERSISTENT_PROFILE, settingValue, sizeof(PERSISTENT_PROFILE));
        }
    }

    //close file
    fclose(settingsFile);

    //logging
    #if (DEBUG)
        printf("settings loaded\n");
    #endif
}