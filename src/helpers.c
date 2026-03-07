/* 
Author: TheJewGamer
Last Update: 3/7/2026
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

//helper method to drop privileges back to user who called sudo
void dropPrivileges()
{
    //var
    struct passwd *userInfo = getpwnam(USER);

    //drop supplementary groups gid then uid
    if (initgroups(USER, userInfo->pw_gid) < 0) { perror("initgroups"); exit(1); }
    if (setgid(userInfo->pw_gid) < 0) { perror("setgid"); exit(1); }
    if (setuid(userInfo->pw_uid) < 0) { perror("setuid"); exit(1); }
}

//helper function to check that needed things are done when called. Prevents launches that could cause errors
void setupCheck()
{
    //set user var. Needs to be done first as using in a lot of places and will crash program if not set
    USER = getenv("SUDO_USER");

    //check to see if running as sudo
    if(!USER)
    {
        //not running as sudo user print error and kill script
        fprintf(stderr, "ERROR: Script not run with sudo. Please relaunch using sudo\n"); //logging
        exit(1);
    }

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
        fprintf(stderr, "ERROR: Could not find home directory of the non sudo user.\n");
        exit(1);
    }

    //check to see if we can get the configuration directory

    //get configuration path
    snprintf(CONFIGURATIONFOLDERPATH, sizeof(CONFIGURATIONFOLDERPATH), "%s/.config/mouse-remap", HOMEPATH);

    //confirm the path exists
    if (!opendir(CONFIGURATIONFOLDERPATH))
    {
        fprintf(stderr, "ERROR: Could not find configuration folder. Please create at the following location under your user profile: ~/.config/mouse-remap");
        exit(1);
    }

    //check to see if we can get the default configuration file

    //vars
    char defaultConfigurationFilePath[512];
    snprintf(defaultConfigurationFilePath, sizeof(defaultConfigurationFilePath), "%s/.config/mouse-remap/default.conf", HOMEPATH);
    struct stat buffer;
    
    if (stat(defaultConfigurationFilePath, &buffer) != 0)
    {
        //logging
        fprintf(stderr, "ERROR: Default configuration file does not exist. Please create one\n");
        exit(1);
    }

    //load saved settings
    loadSettings();

    //confirm that mouse name is not empty
    if(strcmp(MOUSE_NAME, "NOT SET - ERROR") == 0)
    {
        //logging
        fprintf(stderr, "ERROR: MOUSE_NAME var not set in settings file. Please set the name of the mouse to remap\n");

        //quit program
        exit(1);
    }
}