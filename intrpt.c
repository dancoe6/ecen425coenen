#include "clib.h"
#include "yakk.h"
#include "lab8defs.h"

#define LAB8

extern int KeyBuffer;
static unsigned int tick_count;

#ifdef LAB6
extern YKQ *MsgQPtr;
extern struct msg MsgArray[];

extern int GlobalFlag;
#endif

void resetHandler(void){
exit(0);
}


void tickHandler(void){

#ifdef LAB8


#endif

#ifdef LAB6

/* Call YKQPost to post a message to a message queue */
    static int next = 0;
    static int data = 0;

    YKEnterMutex();

    // create a message with tick (sequence #) and pseudo-random data
    MsgArray[next].tick = YKTickNum;
    data = (data + 89) % 100;
    MsgArray[next].data = data;
    YKExitMutex();
     if (YKQPost(MsgQPtr, (void *) &(MsgArray[next])) == 0)
	     printString("  TickISR: queue overflow! \n");
     else if (++next >= MSGARRAYSIZE)
	     next = 0;

#endif

#ifdef LAB7

tick_count++;

#endif

#ifdef LAB8

#else
tick_count++;
printNewLine();
printString("TICK ");
printUInt(tick_count);
printNewLine();
#endif

}



void keyboardHandler(void){

#ifdef LAB8


#endif

#ifdef LAB6

GlobalFlag = 1;


#else
unsigned int delay = 0;
if(KeyBuffer == 'd'){
printNewLine();
printString("DELAY KEY PRESSED");
	while(delay < 5000){
		delay++;
	}
printNewLine();
printString("DELAY COMPLETE");
printNewLine();
}
else if(KeyBuffer == 'p'){
	YKSemPost(&YKSemArray[3]);
}else{
printNewLine();
printString("KEYPRESS (");
printChar(KeyBuffer);
printString(") IGNORED");
printNewLine();
}

#endif

}



void newPieceHandler(void){
  YKSemPost(NSemPtr);

}

void receivedCommandHandler(void){
  YKSemPost(SSemPtr);
}
