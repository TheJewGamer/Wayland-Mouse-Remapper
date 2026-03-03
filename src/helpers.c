#include "../headers/helpers.h"
#include "../headers/vars.h"

//helper method to get the home file path for the user. This is needed as this script is run using sudo
const char *getHome()
{   
    //get user info by looking up the password database (password are not stored here so safe)
    struct passwd *userInfo = getpwnam(sudo_user);
    if (userInfo) 
    {
        //get home dir of user that called sudo
        return userInfo->pw_dir;
    }
    //error should never run because of return statement above so fine
    else
    {   
        //logging
        fprintf(stderr, "ERROR: could not find home directory of the non sudo user.\n");
        exit(1);
    }
    
}

//method to clean the array and other vars. Garbage collection
void freeMappings()
{
    free(mappings);
    mappings = NULL;
    mapping_count = 0;
    mapping_capacity = 0;
    layer_toggle_button = -1;
    layer_hold_button = -1;
    layerShiftActive = 0;
}

//helper method to drop privileges back to user who called sudo
void dropPrivileges()
{
    //var
    struct passwd *userInfo = getpwnam(sudo_user);

    //drop supplementary groups gid then uid
    if (initgroups(sudo_user, userInfo->pw_gid) < 0) { perror("initgroups"); exit(1); }
    if (setgid(userInfo->pw_gid) < 0) { perror("setgid"); exit(1); }
    if (setuid(userInfo->pw_uid) < 0) { perror("setuid"); exit(1); }
}