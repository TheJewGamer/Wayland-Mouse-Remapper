/* 
Author: TheJewGamer
Last Update: 3/11/2026
*/

//includes
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <linux/input.h>

//functions
int mouseKey(const char *keyName, int *type, int *scrollDirection);
int keyboardKey(const char *keyName);
int parseFromKey(const char *keyName, int *type, int *scrollDirection);