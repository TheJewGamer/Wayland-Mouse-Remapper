/* 
Author: TheJewGamer
Last Update: 3/5/2026
*/

#ifndef VARS_H
#define VARS_H

//includes
#include <pthread.h> //threads
#include <linux/input.h> //input

//dBus service vars (used to get input from the KWIN script)
#define DBUS_SERVICE "org.mouse.remap"
#define DBUS_PATH    "/org/mouse/remap"
#define DBUS_IFACE   "org.mouse.remap"

//type for key remapping
struct mapping {
    int from_type;
    int from_code;
    int from_value;
    int to_key;
    int layer_shifted; //bool for detecting layershift or not
};

//mapping vars
extern struct mapping *mappings; //mapping array
extern int mapping_count; //number of active rebinds
extern int mapping_capacity; //number of spaces in the mapping array. Grows and shrinks by power of 2 
extern pthread_mutex_t mappings_mutex; //prevents mapping array being accessed by the dbubs thread and this program as same time. If that happend could crash

//layershift vars
extern int layer_toggle_button;
extern int layer_hold_button;
extern int layerShiftActive;

//misc vars
extern char MOUSE_NAME[];
extern const char *USER;

//path vars
extern const char *HOMEPATH;
extern char CONFIGURATIONFOLDERPATH[512];

//done
#endif