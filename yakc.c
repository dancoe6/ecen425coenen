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

YKSEM YKSemArray[MAX_SEM_COUNT] = {0};
int YKSemIndex = 0;

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
	tmp->pending = NULL;


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
	c = YKSuspCnt;
#ifdef DEBUG
	printString("Entering YKDelayTask...\n");
	tmp = YKSuspList;
	printString("SuspList before YKdelay\n");
	for (i = 0; i < c; i++){
		printInt(tmp->priority);
		printString(": delay - ");
		printInt(tmp->delay);
		printNewLine();
		tmp = tmp->next;
	}
#endif

#ifdef DEBUG
	printString("RdyList before YKDelay\n");
	tmp = YKRdyList;
	while(tmp != NULL){
		printInt(tmp->priority);
		printNewLine();
		tmp = tmp->next;
	}
	printNewLine();
#endif

	YKSuspCnt++;
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
		tmp2->prev = tmp;
		tmp->next = tmp2;
		tmp->prev = NULL;
		YKSuspList = tmp;
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
#ifdef DEBU
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


		if(tmp->delay == 0 && tmp->pending == NULL){ //if it has reached zero, insert in YKRdyList

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
		}
		else
			tmp = tmp->next;
	}
#ifdef DEBU
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


//Creates and initializes a semaphore
//must be called exactly once and before post or pend to that semaphore
semptr YKSemCreate(int initialValue){
	static int YKSemCnt;
	semptr temp;
	temp = &YKSemArray[YKSemIndex];
	YKSemIndex++;
	temp->value = initialValue;
	temp->id = YKSemCnt;
	YKSemCnt++;
	#ifdef DEBUG
	printString("Semaphore created - id: ");
	printInt(temp->id);
	printNewLine();
	#endif
	return temp;
}

//post to the semaphore passed in
void YKSemPost(semptr sem){
	int c, i, first;
	TCBptr tmp, tmp2,topPriority;
	YKEnterMutex();
	first = 1;
	c = YKSuspCnt;

	#ifdef DEBUG
	printNewLine();
	printString("Entering YKSemPost...");
	printString("SuspList before YKSemPost\n");
	tmp = YKSuspList;
	for (i = 0; i < c; i++){
		printInt(tmp->priority);
		printNewLine();
		tmp = tmp->next;
	}
	printNewLine();
	printString("Semaphore being posted is ");
	printInt(sem->id);
	printNewLine();
	#endif

	sem->value++;//increment semaphore value
	if(sem->value > 0){//if no task is pending on this semaphore...
		YKExitMutex();
		return;
	}

	tmp = YKSuspList;
	while(tmp != NULL){//go through each suspended tasks to find if one is pending this semaphore

		if(tmp->pending == sem){ //if this task is pending on given semaphore

			#ifdef DEBUG
			printString("One pending task priority is ");
			printInt(tmp->priority);
			printString(" - id: ");
			printInt(tmp->pending->id);
			printNewLine();
			#endif

			if(first){ //if first found
				topPriority = tmp;
				first = 0; //lower flag
			}
			else if(tmp->priority < topPriority->priority){ //if this task has highest priority so far
				topPriority = tmp;
			}
		}
		tmp = tmp->next;
	}

	if (first != 0) //if no task is pending this semaphore...
		return;
		
	tmp = topPriority; //select the highest priority task pending this semaphore
	
	#ifdef DEBUG
	printString("Top priority pending task's priority is ");
	printInt(tmp->priority);
	printNewLine();
	#endif	

	//remove the task from YKSuspList
	if (tmp->prev == NULL){ //if at the top of the suspend list 
			YKSuspList = tmp->next;
			YKSuspList->prev = NULL;
	}
	else{
		tmp->prev->next = tmp->next;
		tmp->next->prev = tmp->prev;
	}
	


	//add the task to the YKRdyList (sorted by priority)
	tmp2 = YKRdyList;
	tmp->pending = NULL;
	while (tmp2->priority < tmp->priority){
		tmp2 = tmp2->next;	//assumes idle task is at end
	}
	if (tmp2->prev == NULL){
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


	#ifdef DEBUG
	printString("RdyList after YKSemPost\n");
	tmp = YKRdyList;
	while(tmp != NULL){
		printInt(tmp->priority);
		printNewLine();
		tmp = tmp->next;
	}
	printNewLine();
	#endif


	if(YKISRDepth == 0){
		YKScheduler();
	}
}

//pend on a semaphore that is passed in
void YKSemPend(semptr sem){
	TCBptr tmp, tmp2;

	#ifdef DEBUG
	tmp = YKCurrentTask;
	printString("Entering YKSemPend...");
	printNewLine();
	printString("Pending task priority is ");
	printInt(tmp->priority);
	printNewLine();
	printString("Semaphore id is ");
	printInt(sem->id);
	printNewLine();
	#endif

	YKEnterMutex();
	sem->value--; //decrement semaphore value
	if(sem->value >= 0){//if the semaphore was available...
		YKExitMutex();
		return;
	}
	else{ //if the semaphore is unavailable
		YKSuspCnt++;
		tmp = YKCurrentTask;
		tmp->pending = sem;
		YKRdyList = tmp->next; /* update the ready list (by removing the current task) */
		tmp->next->prev = NULL;
		
		//move the current task to the suspend list
		if (YKSuspList == NULL){	/* is this first insertion? */
			tmp->next = NULL;
		}
		else{	/* not first insertion */
			tmp2 = YKSuspList;
			tmp->next = tmp2;
			tmp2->prev = tmp;
		}
		tmp->prev = NULL;
		YKSuspList = tmp;

		YKScheduler();
	}
}


//Create and initialize a message queue and returns a pointer to the kernel's data structure used to maintain that queue
YKQ *YKQCreate(void **start, unsigned size){}

//Remove the oldest message from the indicated message queue if it is non-empty.
void *YKQPend(YKQ *queue){}

//Place a message in a message queue
int YKQPost(YKQ *queue, void *msg){}

