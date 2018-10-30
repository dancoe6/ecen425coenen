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


extern void asm_save_context(void);
extern void asm_load_context(void);
extern void asm_mutex(void);
extern void asm_unmutex(void);

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
}

void YKIdleTask(){
#ifdef DEBUG 
	printString("Entering YKIdleTask...\n");
#endif
	while(1){
		YKEnterMutex();
		YKIdleCount++;
		YKExitMutex();
	}
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
    YKEnterMutex();
#ifdef DEBUG 
	printString("Entering YKDelayTask...\n");
#endif

    tmp = YKCurrentTask;
	tmp->delay = count; //set delay counter to the count value passed in
    YKRdyList = tmp->next; /* update the ready list (by removing the current task) */
    tmp->next->prev = NULL;

    if (YKSuspList == NULL){	/* is this first insertion? */
		YKSuspList = tmp;
		tmp->next = NULL;
		tmp->prev = NULL;
    }
    else{			/* not first insertion */
		tmp2 = YKSuspList;	/* insert in sorted delay list */
		while (tmp2->priority < tmp->priority){
			tmp2 = tmp2->next;	/* assumes idle task is at end */
		}
		if (tmp2->prev == NULL){	/* insert in list before tmp2 */
			YKSuspList = tmp;
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
	asm_unmutex();
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
	if(YKCurrentTask != 0){
		asm_save_context();
	}
	if(YKRdyList != YKCurrentTask){
	YKCurrentTask = YKRdyList;
	asm_load_context();
	}	
}


//The kernel's timer tick interrupt handler
void YKTickHandler(void){
	TCBptr tmp, tmp2;

#ifdef DEBUG 
	printString("Entering YKTickHandler...\n");
#endif

	YKTickNum++;
	tmp = YKSuspList;
	
	tmp->delay--;
	if(tmp->delay == 0){ //if it has reached zero, insert in YKRdyList
		YKEnterMutex();
		if (YKRdyList == NULL){	/* is this first insertion? */
			YKRdyList = tmp;
			tmp->next = NULL;
			tmp->prev = NULL;
		}else{			/* not first insertion */
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
		//and remove it from the YKSuspList
		if(tmp->prev != NULL)
			tmp->prev->next = tmp->next;
		if(tmp->next != NULL)
			tmp->next->prev = tmp->prev;
		YKScheduler();
	}


}

