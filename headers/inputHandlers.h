/* 
Author: TheJewGamer
Last Update: 3/6/2026
*/

//method to send inputs to virtual device
void sendInput(int fd, int type, int code, int value);

//method to simulate key presses
void send_key(int fd, int key);

//method to send macros
void doMacro(int fd, int macro);