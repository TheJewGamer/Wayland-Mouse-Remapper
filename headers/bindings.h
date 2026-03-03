#ifndef BINDINGS_H
#define BINDINGS_H

//includes
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <linux/input.h>

//functions
int mouseKey(const char *name, int *type, int *scrollDirection);
int keyboardKey(const char *name);

#endif