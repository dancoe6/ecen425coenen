/* 
File: lab8defs.h
Revision date: 4 November
Description: Required definitions for EE 425 lab 8 (Message queues)
*/

#define MSGARRAYSIZE	24

struct msg 
{
    int id;
    int type;
	int direction;
};
