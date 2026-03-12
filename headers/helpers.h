/* 
Author: TheJewGamer
Last Update: 3/11/2026
*/

//standard includes
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>
#include <sys/stat.h>
#include <string.h>

//other files
#include "vars.h"
#include "settings.h"

//method to clean the array and other vars. Garbage collection
void freeMappings();

//helper method to run intial checks at launch
void setupCheck();