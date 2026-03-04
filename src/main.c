/* 
Author: TheJewGamer
Last Update: 3/4/2026
*/

//includes
#include <stdio.h> //printing
#include <stdlib.h> //exit/free
#include <string.h> //strings
#include <fcntl.h> //reading/open
#include <unistd.h> //file reading
#include <dirent.h> //directory reading
#include <pwd.h> //user info
#include <grp.h> //groups
#include <time.h> //timing stuff
#include <dbus/dbus.h> //dbus
#include <linux/uinput.h> //input

//file imports
#include "../headers/vars.h"
#include "../headers/config.h"
#include "../headers/helpers.h"

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


//dbus listener method for KWIN script
void *windowListener(void *arg) //TODO: double check why the arg is needed here despite not being used
{
    //vars
    DBusError err;
    dbus_error_init(&err);

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

    //infinite loop but sleeps until dbus gets a message. Will exit if dbus connection is lost though
    while (dbus_connection_read_write_dispatch(windowListenerConnection, -1)) 
    {
        //get first message from the dbus by popping it
        DBusMessage *windowListenerInput = dbus_connection_pop_message(windowListenerConnection);

        //check to see if message was returned (needed to prevent crash in extreme cases)
        if (!windowListenerInput) 
        {
            continue;
        }

        //confirm message is on the correct interface
        if (dbus_message_is_method_call(windowListenerInput, DBUS_IFACE, "SetApp")) 
        {
            //var
            const char *appName = NULL;

            //extract data from dbus message
            if (dbus_message_get_args(windowListenerInput, &err, DBUS_TYPE_STRING, &appName, DBUS_TYPE_INVALID)) 
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
        dbus_message_unref(windowListenerInput);
    }

    return NULL;
}

//TODO make this work better as messes with mouse settings
int setup_uinput() 
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
    //inital preq check
    setupCheck();

    //load default config
    loadConfig("default");

    //open/grab mouse as root
    char *mouseDevice = getMouseEventID(MOUSE_NAME);
    printf("found mouse at %s\n", mouseDevice);
    int src_fd = open(mouseDevice, O_RDONLY);

    //set up virtual device as root
    int out_fd = setup_uinput();

    //drop privileges from sudo to real user
    dropPrivileges();

    //start dubs thread for window listening as  user
    pthread_t tid;
    pthread_create(&tid, NULL, windowListener, NULL);

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
