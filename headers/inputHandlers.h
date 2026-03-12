/* 
Author: TheJewGamer
Last Update: 3/11/2026
*/

//standard includes
#include <unistd.h>
#include <pthread.h>
#include <linux/input.h>
#include <linux/uinput.h>

//other files
#include "bindings.h"
#include "vars.h"

//method to send inputs to virtual device
void sendInput(int fd, int type, int code, int value);

//method to simulate key presses
void send_key(int fd, int key);

//method to send macros
void doMacro(int fd, int macro);

void processEvent(struct input_event ev);

//thread to read input
void *inputReader(void *deviceFileInput);