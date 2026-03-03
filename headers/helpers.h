#ifndef HELPERS_H
#define HELPERS_H

//includes
#include <stdio.h> //printing
#include <stdlib.h> //exit/free
#include <unistd.h> //file reading
#include <dirent.h> //directory reading
#include <pwd.h> //user info
#include <grp.h> //groups

//functions
const char *getHome();

//method to clean the array and other vars. Garbage collection
void freeMappings();

//helper method to drop privileges back to user who called sudo
void dropPrivileges();

#endif