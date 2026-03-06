/* 
Author: TheJewGamer
Last Update: 3/6/2026
*/

//includes
#include <pthread.h>
#include <linux/input.h>

//other files
#include "../headers/vars.h"

//type for key remapping
struct buttonMapping {
    int from_type;
    int from_code;
    int from_value;
    int to_key;
    int layer_shifted; //bool for detecting layershift or not
};

//mapping vars
struct buttonMapping *BUTTON_MAPPINGS = NULL; //mapping array
int BUTTON_MAPPINGS_AMOUNT = 0; //number of active rebinds
int BUTTON_MAPPING_ARRAY_SIZE = 0; //number of spaces in the mapping array. Grows and shrinks by power of 2 
pthread_mutex_t BUTTON_MAPPINGS_MUTEX = PTHREAD_MUTEX_INITIALIZER; //prevents mapping array being accessed by the dbubs thread and this program as same time. If that happend could crash

//layershift vars
int LAYER_TOGGLE_BUTTON = -1;
int LAYER_HOLD_BUTTON = -1;
int LAYER_SHIFT_ACTIVE = 0;
int HELD_KEY = -1;

//misc vars
char MOUSE_NAME[] = "Razer Razer Mouse Dock Pro"; //used to get the event ID of the device TODO: update this so that it can be changed by the UI in the future
const char *USER = NULL;

//path vars
const char *HOMEPATH = NULL;
char CONFIGURATIONFOLDERPATH[512] = "";

//configuration locking var
int PERSISTENT_MODE = 0;