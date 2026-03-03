#include "../headers/vars.h"

//type for key remapping
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

char MOUSE_NAME[] = "Razer Razer Mouse Dock Pro"; //used to get the event ID of the device
const char *sudo_user = NULL;