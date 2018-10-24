#include "clib.h"
//#include "yakk.h"

extern int KeyBuffer;
static unsigned int tick_count;

void resetHandler(void){
exit(0);
}


void tickHandler(void){
tick_count++;
printNewLine();
printString("TICK ");
printUInt(tick_count);
printNewLine();
}



void keyboardHandler(void){
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

else{
printNewLine();
printString("KEYPRESS (");
printChar(KeyBuffer);
printString(") IGNORED");
printNewLine();
}

}
