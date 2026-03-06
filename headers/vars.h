/* 
Author: TheJewGamer
Last Update: 3/6/2026
*/

#ifndef VARS_H
#define VARS_H

//dBus service vars (used to get input from the KWIN script)
#define DBUS_SERVICE "org.mouse.remap"
#define DBUS_PATH    "/org/mouse/remap"
#define DBUS_IFACE   "org.mouse.remap"

//type for key remapping
struct buttonMapping {
    int from_type;
    int from_code;
    int from_value;
    int to_key;
    int layer_shifted; //bool for detecting layershift or not
};

//mapping vars
extern struct buttonMapping *BUTTON_MAPPINGS; //mapping array
extern int BUTTON_MAPPINGS_AMOUNT; //number of active rebinds
extern int BUTTON_MAPPING_ARRAY_SIZE; //number of spaces in the mapping array. Grows and shrinks by power of 2 
extern pthread_mutex_t BUTTON_MAPPINGS_MUTEX; //prevents mapping array being accessed by the dbubs thread and this program as same time. If that happend could crash

//layershift vars
extern int LAYER_TOGGLE_BUTTON;
extern int LAYER_HOLD_BUTTON;
extern int LAYER_SHIFT_ACTIVE;
extern int HELD_KEY;

//misc vars
extern char MOUSE_NAME[];
extern const char *USER;

//path vars
extern const char *HOMEPATH;
extern char CONFIGURATIONFOLDERPATH[512];

//configuration locking vars
extern int PERSISTENT_MODE;

//done
#endif