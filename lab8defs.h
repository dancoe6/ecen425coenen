/*
File: lab8defs.h
Revision date: 4 November
Description: Required definitions for EE 425 lab 8 (Message queues)
*/
#define _LAB_8_DEFS
#ifdef _LAB_8_DEFS
#define MSGARRAYSIZE	24

struct msg
{
    int id; //id of piece
    int type; //type of function call (0 for slide, 1 for rotate)
	int direction; //direction of movement.
                //if slide, 0 means left and 1 means right.
                //if rotate, 0 means counter-clockwise and 1 means clockwise
};

extern YKSEM *NSemPtr;
extern YKSEM *SSemPtr;
#endif
