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
#define MSGQSIZE 30

struct msg MsgArray[MSGARRAYSIZE];  /* buffers for message content */

int Task1Stk[TASK_STACK_SIZE];     /* a stack for each task */
int Task2Stk[TASK_STACK_SIZE];     /* a stack for each task */
int Task3Stk[TASK_STACK_SIZE];     /* a stack for each task */

YKSEM *SSemPtr;              //semaphore that indicates a received command
YKSEM *NSemPtr;             //semaphore that indicates a new piece
void *MsgQ[MSGQSIZE];           /* space for message queue */
YKQ *MsgQPtr;                   /* actual name of queue */

extern unsigned NewPieceID;
extern unsigned NewPieceType;
extern unsigned NewPieceOrientation;
extern unsigned NewPieceColumn;
// extern unsigned ScreenBitMap0;
// extern unsigned ScreenBitMap1;
// extern unsigned ScreenBitMap2;
// extern unsigned ScreenBitMap3;
// extern unsigned ScreenBitMap4;
// extern unsigned ScreenBitMap5;


//helper function to post to the movement queue
void postMovement(unsigned id, unsigned type, unsigned direction){
  static int next = 0;
  YKEnterMutex();
  MsgArray[next].id = (int)id;
  MsgArray[next].type = (int)type;
  MsgArray[next].direction = (int)direction;
  printString("New movement");
  printNewLine();
  printInt((int)type);
  printNewLine();
  YKExitMutex();
  if (YKQPost(MsgQPtr, (void *) &(MsgArray[next])) == 0)
    printString("  TickISR: queue overflow! \n");
  else if (++next >= MSGARRAYSIZE)
    next = 0;
}

void moveToColumn(unsigned pieceID, unsigned pieceColumn, unsigned targetColumn){
  while(pieceColumn != targetColumn){
    if(pieceColumn > targetColumn){
        postMovement(pieceID,0,0);
        pieceColumn--;
    }else if(pieceColumn < targetColumn){
        postMovement(pieceID,0,1);
        pieceColumn++;
    }else{
      //do nothing
    }
  }
}

void rotateToOrientation(unsigned pieceID, unsigned pieceOrientation, unsigned targetOrientation){
  while(pieceOrientation != targetOrientation){
    if(pieceOrientation > targetOrientation){
        postMovement(pieceID,1,1);
        pieceOrientation--;
    }else if(pieceOrientation < targetOrientation){
        postMovement(pieceID,1,0);
        pieceOrientation++;
    }else{
      //do nothing
    }
  }
}

void MessengerTask(void) /* Sends commands to simptris */
{
  struct msg * tmp;

  while(1){
    tmp = (struct msg *) YKQPend(MsgQPtr); //wait for a message to be posted to queue
    printString("New command");
    printNewLine();
    printInt((int)tmp->type);
    printNewLine();
    printInt((int)tmp->direction);
    printNewLine();
    if(tmp->type == 0){ //if it is a slide
      SlidePiece(tmp->id,tmp->direction);
    }else{//if it is a rotate
      RotatePiece(tmp->id,tmp->direction);
    }
    YKSemPend(SSemPtr);//wait for command received interrupt
  }
}


void MovementTask(void) /* Looks at new piece and decides piece movements */
{
  static int next = 0;
  static char leftFlat = 1; //1 means it is flat, 0 means it is not flat
  static char rightFlat = 1; //1 means it is flat, 0 means it is not flat
  static char rightMostRecent = 0; //0 means left is most recent, 1 means right is most recent
  /*
  instead of doing right most recent, we should track height of each bin
  */

  int id;
  int type;
  int direction;
  unsigned pieceID = 0;
  unsigned pieceType = 0;
  unsigned pieceOrientation = 0;
  unsigned pieceColumn = 0;

  while(1){

    YKSemPend(NSemPtr);

    //get information about new piece from globals
    YKEnterMutex();
    pieceID = NewPieceID;
    pieceType = NewPieceType;
    pieceOrientation = NewPieceOrientation;
    pieceColumn = NewPieceColumn;
    printString("New Piece");
    printNewLine();
    printInt((int)pieceType);
    printNewLine();
    printInt((int)pieceOrientation);
    printNewLine();
    printInt((int)pieceColumn);
    printNewLine();
    YKExitMutex();
    if(pieceType == 1){ //if it is straight
      if(leftFlat && rightFlat){ //if both sides are flat
        if(rightMostRecent){ //left bin is shorter, place there
          moveToColumn(pieceID,pieceColumn,1);
          rotateToOrientation(pieceID,pieceOrientation,0);
          rightMostRecent = 0;
        }else{ //right bin is shorter, place there
          moveToColumn(pieceID,pieceColumn,4);
          rotateToOrientation(pieceID,pieceOrientation,0);
          rightMostRecent = 1;
        }
      }else if(leftFlat){ //if left is flat
        moveToColumn(pieceID,pieceColumn,1);
        rotateToOrientation(pieceID,pieceOrientation,0);
        rightMostRecent = 0;
      }else{ //right is flat ( one of the two bins must always be flat)
        moveToColumn(pieceID,pieceColumn,4);
        rotateToOrientation(pieceID,pieceOrientation,0);
        rightMostRecent = 1;
      }
    }else{ //is corner piece
      if(!leftFlat){ //if left is not flat
        //move to column 2
        //rotate until newPieceOrientation == 2
        moveToColumn(pieceID,pieceColumn,2);
        rotateToOrientation(pieceID,pieceOrientation,2);
        rightMostRecent = 0;
        leftFlat = 1;
      }else if(!rightFlat){ //if right is not is flat
        //move to column 3
        //rotate until newPieceOrientation == 3
        moveToColumn(pieceID,pieceColumn,3);
        rotateToOrientation(pieceID,pieceOrientation,3);
        rightMostRecent = 1;
        rightFlat = 1;
      }else{//both bins are flat, choose shorter
        if(rightMostRecent){ //left bin is shorter, place there
          //move to column 1
          //rotate until newPieceOrientation == 0
          //move to column 0
          moveToColumn(pieceID,pieceColumn,1);
          rotateToOrientation(pieceID,pieceOrientation,0);
          moveToColumn(pieceID,pieceColumn,0);
          rightMostRecent = 0;
          leftFlat = 0;
        }else{ //right bin is shorter, place there
          //move to column 4
          //rotate until newPieceOrientation == 1
          //move to column 5
          moveToColumn(pieceID,pieceColumn,4);
          rotateToOrientation(pieceID,pieceOrientation,1);
          moveToColumn(pieceID,pieceColumn,5);
          rightMostRecent = 1;
          rightFlat = 0;
        }
      }
    }
    }
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

    YKNewTask(MessengerTask, (void *) &Task1Stk[TASK_STACK_SIZE], 1);
  	YKNewTask(MovementTask, (void *) &Task2Stk[TASK_STACK_SIZE], 2);
    StartSimptris();

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
  long seed = 1247;
    YKInitialize();

	MsgQPtr = YKQCreate(MsgQ, MSGQSIZE);
	SSemPtr = YKSemCreate(0);
  NSemPtr = YKSemCreate(0);



	YKNewTask(StatTask, (void *) &Task3Stk[TASK_STACK_SIZE], 3);

  SeedSimptris(seed);
  YKRun();
}
