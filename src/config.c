//file imports
#include "../headers/config.h"
#include "../headers/bindings.h"
#include "../headers/vars.h"
#include "../headers/helpers.h"

//method to parse mappings from provided configuration file
void readConfig(FILE *configurationFileData)
{
    //vars
    int inLayerSection = 0;
    char line[256];

    //loop through each line in configuration file until no more lines
    while (fgets(line, sizeof(line), configurationFileData))
    {
        //check to see if line is an empty line or comment
        if (line[0] == '#' || line[0] == '\n')
        {
            //skip if new line or comment
            continue;
        }

        //clean line of new line at end
        line[strcspn(line, "\n")] = 0;

        //check to see if we have hit layershift section
        if (strcmp(line, "[layershift]") == 0)
        {
            //update var
            inLayerSection = 1;

            //end this loop run
            continue;
        }

        //buffer vars for key mappings
        char from[64];
        char to[64];

        //try to split line on =
        if (sscanf(line, "%63[^=]=%63s", from, to) != 2)
        {
            //could not split so end this loop run
            continue;
        }

        //check to see if line contains layer_shift_toggle button
        if (strcmp(from, "LAYER_SHIFT_TOGGLE") == 0)
        {
            //vars
            int type;
            int value;
            layer_toggle_button = mouseKey(to, &type, &value);

            //end this loop run
            continue;
        }
        //check to see if line contains layer_shift_hold button
        if (strcmp(from, "LAYER_SHIFT_HOLD") == 0)
        {
            int type, value;
            layer_hold_button = mouseKey(to, &type, &value);

            //end this loop run
            continue;
        }

        //vars
        struct mapping currentMapping;
        currentMapping.from_code = mouseKey(from, &currentMapping.from_type, &currentMapping.from_value); //convet to mouse key
        currentMapping.to_key = keyboardKey(to); //convert to key code
        currentMapping.layer_shifted = inLayerSection; //set if in layershift

        //confirm valid key/mouse codes
        if (currentMapping.from_code < 0 || currentMapping.to_key < 0)
        {
            //logging
            fprintf(stderr, "ERROR: unknown mapping: %s=%s\n", from, to);
            
            //end this loop run
            continue;
        }

        //check mappning array size and increase if needed
        if (mapping_count >= mapping_capacity)
        {
            //first run check
            if (mapping_capacity == 0)
            {
                //set inital size
                mapping_capacity = 4;
            }
            //not first run
            else
            {
                //double arrray size
                mapping_capacity *= 2;
            }

            //resize mapping array
            mappings = realloc(mappings, mapping_capacity * sizeof(struct mapping));
        }
        //add mapping to array
        mappings[mapping_count++] = currentMapping;
    }
}

//method to load the custom mapping configuration based on currently provided appName
void loadConfig(const char *appName)
{
    //vars
    const char *homePath = getHome();
    char configurationPathBuffer[512];
    char configurationFilePath[512] = {0};

    //get configuration path
    snprintf(configurationPathBuffer, sizeof(configurationPathBuffer), "%s/.config/mouse-remap", homePath);

    //open configuration directory
    DIR *configurationDirPath = opendir(configurationPathBuffer);

    //confirm configuration directory was found
    if (configurationDirPath)
    {
        struct dirent *entry;

        //loop through every file configuration directory
        while ((entry = readdir(configurationDirPath)) != NULL)
        {
            //skip default.conf
            if (strcmp(entry->d_name, "default.conf") == 0) continue;

            //check if appName is contained in filename of current configuration file
            if (strstr(entry->d_name, appName))
            {
                //build full path to configuration file if name matches
                snprintf(configurationFilePath, sizeof(configurationFilePath), "%s/.config/mouse-remap/%s", homePath, entry->d_name);
                break;
            }
        }
        //stop reading from configuration directory
        closedir(configurationDirPath);
    }

    //open configuration file
    FILE *configurationFileData = fopen(configurationFilePath, "r");

    //confirm not null
    if (!configurationFileData)
    {
        //load default configuration file as no matching configuration file for current focused app found
        snprintf(configurationFilePath, sizeof(configurationFilePath), "%s/.config/mouse-remap/default.conf", homePath);
        configurationFileData = fopen(configurationFilePath, "r");

        //confirm that default configuration file exists. TODO: Remove this as scritp should check for this in the begining not every run
        if (!configurationFileData)
        {
            //logging
            fprintf(stderr, "no config found for %s and no default.conf\n", appName);
            return;
        }
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