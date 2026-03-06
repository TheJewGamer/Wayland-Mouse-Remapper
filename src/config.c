/* 
Author: TheJewGamer
Last Update: 3/6/2026
*/

//includes
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <pthread.h>
#include <dbus/dbus.h> 

//file imports
#include "../headers/config.h"
#include "../headers/bindings.h"
#include "../headers/vars.h"
#include "../headers/helpers.h"

//method to parse BUTTON_MAPPINGS from provided configuration file
void readConfig(FILE *configurationFileData)
{
    //vars
    int inLayerShiftSection = 0;
    char currentLine[256];

    //loop through each line in configuration file until no more lines
    while (fgets(currentLine, sizeof(currentLine), configurationFileData))
    {
        //check to see if line is an empty line or comment
        if (currentLine[0] == '#' || currentLine[0] == '\n')
        {
            //go to next while loop run
            continue;
        }

        //clean line of new line at end
        currentLine[strcspn(currentLine, "\n")] = 0;

        //check to see if we have hit layershift section
        if (strcmp(currentLine, "[layershift]") == 0)
        {
            //update var
            inLayerShiftSection = 1;

            //go to next while loop run
            continue;
        }

        //buffer vars for remapped keys
        char fromKey[64];
        char toKey[64];

        //try to split line on =
        if (sscanf(currentLine, "%63[^=]=%63s", fromKey, toKey) != 2)
        {
            //could not split so just skip this binding and go to next while loop run
            continue;
        }

        //check to see if line contains layer_shift_toggle button
        if (strcmp(fromKey, "LAYER_SHIFT_TOGGLE") == 0)
        {
            //dummy vars
            int type;
            int value;

            //set mouse toggle button to from key
            LAYER_TOGGLE_BUTTON = mouseKey(toKey, &type, &value);

            //go to next while loop run
            continue;
        }
        //check to see if line contains layer_shift_hold button
        if (strcmp(fromKey, "LAYER_SHIFT_HOLD") == 0)
        {
            //dummy vars
            int type;
            int value;

            //set mouse hold button to from key
            LAYER_HOLD_BUTTON = mouseKey(toKey, &type, &value);

            //go to next while loop run
            continue;
        }

        //vars
        struct buttonMapping currentMapping;
        currentMapping.from_code = mouseKey(fromKey, &currentMapping.from_type, &currentMapping.from_value); //convet to mouse key
        currentMapping.to_key = keyboardKey(toKey); //convert to key code
        currentMapping.layer_shifted = inLayerShiftSection; //set if in layershift

        //confirm valid key/mouse codes
        if (currentMapping.from_code < 0 || currentMapping.to_key < 0)
        {
            //logging
            fprintf(stderr, "ERROR: unknown mapping: %s=%s\n", fromKey, toKey);
            
            //go to next while loop run
            continue;
        }

        //check mappning array size and increase if needed
        if (BUTTON_MAPPINGS_AMOUNT >= BUTTON_MAPPING_ARRAY_SIZE)
        {
            //first run check
            if (BUTTON_MAPPING_ARRAY_SIZE == 0)
            {
                //set inital size
                BUTTON_MAPPING_ARRAY_SIZE = 4;
            }
            //not first run
            else
            {
                //double arrray size
                BUTTON_MAPPING_ARRAY_SIZE *= 2;
            }

            //resize mapping array
            BUTTON_MAPPINGS = realloc(BUTTON_MAPPINGS, BUTTON_MAPPING_ARRAY_SIZE * sizeof(struct buttonMapping));
        }
        //add mapping to array
        BUTTON_MAPPINGS[BUTTON_MAPPINGS_AMOUNT++] = currentMapping;
    }
}

//method to load the custom mapping configuration based on currently provided appName
void loadConfig(const char *appName)
{
    //clear mappings
    freeMappings();

    //logging
    printf("switching to app: %s\n", appName);

    //vars
    char configurationFilePath[512] = {0};

    //open configuration directory
    DIR *configurationFolder = opendir(CONFIGURATIONFOLDERPATH);

    //var to hold files in folder
    struct dirent *entry;

    //loop through every file configuration directory
    while ((entry = readdir(configurationFolder)) != NULL)
    {
        //skip default.conf
        if (strcmp(entry->d_name, "default.conf") == 0) continue;

        //check if appName is contained in filename of current configuration file
        if (strstr(entry->d_name, appName))
        {
            //build full path to configuration file if name matches
            snprintf(configurationFilePath, sizeof(configurationFilePath), "%s/.config/mouse-remap/%s", HOMEPATH, entry->d_name);

            //end this while loop
            break;
        }
    }
    //stop reading from configuration directory
    closedir(configurationFolder);

    //open configuration file
    FILE *configurationFileData = fopen(configurationFilePath, "r");

    //confirm not null
    if (!configurationFileData)
    {
        //load default configuration file as no matching configuration file for current focused app found
        snprintf(configurationFilePath, sizeof(configurationFilePath), "%s/.config/mouse-remap/default.conf", HOMEPATH);
        configurationFileData = fopen(configurationFilePath, "r");

        //logging
        printf("no config for %s, using default\n", appName);
    }
    //found matching configuration file
    else
    {
        //logging
        printf("loaded config for %s\n", configurationFilePath);
    }

    //parse configuration file
    readConfig(configurationFileData);

    //close configuration file
    fclose(configurationFileData);
}

//dbus listener method for KWIN script
void *windowListener(void *arg) //Note arg is needed here despite not being used.
{
    //vars
    DBusError err;
    dbus_error_init(&err);

    //logging
    printf("dbus thread started\n");

    //connect to user dbus session
    DBusConnection *windowListenerConnection = dbus_bus_get(DBUS_BUS_SESSION, &err);

    //check to see if connection is successful
    if (dbus_error_is_set(&err)) 
    {
        //errror on dbus connection

        //logging
        fprintf(stderr, "dbus connection error: %s\n", err.message);
        dbus_error_free(&err);
        return NULL;
    }

    //logging
    printf("dbus thread connected\n");

    //register custom dbus service
    dbus_bus_request_name(windowListenerConnection, DBUS_SERVICE, DBUS_NAME_FLAG_REPLACE_EXISTING, &err);
    
    //check to see if registeration was successful
    if (dbus_error_is_set(&err)) 
    {
        //error

        //logging
        fprintf(stderr, "dbus name error: %s\n", err.message);
        dbus_error_free(&err);
        return NULL;
    }

    //logging
    printf("dbubs thread registered\n");

    //infinite loop but sleeps until dbus gets a message. Will exit if dbus connection is lost though
    while (dbus_connection_read_write_dispatch(windowListenerConnection, -1)) 
    {
        printf("dbus message received\n");

        //var
        const char *appName = NULL;

        //get first message from the dbus by popping it
        DBusMessage *windowListenerInput = dbus_connection_pop_message(windowListenerConnection);

        //check to see if message was returned (needed to prevent crash in extreme cases)
        if (!windowListenerInput) 
        {
            //go to next while loop run
            continue;
        }

        //check to see if interface called using SetApp
        if (dbus_message_is_method_call(windowListenerInput, DBUS_IFACE, "SetApp")) 
        {
            //logging
            printf("SetApp message\n");

            //extract data from dbus message
            if (dbus_message_get_args(windowListenerInput, &err, DBUS_TYPE_STRING, &appName, DBUS_TYPE_INVALID)) 
            {
                //check to see if in persistentMode
                if(PERSISTENT_MODE == 0)
                {
                    //logging
                    printf("SetApp change app\n");

                    //lock mapping array when changing things
                    pthread_mutex_lock(&BUTTON_MAPPINGS_MUTEX);
                    
                    //load the new configuration
                    loadConfig(appName);

                    //unlock the mapping array as done
                    pthread_mutex_unlock(&BUTTON_MAPPINGS_MUTEX);
                }
            }
        }
        //check to see if interface called using set config
        else if(dbus_message_is_method_call(windowListenerInput, DBUS_IFACE, "SetConfig"))
        {
            //logging
            printf("SetConfig message\n");

            //extract data from dbus message
            if (dbus_message_get_args(windowListenerInput, &err, DBUS_TYPE_STRING, &appName, DBUS_TYPE_INVALID)) 
            {
                //logging
                printf("SetConfig change app\n");

                //lock mapping array when changing things
                pthread_mutex_lock(&BUTTON_MAPPINGS_MUTEX);
                
                loadConfig(appName);

                //unlock the mapping array as done
                pthread_mutex_unlock(&BUTTON_MAPPINGS_MUTEX);
            }
        }
        //check to see if interface called to set SetPersistentMode
        else if(dbus_message_is_method_call(windowListenerInput, DBUS_IFACE, "SetPersistentMode"))
        {
            //logging
            printf("SetPersistentMode message\n");

            //holder var
            int enabled = 0;

            //extract data from dbus message
            if (dbus_message_get_args(windowListenerInput, &err, DBUS_TYPE_BOOLEAN, &enabled, DBUS_TYPE_INVALID))
            {
                //logging
                printf("SetPersistentMode\n");

                //update var
                PERSISTENT_MODE = enabled;

                //logging
                printf("persistent mode: %s\n", PERSISTENT_MODE ? "on" : "off");
            }
        }

        //remove the current message garabge collection
        dbus_message_unref(windowListenerInput);
    }

    return NULL;
}