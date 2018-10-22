
#include "yaku.h"

extern int YKCtxSwCount; //Global variable tracking context switches
extern int YKIdleCount; //Global variable used by idle task
extern int YKTickNum; //Global variable incremented by tick handler

enum taskState{ running, ready, delayed, suspended};
typedef struct taskblock *TCBptr;
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
}  TCB;

extern TCBptr YKRdyList;		/* a list of TCBs of all ready tasks
						  in order of decreasing priority */ 
extern TCBptr YKSuspList;		/* tasks delayed or suspended */
extern TCBptr YKAvailTCBList;		/* a list of available TCBs */
extern TCB    YKTCBArray[MAX_TASK_COUNT+1];	/* array to allocate all needed TCBs
				   (extra one is for the idle task) */

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

