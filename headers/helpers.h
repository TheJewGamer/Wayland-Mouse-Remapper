/* 
Author: TheJewGamer
Last Update: 3/4/2026
*/

#ifndef HELPERS_H
#define HELPERS_H

//includes
#include <stdio.h> //printing
#include <stdlib.h> //exit/free
#include <unistd.h> //file reading
#include <dirent.h> //directory reading
#include <pwd.h> //user info
#include <grp.h> //groups

//method to clean the array and other vars. Garbage collection
void freeMappings();

//helper method to drop privileges back to user who called sudo
void dropPrivileges();

//helper method to run intial checks at launch
void setupCheck();

#endif