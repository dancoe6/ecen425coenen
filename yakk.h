
#include "yaku.h"

extern int YKCtxSwCount; //Global variable tracking context switches
extern int YKIdleCount; //Global variable used by idle task
extern int YKTickNum; //Global variable incremented by tick handler

enum taskState{ running, ready, delayed, suspended};
typedef struct taskblock *TCBptr;
typedef struct semaphore *semptr;
typedef struct taskblock
{				/* the TCB struct definition */
    void *stackptr;		/* pointer to current top of stack */
	int flags;
	void* ip; 			/* program counter for the task */

	//store context in TCB struct
	int ax;
	int bx;
	int cx;
	int dx;
	int si;
	int di;
	int bp;
	int es;
	int ds;
    enum taskState state;	/* current state */
    int priority;		/* current priority */
    int delay;			/* #ticks yet to wait */
    TCBptr next;		/* forward ptr for dbl linked list */
    TCBptr prev;		/* backward ptr for dbl linked list */
    semptr pending; /*semaphore that task is waiting for*/
}  TCB;

extern TCBptr YKRdyList;		/* a list of TCBs of all ready tasks
						  in order of decreasing priority */
extern TCBptr YKSuspList;		/* tasks delayed or suspended */
extern TCBptr YKAvailTCBList;		/* a list of available TCBs */
extern TCB    YKTCBArray[MAX_TASK_COUNT+1];	/* array to allocate all needed TCBs
				   (extra one is for the idle task) */

//semaphore structures

typedef struct semaphore{
  int value;
  int id;
} YKSEM;

extern YKSEM YKSemArray[MAX_SEM_COUNT];

typedef struct msgqueue
{

    /* What goes here?? */
    int info;
    // nextspace;
    // lastmsg;

} YKQ;


//Initializes all required kernel data structures
void YKInitialize(void);

//Creates a new task
void YKNewTask(void (* task)(void), void *taskStack, unsigned char priority);


//Starts actual execution of user code
void YKRun(void);


//Delays a task for specified number of clock ticks
void YKDelayTask(unsigned count);


//Disables interrupts, written in assembly
void YKEnterMutex(void);

//Enables interrupts
void YKExitMutex(void);

//Called on entry to ISR
void YKEnterISR(void);

//Called on exit from ISR
void YKExitISR(void);

//Determines the highest priority ready task
void YKScheduler(void);

//Begins or resumes execution of the next task
void YKDispatcher(void);

//The kernel's timer tick interrupt handler
void YKTickHandler(void);

//Creates and initializes a semaphore
//must be called exactly once and before post or pend to that semaphore
semptr YKSemCreate(int initialValue);

//post to the semaphore passed in
void YKSemPost(semptr semaphore);

//pend on a semaphore that is passed in
void YKSemPend(semptr semaphore);

//Create and initialize a message queue and returns a pointer to the kernel's data structure used to maintain that queue
YKQ *YKQCreate(void **start, unsigned size);

//Remove the oldest message from the indicated message queue if it is non-empty.
void *YKQPend(YKQ *queue);

//Place a message in a message queue
int YKQPost(YKQ *queue, void *msg);

