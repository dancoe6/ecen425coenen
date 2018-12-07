/* 
File: lab8_app.c
Revision date: 10 November 2005
Description: Application code for EE 425 lab 7 (Event flags)
*/

#include "clib.h"
#include "yakk.h"                     /* contains kernel definitions */
#include "simptris.h"
#include "lab8defs.h"

#define TASK_STACK_SIZE   512         /* stack size in words */
#define MSGQSIZE 24

struct msg MsgArray[MSGARRAYSIZE];  /* buffers for message content */

int Task1Stk[TASK_STACK_SIZE];     /* a stack for each task */
int Task2Stk[TASK_STACK_SIZE];     /* a stack for each task */
int Task3Stk[TASK_STACK_SIZE];     /* a stack for each task */

YKSEM *SSemPtr;              /* YKSEM must be defined in yakk.h */
void *MsgQ[MSGQSIZE];           /* space for message queue */
YKQ *MsgQPtr;                   /* actual name of queue */

void MessengerTask(void) /* Sends commands to simptris */
{

}


void MovementTask(void) /* Looks at new piece and decides piece movements */
{
 
}

void StatTask(void)           /* tracks statistics */
{
    unsigned max, switchCount, idleCount;
    int tmp;

    YKDelayTask(1);
    printString("Welcome to the YAK kernel\r\n");
    printString("Determining CPU capacity\r\n");
    YKDelayTask(1);
    YKIdleCount = 0;
    YKDelayTask(5);
    max = YKIdleCount / 25;
    YKIdleCount = 0;
    
    while (1)
    {
        YKDelayTask(20);
        
        YKEnterMutex();
        switchCount = YKCtxSwCount;
        idleCount = YKIdleCount;
        YKExitMutex();
       
        printString("<CS: ");
        printInt((int)switchCount);
        printString(", CPU: ");
        tmp = (int) (idleCount/max);
        printInt(100-tmp);
        printString(">\r\n");
        
        YKEnterMutex();
        YKCtxSwCount = 0;
        YKIdleCount = 0;
        YKExitMutex();
    }
}   


void main(void)
{
    YKInitialize();

	MsgQPtr = YKQCreate(MsgQ, MSGQSIZE);
	SSemPtr = YKSemCreate(0);

	YKNewTask(MessengerTask, (void *) &Task1Stk[TASK_STACK_SIZE], 1);
	YKNewTask(MovementTask, (void *) &Task2Stk[TASK_STACK_SIZE], 2);  
	YKNewTask(StatTask, (void *) &Task3Stk[TASK_STACK_SIZE], 3);      	

    YKRun();
}
