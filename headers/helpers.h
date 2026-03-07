/* 
Author: TheJewGamer
Last Update: 3/6/2026
*/

//method to clean the array and other vars. Garbage collection
void freeMappings();

//helper method to drop privileges back to user who called sudo
void dropPrivileges();

//helper method to run intial checks at launch
void setupCheck();