/* 
Author: TheJewGamer
Last Update: 3/1/2026
*/

//includes
#include <stdio.h> //printing
#include <stdlib.h> //exit/free
#include <string.h> //strings
#include <fcntl.h> //reading/open
#include <unistd.h> //file reading
#include <dirent.h> //directory reading
#include <pthread.h> //threads
#include <pwd.h> //user info
#include <grp.h> //groups
#include <time.h> //timing stuff
#include <dbus/dbus.h> //dbus
#include <linux/input.h> //input
#include <linux/uinput.h> //input

//GLOBAL VARS
#define MOUSE_NAME "" //used to get the event ID of the device
const char *sudo_user = NULL;

//dBus service vars (used to get input from the KWIN script)
#define DBUS_SERVICE "org.mouse.remap"
#define DBUS_PATH    "/org/mouse/remap"
#define DBUS_IFACE   "org.mouse.remap"

//format for key remapping
struct mapping {
    int from_type;
    int from_code;
    int from_value;
    int to_key;
    int layer_shifted; //bool for detecting layershift or not
};

//mapping vars
struct mapping *mappings = NULL; //mapping array
int mapping_count = 0; //number of active rebinds
int mapping_capacity = 0; //number of spaces in the mapping array. Grows and shrinks by power of 2 
pthread_mutex_t mappings_mutex = PTHREAD_MUTEX_INITIALIZER; //prevents mapping array being accessed by the dbubs thread and this program as same time. If that happend could crash

//layershift vars
int layer_toggle_button = -1;
int layer_hold_button = -1;
int layerShiftActive = 0;

//repeat vars
int repeat_key = -1;
int repeat_restart = 0;
pthread_mutex_t repeat_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t repeat_cond = PTHREAD_COND_INITIALIZER;

//register mouse keys
int mouseKey(const char *name, int *type, int *scrollDirection) {
    //var
    *scrollDirection = 0;
    
    //mouse buttons
    if (strcmp(name, "BTN_LEFT") == 0)         { *type = EV_KEY; return BTN_LEFT; }
    if (strcmp(name, "BTN_RIGHT") == 0)        { *type = EV_KEY; return BTN_RIGHT; }
    if (strcmp(name, "BTN_MIDDLE") == 0)       { *type = EV_KEY; return BTN_MIDDLE; }
    if (strcmp(name, "BTN_SIDE") == 0)         { *type = EV_KEY; return BTN_SIDE; }
    if (strcmp(name, "BTN_EXTRA") == 0)        { *type = EV_KEY; return BTN_EXTRA; }

    //mouse wheel
    if (strcmp(name, "REL_HWHEEL_LEFT") == 0)  { *type = EV_REL; *scrollDirection = -1; return REL_HWHEEL; }
    if (strcmp(name, "REL_HWHEEL_RIGHT") == 0) { *type = EV_REL; *scrollDirection = 1;  return REL_HWHEEL; }
    if (strcmp(name, "REL_WHEEL_UP") == 0)     { *type = EV_REL; *scrollDirection = 1;  return REL_WHEEL; }
    if (strcmp(name, "REL_WHEEL_DOWN") == 0)   { *type = EV_REL; *scrollDirection = -1; return REL_WHEEL; }

    //error
    return -1;
}

//register keyboard keys
int keyboardKey(const char *name) {
    //special keys
    if (strcmp(name, "KEY_ESC") == 0) return KEY_ESC;
    if (strcmp(name, "KEY_BACKSPACE") == 0) return KEY_BACKSPACE;
    if (strcmp(name, "KEY_TAB") == 0) return KEY_TAB;
    if (strcmp(name, "KEY_LEFTBRACE") == 0) return KEY_LEFTBRACE;
    if (strcmp(name, "KEY_RIGHTBRACE") == 0) return KEY_RIGHTBRACE;
    if (strcmp(name, "KEY_ENTER") == 0) return KEY_ENTER;
    if (strcmp(name, "KEY_SEMICOLON") == 0) return KEY_SEMICOLON;
    if (strcmp(name, "KEY_APOSTROPHE") == 0) return KEY_APOSTROPHE;
    if (strcmp(name, "KEY_GRAVE") == 0) return KEY_GRAVE; // backtik: `
    if (strcmp(name, "KEY_BACKSLASH") == 0) return KEY_BACKSLASH;
    if (strcmp(name, "KEY_COMMA") == 0) return KEY_COMMA;
    if (strcmp(name, "KEY_DOT") == 0) return KEY_DOT;
    if (strcmp(name, "KEY_SLASH") == 0) return KEY_SLASH;
    if (strcmp(name, "KEY_SPACE") == 0) return KEY_SPACE;
    if (strcmp(name, "KEY_HOME") == 0) return KEY_HOME;
    if (strcmp(name, "KEY_PAGEUP") == 0) return KEY_PAGEUP;
    if (strcmp(name, "KEY_END") == 0) return KEY_END;
    if (strcmp(name, "KEY_PAGEDOWN") == 0) return KEY_PAGEDOWN;
    if (strcmp(name, "KEY_INSERT") == 0) return KEY_INSERT;
    if (strcmp(name, "KEY_DELETE") == 0) return KEY_DELETE;

    //modifier keys
    if (strcmp(name, "KEY_CAPSLOCK") == 0) return KEY_CAPSLOCK;
    if (strcmp(name, "KEY_NUMLOCK") == 0) return KEY_NUMLOCK;
    if (strcmp(name, "KEY_SCROLLLOCK") == 0) return KEY_SCROLLLOCK;
    if (strcmp(name, "KEY_LEFTSHIFT") == 0) return KEY_LEFTSHIFT;
    if (strcmp(name, "KEY_RIGHTSHIFT") == 0) return KEY_RIGHTSHIFT;
    if (strcmp(name, "KEY_LEFTCTRL") == 0) return KEY_LEFTCTRL;
    if (strcmp(name, "KEY_RIGHTCTRL") == 0) return KEY_RIGHTCTRL;
    if (strcmp(name, "KEY_LEFTALT") == 0) return KEY_LEFTALT;
    if (strcmp(name, "KEY_RIGHTALT") == 0) return KEY_RIGHTALT;
    if (strcmp(name, "KEY_LEFTMETA") == 0) return KEY_LEFTMETA; //windows key
    if (strcmp(name, "KEY_RIGHTMETA") == 0) return KEY_RIGHTMETA; //menu key

    //number row
    if (strcmp(name, "KEY_1") == 0) return KEY_1;
    if (strcmp(name, "KEY_2") == 0) return KEY_2;
    if (strcmp(name, "KEY_3") == 0) return KEY_3;
    if (strcmp(name, "KEY_4") == 0) return KEY_4;
    if (strcmp(name, "KEY_5") == 0) return KEY_5;
    if (strcmp(name, "KEY_6") == 0) return KEY_6;
    if (strcmp(name, "KEY_7") == 0) return KEY_7;
    if (strcmp(name, "KEY_8") == 0) return KEY_8;
    if (strcmp(name, "KEY_9") == 0) return KEY_9;
    if (strcmp(name, "KEY_0") == 0) return KEY_0;
    if (strcmp(name, "KEY_MINUS") == 0) return KEY_MINUS;
    if (strcmp(name, "KEY_EQUAL") == 0) return KEY_EQUAL;

    //letters
    if (strcmp(name, "KEY_Q") == 0) return KEY_Q;
    if (strcmp(name, "KEY_W") == 0) return KEY_W;
    if (strcmp(name, "KEY_E") == 0) return KEY_E;
    if (strcmp(name, "KEY_R") == 0) return KEY_R;
    if (strcmp(name, "KEY_T") == 0) return KEY_T;
    if (strcmp(name, "KEY_Y") == 0) return KEY_Y;
    if (strcmp(name, "KEY_U") == 0) return KEY_U;
    if (strcmp(name, "KEY_I") == 0) return KEY_I;
    if (strcmp(name, "KEY_O") == 0) return KEY_O;
    if (strcmp(name, "KEY_P") == 0) return KEY_P;
    if (strcmp(name, "KEY_A") == 0) return KEY_A;
    if (strcmp(name, "KEY_S") == 0) return KEY_S;
    if (strcmp(name, "KEY_D") == 0) return KEY_D;
    if (strcmp(name, "KEY_F") == 0) return KEY_F;
    if (strcmp(name, "KEY_G") == 0) return KEY_G;
    if (strcmp(name, "KEY_H") == 0) return KEY_H;
    if (strcmp(name, "KEY_J") == 0) return KEY_J;
    if (strcmp(name, "KEY_K") == 0) return KEY_K;
    if (strcmp(name, "KEY_L") == 0) return KEY_L;
    if (strcmp(name, "KEY_Z") == 0) return KEY_Z;
    if (strcmp(name, "KEY_X") == 0) return KEY_X;
    if (strcmp(name, "KEY_C") == 0) return KEY_C;
    if (strcmp(name, "KEY_V") == 0) return KEY_V;
    if (strcmp(name, "KEY_B") == 0) return KEY_B;
    if (strcmp(name, "KEY_N") == 0) return KEY_N;
    if (strcmp(name, "KEY_M") == 0) return KEY_M;

    //function keys
    if (strcmp(name, "KEY_F1") == 0) return KEY_F1;
    if (strcmp(name, "KEY_F2") == 0) return KEY_F2;
    if (strcmp(name, "KEY_F3") == 0) return KEY_F3;
    if (strcmp(name, "KEY_F4") == 0) return KEY_F4;
    if (strcmp(name, "KEY_F5") == 0) return KEY_F5;
    if (strcmp(name, "KEY_F6") == 0) return KEY_F6;
    if (strcmp(name, "KEY_F7") == 0) return KEY_F7;
    if (strcmp(name, "KEY_F8") == 0) return KEY_F8;
    if (strcmp(name, "KEY_F9") == 0) return KEY_F9;
    if (strcmp(name, "KEY_F10") == 0) return KEY_F10;
    if (strcmp(name, "KEY_F11") == 0) return KEY_F11;
    if (strcmp(name, "KEY_F12") == 0) return KEY_F12;

    //extra function keys (these are added so extra mouse buttons can be used. By default mouses can only have two side buttons so can mark extra buttons as these)
    if (strcmp(name, "KEY_F13") == 0) return KEY_F13;
    if (strcmp(name, "KEY_F14") == 0) return KEY_F14;
    if (strcmp(name, "KEY_F15") == 0) return KEY_F15;
    if (strcmp(name, "KEY_F16") == 0) return KEY_F16;
    if (strcmp(name, "KEY_F17") == 0) return KEY_F17;
    if (strcmp(name, "KEY_F18") == 0) return KEY_F18;
    if (strcmp(name, "KEY_F19") == 0) return KEY_F19;
    if (strcmp(name, "KEY_F20") == 0) return KEY_F20;
    if (strcmp(name, "KEY_F21") == 0) return KEY_F21;
    if (strcmp(name, "KEY_F22") == 0) return KEY_F22;
    if (strcmp(name, "KEY_F23") == 0) return KEY_F23;
    if (strcmp(name, "KEY_F24") == 0) return KEY_F24;

    //numpad
    if (strcmp(name, "KEY_KP0") == 0) return KEY_KP0;
    if (strcmp(name, "KEY_KP1") == 0) return KEY_KP1;
    if (strcmp(name, "KEY_KP2") == 0) return KEY_KP2;
    if (strcmp(name, "KEY_KP3") == 0) return KEY_KP3;
    if (strcmp(name, "KEY_KP4") == 0) return KEY_KP4;
    if (strcmp(name, "KEY_KP5") == 0) return KEY_KP5;
    if (strcmp(name, "KEY_KP6") == 0) return KEY_KP6;
    if (strcmp(name, "KEY_KP7") == 0) return KEY_KP7;
    if (strcmp(name, "KEY_KP8") == 0) return KEY_KP8;
    if (strcmp(name, "KEY_KP9") == 0) return KEY_KP9;
    if (strcmp(name, "KEY_KPMINUS") == 0) return KEY_KPMINUS;
    if (strcmp(name, "KEY_KPPLUS") == 0) return KEY_KPPLUS;
    if (strcmp(name, "KEY_KPDOT") == 0) return KEY_KPDOT;
    if (strcmp(name, "KEY_KPENTER") == 0) return KEY_KPENTER;
    if (strcmp(name, "KEY_KPSLASH") == 0) return KEY_KPSLASH;
    if (strcmp(name, "KEY_KPASTERISK") == 0) return KEY_KPASTERISK;

    //arrow keys
    if (strcmp(name, "KEY_UP") == 0) return KEY_UP;
    if (strcmp(name, "KEY_LEFT") == 0) return KEY_LEFT;
    if (strcmp(name, "KEY_RIGHT") == 0) return KEY_RIGHT;
    if (strcmp(name, "KEY_DOWN") == 0) return KEY_DOWN;

    //macros actions
    if (strcmp(name, "COPY_MACRO") == 0) return -2;
    if (strcmp(name, "PASTE_MACRO") == 0) return -3;

    //error
    return -1;
}

//helper method to send inputs to virtual device
void emit(int fd, int type, int code, int value) 
{
    struct input_event ev = {0};
    ev.type = type;
    ev.code = code;
    ev.value = value;
    write(fd, &ev, sizeof(ev));
}

//helper method to simulate key presses
void send_key(int fd, int key) 
{
    emit(fd, EV_KEY, key, 1); //key down
    emit(fd, EV_SYN, SYN_REPORT, 0); //key down complete
    emit(fd, EV_KEY, key, 0); //key up
    emit(fd, EV_SYN, SYN_REPORT, 0); //key up complete
}

void doMacro(int fd, int macro)
{
    //copy macro
    if (macro == -2)
    {
        emit(fd, EV_KEY, KEY_LEFTCTRL, 1);
        emit(fd, EV_KEY, KEY_C, 1);
        emit(fd, EV_SYN, SYN_REPORT, 0);
        emit(fd, EV_KEY, KEY_C, 0);
        emit(fd, EV_KEY, KEY_LEFTCTRL, 0);
        emit(fd, EV_SYN, SYN_REPORT, 0);
    }
    //paste macro
    else if (macro == -3)
    {
        emit(fd, EV_KEY, KEY_LEFTCTRL, 1);
        emit(fd, EV_KEY, KEY_V, 1);
        emit(fd, EV_SYN, SYN_REPORT, 0);
        emit(fd, EV_KEY, KEY_V, 0);
        emit(fd, EV_KEY, KEY_LEFTCTRL, 0);
        emit(fd, EV_SYN, SYN_REPORT, 0);
    }
}

//TODO: Look through this confirm var names and other stuff is all good
void *repeat_thread(void *arg)
{
    int fd = *(int *)arg;
    struct timespec ts;

    while (1)
    {
        pthread_mutex_lock(&repeat_mutex);

        //sleep until a key is pressed
        while (repeat_key == -1)
            pthread_cond_wait(&repeat_cond, &repeat_mutex);

        int key = repeat_key;
        repeat_restart = 0;
        pthread_mutex_unlock(&repeat_mutex);

        //send initial key press immediately
        send_key(fd, key);

        //initial delay using timed wait so it can be interrupted
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_nsec += 500000000; //500ms
        if (ts.tv_nsec >= 1000000000) { ts.tv_sec++; ts.tv_nsec -= 1000000000; }

        pthread_mutex_lock(&repeat_mutex);
        pthread_cond_timedwait(&repeat_cond, &repeat_mutex, &ts);
        pthread_mutex_unlock(&repeat_mutex);

        //repeat until button released or restarted
        while (repeat_key == key && !repeat_restart)
        {
            send_key(fd, key);

            clock_gettime(CLOCK_REALTIME, &ts);
            ts.tv_nsec += 33000000; //33ms
            if (ts.tv_nsec >= 1000000000) { ts.tv_sec++; ts.tv_nsec -= 1000000000; }

            pthread_mutex_lock(&repeat_mutex);
            pthread_cond_timedwait(&repeat_cond, &repeat_mutex, &ts);
            pthread_mutex_unlock(&repeat_mutex);
        }
    }
    return NULL;
}

//get mouse event id by name. Needed as event ids can change on reboot
char* getMouseEventID(const char *mouseName)
{
    //var
    static char mouseFilePath[64];

    //open input devices file
    FILE *inputDevicesFile = fopen("/proc/bus/input/devices", "r");

    //confirm that file was opened successfully
    if (!inputDevicesFile)
    { 
        //logging
        perror("open /proc/bus/input/devices"); 
        
        //stop script
        exit(1); 
    }

    //vars
    char currentLine[256];
    int foundMouse = 0; //bool

    //loop through each line in file
    while (fgets(currentLine, sizeof(currentLine), inputDevicesFile))
    {
        //check to see if current line contains mouseName
        if (strstr(currentLine, mouseName))
        {
            //it did update var
            foundMouse = 1;
        }

        //check to see if we found the mouseName line
        if (foundMouse)
        {
            //find the handler line
            if(strstr(currentLine, "Handlers="))
            {
                //find the event line and extract device path. IE: /dev/input/eventX
                char *event = strstr(currentLine, "event");

                //var to hold event number
                int eventNum;
                
                //extract number after the word 'event'
                sscanf(event, "event%d", &eventNum);

                //build full device path
                snprintf(mouseFilePath, sizeof(mouseFilePath), "/dev/input/event%d", eventNum);
                
                //close device file
                fclose(inputDevicesFile);

                //return device path
                return mouseFilePath;
            }
        }
    }

    //Did not find mouse based on name
    fclose(inputDevicesFile); //close file
    fprintf(stderr, "ERROR: could not find mouse device: %s\n", mouseName); //Loggins
    exit(1); //stop script
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

//dbus listener method for KWIN script
void *dbus_thread(void *arg) 
{
    //vars
    DBusError err;
    dbus_error_init(&err);

    //connect to user dbus session
    DBusConnection *conn = dbus_bus_get(DBUS_BUS_SESSION, &err);

    //check to see if connection is successful
    if (dbus_error_is_set(&err)) 
    {
        //errror on dbus connection

        //logging
        fprintf(stderr, "dbus connection error: %s\n", err.message);
        dbus_error_free(&err);
        return NULL;
    }

    //register custom dbus service
    int ret = dbus_bus_request_name(conn, DBUS_SERVICE, DBUS_NAME_FLAG_REPLACE_EXISTING, &err);
    
    //check to see if registeration was successful
    if (dbus_error_is_set(&err)) 
    {
        //error

        //logging
        fprintf(stderr, "dbus name error: %s\n", err.message);
        dbus_error_free(&err);
        return NULL;
    }

    //infinite loop but sleeps until dbus gets a message. Will exit if dbus connection is lost though
    while (dbus_connection_read_write_dispatch(conn, -1)) 
    {
        //get first message from the dbus by popping it
        DBusMessage *msg = dbus_connection_pop_message(conn);

        //check to see if message was returned (needed to prevent crash in extreme cases)
        if (!msg) 
        {
            continue;
        }

        //confirm message is on the correct interface
        if (dbus_message_is_method_call(msg, DBUS_IFACE, "SetApp")) 
        {
            //var
            const char *appName = NULL;

            //extract data from dbus message
            if (dbus_message_get_args(msg, &err, DBUS_TYPE_STRING, &appName, DBUS_TYPE_INVALID)) 
            {
                //logging
                printf("switching to app: %s\n", appName);

                //lock mapping array when changing things
                pthread_mutex_lock(&mappings_mutex);
                
                //load the new configuration
                freeMappings();
                loadConfig(appName);

                //unlock the mapping array as done
                pthread_mutex_unlock(&mappings_mutex);
            }
        }
        //remove the current message garabge collection
        dbus_message_unref(msg);
    }

    return NULL;
}

//TODO make this work better as messes with mouse settings
int setup_uinput(int src_fd) 
{
    //vars for virtual mouse device
    int fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    if (fd < 0)
    { 
        //logging ERROR
        perror("open /dev/uinput");
        
        //stop script
        exit(1); 
    }

    //register event types
    ioctl(fd, UI_SET_EVBIT, EV_KEY); //button presses
    ioctl(fd, UI_SET_EVBIT, EV_REL); //movement
    ioctl(fd, UI_SET_EVBIT, EV_SYN); //sync events

    //register standard mouse buttons
    ioctl(fd, UI_SET_KEYBIT, BTN_LEFT);
    ioctl(fd, UI_SET_KEYBIT, BTN_RIGHT);
    ioctl(fd, UI_SET_KEYBIT, BTN_MIDDLE);
    ioctl(fd, UI_SET_KEYBIT, BTN_SIDE);
    ioctl(fd, UI_SET_KEYBIT, BTN_EXTRA);

    //register all keyboard keys
    for (int i = KEY_ESC; i < KEY_MAX; i++)
    {
        ioctl(fd, UI_SET_KEYBIT, i);
    }

    //register mouse wheel and relvant axies
    ioctl(fd, UI_SET_RELBIT, REL_X);
    ioctl(fd, UI_SET_RELBIT, REL_Y);
    ioctl(fd, UI_SET_RELBIT, REL_WHEEL);
    ioctl(fd, UI_SET_RELBIT, REL_HWHEEL);
    
    //set as virtual pointer device
    ioctl(fd, UI_SET_PROPBIT, INPUT_PROP_POINTER);
    ioctl(fd, UI_SET_PROPBIT, INPUT_PROP_DIRECT);

    //create virtual mouse
    struct uinput_setup usetup = {0};
    usetup.id.bustype = BUS_VIRTUAL;
    usetup.id.vendor  = 0x1234;
    usetup.id.product = 0x5678;
    strcpy(usetup.name, "virtual-mouse");

    ioctl(fd, UI_DEV_SETUP, &usetup);
    ioctl(fd, UI_DEV_CREATE);

    //done
    return fd;
}

//main
int main() 
{
    //check to see if running as sudo
    if(!getenv("SUDO_USER"))
    {
        //not running as sudo user print error and kill script
        fprintf(stderr, "Script not run with sudo. Please relaunch using sudo"); //logging
        exit(1);
    }

    //set sudo_user var
    sudo_user = getenv("SUDO_USER");

    //load default config
    loadConfig("default");

    //open/grab mouse as root
    char *mouseDevice = getMouseEventID(MOUSE_NAME);
    printf("found mouse at %s\n", mouseDevice);
    int src_fd = open(mouseDevice, O_RDONLY);

    //set up virtual device as root
    int out_fd = setup_uinput(src_fd);

    //drop privileges from sudo to real user
    dropPrivileges();

    //start dbus thread as standard user
    pthread_t tid;
    pthread_create(&tid, NULL, dbus_thread, NULL);

    //start repeat thread as normal user
    pthread_t repeat_tid;
    pthread_create(&repeat_tid, NULL, repeat_thread, &out_fd);

    //holds incoming input
    struct input_event ev;

    //read input from actual mouse
    while (read(src_fd, &ev, sizeof(ev)) == sizeof(ev)) 
    {
        //var for checking if current key is remapped or not
        int remapped = 0;

        //check to see if layershift toggle button is pressed
        if (ev.type == EV_KEY && ev.code == layer_toggle_button)
        {
            //key down only
            if (ev.value == 1)
            {
                //invert var
                layerShiftActive = !layerShiftActive;

                //stop repeating current key TODO not sure if I should keep this or not
                pthread_mutex_lock(&repeat_mutex);
                repeat_key = -1;
                pthread_cond_signal(&repeat_cond);
                pthread_mutex_unlock(&repeat_mutex);
            }
            
            //end this loop run
            continue;
        }

        //check to see if layershift hold button is pressed
        if (ev.type == EV_KEY && ev.code == layer_hold_button)
        {
            //active while held
            layerShiftActive = ev.value;

            //stop repeating current key TODO not sure if I should keep this or not
            pthread_mutex_lock(&repeat_mutex);
            repeat_key = -1;
            pthread_cond_signal(&repeat_cond);
            pthread_mutex_unlock(&repeat_mutex);

            //end this loop run here
            continue;
        }

        //lock so configuration cannot be swapped
        pthread_mutex_lock(&mappings_mutex);

        //loop through all active mappings
        for (int i = 0; i < mapping_count; i++) 
        {
            //var
            struct mapping *currentMapping = &mappings[i];

            //skip mappings that are not part of current layer
            if (currentMapping->layer_shifted != layerShiftActive) continue;

            //check to see if current key event is currently remapped
            if (ev.type == EV_KEY && currentMapping->from_type == EV_KEY && ev.code == currentMapping->from_code) 
            {
                //key down
                if (ev.value == 1)
                {
                    //check to see if marco. We use negatives for marco so easy to check
                    if (currentMapping->to_key < 0)
                    {
                        //run marco
                        doMacro(out_fd, currentMapping->to_key);
                    }
                    //not an marco
                    else
                    {
                        //start repeating
                        pthread_mutex_lock(&repeat_mutex);
                        repeat_key = currentMapping->to_key;
                        repeat_restart = 1;
                        pthread_cond_signal(&repeat_cond);
                        pthread_mutex_unlock(&repeat_mutex);
                    }
                }
                //key up
                else if (ev.value == 0)
                {
                    //button released stop repeating
                    pthread_mutex_lock(&repeat_mutex);
                    repeat_key = -1;
                    pthread_mutex_unlock(&repeat_mutex);
                }

                //update var
                remapped = 1;

                //end loop
                break;
            }

            //check to see if scroll event and is currently remapped
            if (ev.type == EV_REL && currentMapping->from_type == EV_REL && ev.code == currentMapping->from_code && ev.value == currentMapping->from_value) 
            {
                //matches so get remap key and send it
                send_key(out_fd, currentMapping->to_key);
                remapped = 1; //update var
                break; //done
            }
        }
        //unlock configuration can be swapped
        pthread_mutex_unlock(&mappings_mutex);

        //check to see if key was remapped or not
        if (!remapped)
        {
            //was not remapped so just send the key event as normal to the virtual device
            write(out_fd, &ev, sizeof(ev));
        }
    }

    //clean up program and exit if input device is lost
    ioctl(out_fd, UI_DEV_DESTROY);
    close(out_fd);
    close(src_fd);
    freeMappings();
    return 0;
}
