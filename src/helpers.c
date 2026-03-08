/* 
Author: TheJewGamer
Last Update: 3/8/2026
*/

//standard includes
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>
#include <sys/stat.h>
#include <string.h>

//file includes
#include "../headers/vars.h"
#include "../headers/settings.h"

//method to clean the array and other vars. Garbage collection
void freeMappings()
{
    free(BUTTON_MAPPINGS);
    BUTTON_MAPPINGS = NULL;
    BUTTON_MAPPINGS_AMOUNT = 0;
    BUTTON_MAPPING_ARRAY_SIZE = 0;
    LAYER_TOGGLE_BUTTON = -1;
    LAYER_HOLD_BUTTON = -1;
    LAYER_SHIFT_ACTIVE = 0;
}

//helper function to check that needed things are done when called. Prevents launches that could cause errors
void setupCheck()
{
    //set user var. Needs to be done first as using in a lot of places and will crash program if not set
    USER = getenv("USER");

    //check to see if home directory can be found
    struct passwd *userInfo = getpwnam(USER); //get user info by looking up the password database (password are not stored here so safe)
    if (userInfo) 
    {
        //get home dir of user that called sudo
        HOMEPATH = userInfo->pw_dir;
    }
    //error
    else
    {   
        //logging
        fprintf(stderr, "ERROR: Could not find home directory\n");
        
        //exit script
        exit(1);
    }

    //check to see if we can get the configuration directory

    //get configuration path
    snprintf(CONFIGURATIONFOLDERPATH, sizeof(CONFIGURATIONFOLDERPATH), "%s/.config/wayland-mouse-remapper", HOMEPATH);

    //confirm the path exists
    if (!opendir(CONFIGURATIONFOLDERPATH))
    {
        //logging
        fprintf(stderr, "ERROR: Could not find configuration folder. Please create at the following location under your user profile: ~/.config/wayland-mouse-remapper or run the setup script.");
        
        //exit script
        exit(1);
    }

    //check to see if we can get the default configuration file

    //vars
    char defaultConfigurationFilePath[512];
    snprintf(defaultConfigurationFilePath, sizeof(defaultConfigurationFilePath), "%s/.config/wayland-mouse-remapper/default.conf", HOMEPATH);
    struct stat buffer;
    
    if (stat(defaultConfigurationFilePath, &buffer) != 0)
    {
        //logging
        fprintf(stderr, "ERROR: Default configuration file does not exist. Please run the setup script.\n");
        
        //exit script
        exit(1);
    }

    //load saved settings
    loadSettings();

    //confirm that mouse setting is not empty
    if(strlen(MOUSE_PHYS) <= 0)
    {
        //logging
        fprintf(stderr, "ERROR: MOUSE_PHYS var not set in settings file. Please run the setup script to set this var.\n");

        //quit program
        exit(1);
    }
}