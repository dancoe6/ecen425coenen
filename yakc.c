#include "yakk.h"
#include "yaku.h"
#include "clib.h"

#define NULL 0
//#define DEBUG 0

int YKCtxSwCount = 0; //Global variable tracking context switches
int YKIdleCount = 0; //Global variable used by idle task
int YKTickNum = 0; //Global variable incremented by tick handler
int YKISRDepth = 0;

TCBptr YKRdyList = 0;		/* a list of TCBs of all ready tasks in order of decreasing priority */ 
TCBptr YKSuspList = 0;		/* tasks delayed or suspended */
TCBptr YKAvailTCBList = 0;		/* a list of available TCBs */
TCB YKTCBArray[MAX_TASK_COUNT+1] = {0};	/* array to allocate all needed TCBs
				   				(extra one is for the idle task) */
int YKRunState = 0; // Flag to indicate if YKRun has been called
TCBptr YKCurrentTask = 0; // 
void YKIdleTask(void);
int IdleStk[IDLE_STACK_SIZE];
int YKSuspCnt = 0;

extern void asm_save_context(void);
extern void asm_load_context(void);
extern void asm_mutex(void);
extern void asm_unmutex(void);
extern void asm_idle_task(void);

//Initializes all required kernel data structures
void YKInitialize(void){
	int i;
	YKEnterMutex();
#ifdef DEBUG 
printString("Entering YKInitialize...\n");
#endif
    YKAvailTCBList = &(YKTCBArray[0]);
    for (i = 0; i < MAX_TASK_COUNT+1; i++)
		YKTCBArray[i].next = &(YKTCBArray[i+1]);
    YKTCBArray[MAX_TASK_COUNT+1].next = NULL;
	YKNewTask(YKIdleTask, (void *)&IdleStk[IDLE_STACK_SIZE], MAX_TASK_COUNT+1);
	YKExitMutex();
}

//Creates a new task
void YKNewTask(void (* task)(void), void *taskStack, unsigned char priority){
	TCBptr tmp, tmp2;
	YKEnterMutex();
#ifdef DEBUG 
	printString("Entering YKNewTask...\n");
#endif

	tmp = YKAvailTCBList;
    YKAvailTCBList = tmp->next;
	tmp->stackptr = taskStack;
	tmp->flags = 512;
	tmp->ip = task;
	tmp->ax = 0;
	tmp->bx = 0;
	tmp->cx = 0;
	tmp->dx = 0;
	tmp->si = 0;
	tmp->di = 0;
	tmp->bp = 0;
	tmp->es = 0;
	tmp->ds = 0;
	tmp->state = running;
	tmp->priority = priority;
	tmp->delay = 0;


	if (YKRdyList == NULL){	/* is this first insertion? */
		tmp->next = NULL;
		tmp->prev = NULL;
		YKRdyList = tmp;
    }
    else{			/* not first insertion */
		tmp2 = YKRdyList;	/* insert in sorted ready list */
		while (tmp2->priority < tmp->priority){
			tmp2 = tmp2->next;	/* assumes idle task is at end */
		}
		if (tmp2->prev == NULL){	/* insert in list before tmp2 */
			YKRdyList = tmp;
			tmp->prev = NULL;
			tmp->next = tmp2;
			tmp2->prev = tmp;
		}else{
			tmp2->prev->next = tmp;
			tmp->prev = tmp2->prev;
			tmp->next = tmp2;
			tmp2->prev = tmp;
		}
    }

	if(YKRunState){ // If YKRun has been called...
		YKScheduler();
	}
	YKExitMutex();
}

void YKIdleTask(){
#ifdef DEBUG 
	printString("Entering YKIdleTask...\n");
#endif
/*	while(1){
		YKEnterMutex();
		YKIdleCount++;
		YKExitMutex();
	}*/
	asm_idle_task();
}

//Starts actual execution of user code
void YKRun(void){
	YKEnterMutex();
	YKRunState = 1;
	YKScheduler();
}


//Delays a task for specified number of clock ticks
void YKDelayTask(unsigned count){
    TCBptr tmp, tmp2;
	int i, c;
    YKEnterMutex();
#ifdef DEBUG 
	printString("Entering YKDelayTask...\n");
#endif
	YKSuspCnt++;
	c = YKSuspCnt;
    tmp = YKCurrentTask;
	tmp->delay = count; //set delay counter to the count value passed in
    YKRdyList = tmp->next; /* update the ready list (by removing the current task) */
    tmp->next->prev = NULL;

    if (YKSuspList == NULL){	/* is this first insertion? */
		tmp->next = NULL;
		tmp->prev = NULL;
		YKSuspList = tmp;
    }
    else{			/* not first insertion */
	/* insert in sorted delay list */
		tmp2 = YKSuspList;
		while (tmp2->priority < tmp->priority && tmp2->next != NULL){	
			tmp2 = tmp2->next;
		}
		if (tmp2->next == NULL){
			tmp2->next = tmp;
			tmp->prev = tmp2;
			tmp->next = NULL;
		}
		else if (tmp2->prev == NULL){	/* insert in list before tmp2 */
			YKSuspList = tmp;
			tmp->prev = NULL;
			tmp->next = tmp2;
			tmp2->prev = tmp;
		}
		else{

			tmp2->prev->next = tmp;
			tmp->prev = tmp2->prev;
			tmp->next = tmp2;
			tmp2->prev = tmp;
		}
    }
/*
	tmp = YKSuspList;
	printString("SuspList after YKdelay sort\n");
	for (i = 0; i < c; i++){
		printInt(tmp->delay);
		printNewLine();
		tmp = tmp->next;
	}*/

    YKScheduler();
    
/*
Change the “state” field within the current task’s TCB to “delayed”;
Change the “delay” field with the current task’s TCB to the argument “count”;
Call the scheduler function;
*/
}


//Disables interrupts, written in assembly
void YKEnterMutex(void){
	
	asm_mutex();
}

//Enables interrupts
void YKExitMutex(void){
	if(YKRunState){ // If YKRun has been called...
		asm_unmutex();
	}
}


//Called on entry to ISR
void YKEnterISR(void){
	YKISRDepth++;
/*
??If it is not a nested interrupt, context should be saved to the TCB??
Increment ISR depth counter;
*/
}

//Called on exit from ISR
void YKExitISR(void){
/*
Decrement ISR depth counter;
If ISR depth counter is zero, call scheduler function;
*/
	if(YKISRDepth == 0){
		YKScheduler();
	}
}

//Determines the highest priority ready task
void YKScheduler(void){
	if(YKRdyList != YKCurrentTask){
		YKCtxSwCount++;
		YKDispatcher();
	}
	YKExitMutex();
}

//Begins or resumes execution of the next task
void YKDispatcher(void){
#ifdef DEBUG 
	printString("Entering dispatcher...\n");
#endif
	if(YKCurrentTask != 0){//don't save context on first call
		asm_save_context();
	}

	if(YKRdyList != YKCurrentTask){
		YKCurrentTask = YKRdyList;
		asm_load_context();
	}
}


//The kernel's timer tick interrupt handler
void YKTickHandler(void){
	TCBptr tmp, tmp2, tmp3;
	int i, c;
	c = YKSuspCnt;

#ifdef DEBUG 
	printString("Entering YKTickHandler...\n");
	printNewLine();
#endif
#ifdef DEBUG 
	tmp = YKSuspList;
	printString("SuspList before YKTick\n");
	for (i = 0; i < c; i++){
		printInt(tmp->priority);
		printString(" - ");
		printInt(tmp->delay);
		printNewLine();
		tmp = tmp->next;
	}
#endif
	
	YKEnterMutex();
	YKTickNum++;
	tmp = YKSuspList;
	for (i = 0; i < c; i++){
		if(tmp->delay > 0)
			tmp->delay--;
		tmp = tmp->next;

	}


	tmp = YKSuspList;
	for (i = 0; i < c; i++){

		
		if(tmp->delay == 0){ //if it has reached zero, insert in YKRdyList

			//printInt(YKSuspList->priority);
			//printInt(tmp->next->delay);
		
			tmp3 = tmp->next;
			if (tmp->prev == NULL){ //if the top of the delay list is ready
				YKSuspList = tmp->next;
				YKSuspList->prev = tmp->prev;
			}
			else{
				tmp->prev->next = tmp->next;
				tmp->next->prev = tmp->prev;
			}
			

			//add it to the YKRdyList
			tmp2 = YKRdyList;	
			while (tmp2->priority < tmp->priority){
				tmp2 = tmp2->next;	//assumes idle task is at end 
			}
			if (tmp2->prev == NULL){	// insert in list before tmp2
				YKRdyList = tmp;
				tmp->prev = NULL;
				tmp->next = tmp2;
				tmp2->prev = tmp;
			}
			else{
				tmp2->prev->next = tmp;
				tmp->prev = tmp2->prev;
				tmp->next = tmp2;
				tmp2->prev = tmp;
			}
			YKSuspCnt--;
			tmp = tmp3;
			//break;
		}
		else
			tmp = tmp->next;
	}
#ifdef DEBUG 
	c = YKSuspCnt;
	tmp = YKSuspList;
	printString("SuspList after YKTick\n");
	for (i = 0; i < c; i++){
		printInt(tmp->priority);
		printString(" - ");
		printInt(tmp->delay);
		printNewLine();
		tmp = tmp->next;
	}
#endif

	YKScheduler();

}







/*






*/





























