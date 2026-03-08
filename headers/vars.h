/* 
Author: TheJewGamer
Last Update: 3/8/2026
*/

//dBus service vars (used to get input from the KWIN script)
#define DBUS_SERVICE "org.mouse.remap"
#define DBUS_PATH    "/org/mouse/remap"
#define DBUS_IFACE   "org.mouse.remap"

//debug var
#define DEBUG 0

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

//mouse vars
extern const char *USER;
extern int VIRTUALMOUSE;
extern int MOUSEDEVICEFILE;
extern int MOUSEKEYBOARDFILE;

//path vars
extern const char *HOMEPATH;
extern char CONFIGURATIONFOLDERPATH[512];

//settings files vars
extern int PERSISTENT_MODE;
extern char PERSISTENT_PROFILE[256];
extern char MOUSE_PHYS[256];
extern char MOUSE_KEYBOARD_PHYS[256];