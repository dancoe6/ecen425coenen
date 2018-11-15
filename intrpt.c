#include "clib.h"
#include "yakk.h"
#include "lab6defs.h"
#include "intrpt.h"
#define LAB6

extern int KeyBuffer;
static unsigned int tick_count;

extern YKQ *MsgQPtr;
extern struct msg MsgArray[];
extern int GlobalFlag;

void resetHandler(void){
exit(0);
}


void tickHandler(void){

#ifdef LAB6

/* Call YKQPost to post a message to a message queue */
    static int next = 0;
    static int data = 0;

    // create a message with tick (sequence #) and pseudo-random data
    MsgArray[next].tick = YKTickNum;
    data = (data + 89) % 100;
    MsgArray[next].data = data;
     if (YKQPost(MsgQPtr, (void *) &(MsgArray[next])) == 0)
	printString("  TickISR: queue overflow! \n");
     else if (++next >= MSGARRAYSIZE)
	next = 0;


#else
tick_count++;
printNewLine();
printString("TICK ");
printUInt(tick_count);
printNewLine();
#endif

}



void keyboardHandler(void){

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
	//printString("p key pressed, need to uncomment yksempost");
	//printNewLine();
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
