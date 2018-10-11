#include "yakk.h"
#include "yaku.h"
#include "clib.h"

#define NULL 0

int YKCtxSwCount = 0; //Global variable tracking context switches
int YKIdleCount = 0; //Global variable used by idle task
int YKTickNum = 0; //Global variable incremented by tick handler

TCBptr YKRdyList = 0;		/* a list of TCBs of all ready tasks in order of decreasing priority */ 
TCBptr YKSuspList = 0;		/* tasks delayed or suspended */
TCBptr YKAvailTCBList = 0;		/* a list of available TCBs */
TCB YKTCBArray[MAX_TASK_COUNT+1] = {0};	/* array to allocate all needed TCBs
				   				(extra one is for the idle task) */
int YKRunState = 0; // Flag to indicate if YKRun has been called
TCBptr YKCurrentTask = 0; // 
int IdleStk[IDLE_STACK_SIZE];
void YKIdleTask(void);

extern void asm_load_context(void);
extern void asm_mutex(void);
extern void asm_unmutex(void);

//Initializes all required kernel data structures
void YKInitialize(void){
	int i;
	printString("Entering YKInitialize...\n");
    YKAvailTCBList = &(YKTCBArray[0]);
    for (i = 0; i < MAX_TASK_COUNT+1; i++)
		YKTCBArray[i].next = &(YKTCBArray[i+1]);
    YKTCBArray[MAX_TASK_COUNT+1].next = NULL;
	YKNewTask(YKIdleTask, (void *)&IdleStk[IDLE_STACK_SIZE], MAX_TASK_COUNT+1);
	YKCurrentTask = YKRdyList; // Set the initial current task to the Idle task
/*
Create the idle task using YKNewTask with lowest priority;
Load initial values into registers?
*/
}

//Creates a new task
void YKNewTask(void (* task)(void), void *taskStack, unsigned char priority){
	TCBptr tmp, tmp2;
	printString("Entering YKNewTask...\n");

	tmp = YKAvailTCBList;
    YKAvailTCBList = tmp->next;
	tmp->stackptr = taskStack;
	tmp->pc = task;
	tmp->ax = 0;
	tmp->bx = 0;
	tmp->cx = 0;
	tmp->dx = 0;
	tmp->si = 0;
	tmp->di = 0;
	tmp->bp = 0;
	tmp->es = 0;
	tmp->ds = 0;
	tmp->flags = 512;
	tmp->state = running;
	tmp->priority = priority;
	tmp->delay = 0;


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
			printString("here1\n");
			YKRdyList = tmp;
			tmp->prev = NULL;
			tmp->next = tmp2;
			tmp2->prev = tmp;
		}else{
			printString("here2\n");
			tmp2->prev->next = tmp;
			tmp->prev = tmp2->prev;
			tmp->next = tmp2;
			tmp2->prev = tmp;
		}
    }
	printInt(YKRdyList->priority);
	printString("\n");
	printInt(YKRdyList->next->priority);
	printString("\n");
	printInt(YKRdyList->next->next->priority);
	printString("\n");
	printInt(YKRdyList->next->next->next->priority);
	printString("\n");
	if(YKRunState){ // If YKRun has been called...
		YKScheduler();
	}
}

void YKIdleTask(){
	while(1){
		printString("here\n");
		YKEnterMutex();
		YKIdleCount++;
		YKExitMutex();
	}
}

//Starts actual execution of user code
void YKRun(void){
	YKRunState = 1;
	YKScheduler();
}


//Delays a task for specified number of clock ticks
void YKDelayTask(unsigned count){
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
/*
If it is not a nested interrupt, context should be saved to the TCB
Increment ISR depth counter;
*/
}

//Called on exit from ISR
void YKExitISR(void){
/*
Decrement ISR depth counter;
If ISR depth counter is zero, call scheduler function;
*/
}

//Determines the highest priority ready task
void YKScheduler(void){
	if(YKRdyList != YKCurrentTask){
		YKCtxSwCount++;
		YKCurrentTask = YKRdyList;
		YKDispatcher();
	}
}

//Begins or resumes execution of the next task
void YKDispatcher(void){
	printString("dispatcher\n");
	asm_load_context();


/*
Loads context of next task to be run by referencing the global TCB list;
Call iret;
*/
}


//The kernel's timer tick interrupt handler
void YKTickHandler(void){
/*
Increment YKTickNum;
Decrement all “delayed” tasks’ delay counter;
If any task delay counter reaches zero, update that task to “ready”;
Call scheduler function;
*/
}

