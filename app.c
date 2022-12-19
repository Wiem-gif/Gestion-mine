

#include <stdio.h>
#include "includes.h"

/* Definition of Task Stacks */
#define   TASK_STACKSIZE       512
OS_STK    taskStart_stk[TASK_STACKSIZE];
OS_STK    task1_stk[TASK_STACKSIZE];
OS_STK    task2_stk[TASK_STACKSIZE];
OS_STK    task3_stk[TASK_STACKSIZE];
OS_STK    task4_stk[TASK_STACKSIZE];

/* Definition of Tasks' Priorities */
#define TASKSTART_PRIORITY  10
#define TASK1_PRIORITY      13      // Methane control
#define TASK2_PRIORITY      14      // Water control
#define TASK3_PRIORITY      15       // Alarme control
#define TASK4_PRIORITY      16        // Pompe control

/* **********Definition of Event Flag variables *************/
FLAGS_1        0x01      /* bit pattern for task1: task1 will wait for bit 0 (Number of bits: 8 (or 16 or 32)) */
FLAGS_2        0x02      /* bit pattern for task2: task2 will wait for bit 1*/
FLAGS_3        0x03      /* bit pattern for task3: task3 will wait for bit 2(Number of bits: 8 (or 16 or 32)) */
FLAGS_4        0x04      /* bit pattern for task4: task4 will wait for bit 3*/
OS_FLAG_GRP     *FlagGroupE;                         /* Event flag group pointer                  */
OS_FLAG_GRP     *FlagGroupM;                         /* Event flag group pointer                  */





/* Definition of binary Semaphore variables */



 OS_EVENT* RandomSem;

#define   MSG_QUEUE_SIZE       2                        /* Size of message queue */
OS_EVENT *MsgQueue;                                    /*  Pointer of message queue */
void     *MsgQueueTbl[MSG_QUEUE_SIZE];                  /* Storage of message */


//task1 control the value of methane (MS), when it exceeds the high  level 1 value (MS_L1) this task send a flag_1 to the task3 for active the alarm 
 //and when MS exceeds the high level 2  value (MS_L2) ,it send flag_2 to task3 for active the alarm and disable the pompe 




/* Prints priority and random number between 1 and 100 then sleeps for 3 seconds */
void task1(void* pdata)
{
	INT8U MS, err;
	time_t t1;
	srand(time(&t1));   /*The srand() function is used to set the starting value for the series of random integers. */
	int MS_L1=20 ; /*the default value of MS_L1*/
	int MS_L2=40 ; /*the default value of MS_L2*/

	while (1)
	{
		/* Acquire semaphore to perform random numbers        */
		
		 RandomSemPend(RandomSem,0,&err);
		MS = (int)(rand() % 100 + 1);
		/* Release semaphore           */
		
	     RandomSemPost(RandomSem);

		printf("Task %d's random #: %d\n", OSPrioCur, MS);

		
		if (MS>=MS_L1){

			OSFlagPost((OS_FLAG_GRP *)FlagGroupM, (OS_FLAGS)FLAGS_1, OS_FLAG_SET, &err);

		}
		else if (MS>MS_L2){

			OSFlagPost((OS_FLAG_GRP *)FlagGroupM, (OS_FLAGS)FLAGS_2, OS_FLAG_SET, &err);


		}
		OSTimeDlyHMSM(0, 0, 3, 0);
	}

}
//task2 control the value of water  (LS), when it exceeds the high value (HLS) this task send a flag_3 to the task4 for active the pompe
 //and when LS decrease to the low value (LLS) ,it send flag_4 to task4 for disable the pompe  


/* Prints priority and random number between 1 and 500 then sleeps for 6 seconds */

void task2(void* pdata)
{
	INT8U LS, err;
	time_t t1;
	srand(time(&t1));
	int LLS=40 ;   /*the default value of LLS*/ 
	int HLS=90 ;   /*the default value of HLS*/

	while (1)
	{
		/* Acquire semaphore to perform random numbers        */
		
		 RandomSemPend(RandomSem,0,&err);
		LS = (int)(rand() % 500 + 1);
		/* Release semaphore                                  */
		
		 RandomSemPost(RandomSem);


		printf("Task %d's random #: %d\n", OSPrioCur, LS);

		if (LS>HLS){

			OSFlagPost((OS_FLAG_GRP *)FlagGroupE, (OS_FLAGS)FLAGS_3, OS_FLAG_SET,  &err);

		}
		else if (LS<LLS){

			OSFlagPost((OS_FLAG_GRP *)FlagGroupE, (OS_FLAGS)FLAGS_4, OS_FLAG_SET,  &err);


		}
		OSTimeDlyHMSM(0, 0, 6, 0);
	}
	}
}
/* task3 waits for 1 of 2 events to occur (FLAGS_1 or FLAGS_2), 
   Prints "Active alarm",  (FLAGS_1 or FLAGS_2) */
   /* Send a message queue to task4 to disable the pompe */

void task3(void* pdata)
{
	INT8U  err;
	while(1){
		
		//task3 waits for bit0 to be set
		//wait option: wait for any flag in the group AND then clear the flags that are present  
		OSFlagPend((OS_FLAG_GRP *)FlagGroupM, (OS_FLAGS)FLAGS_1 ,  OS_FLAG_WAIT_SET_ANY + OS_FLAG_CONSUME, 0, &err);
		printf(" Active alarm ");
		
		//task3 waits for bit1 to be set
		//wait option: wait for any flag in the group AND then clear the flags that are present 
		OSFlagPend((OS_FLAG_GRP *)FlagGroupM, (OS_FLAGS)FLAGS_2 ,  OS_FLAG_WAIT_SET_ANY + OS_FLAG_CONSUME, 0, &err);
		printf(" Active alarm ");
		char msg[20];
		stropy(msg, "Disable  pompe!!");
		for(;;){
			OSQPost(MsgQueue,(void*)msg);
			OSTimeDlyHMS(0,0,0,500);
		}


		

	}	


}

/* task4 waits for 1 of 2 events to occur (FLAGS_3 or FLAGS_4), 
   Prints "Active  pompe",  (FLAGS_3) */
   /*Prints " Disable  pompe ", (receive a message queue from task3 and disable  pompe or FLAGS_4 )  */
void task4(void* pdata){
INT8U  err;
while(1){


OSFlagPend((OS_FLAG_GRP *)FlagGroupE, (OS_FLAGS)FLAGS_3 ,  OS_FLAG_WAIT_SET_ANY + OS_FLAG_CONSUME, 0, &err);
		printf(" Active pompe ");

		if(OSFlagPend((OS_FLAG_GRP *)FlagGroupE, (OS_FLAGS)FLAGS_4 ,  OS_FLAG_WAIT_SET_ANY + OS_FLAG_CONSUME, 0, &err)||(for(;;){

			msg=(char*)OSQPend(MsgQueue,0,&err);
			OSTimeDlyHMS(0,0,0,100);
				})){
		  printf(" Disable pompe ");
		}
		


}
}
/*taskStart creates the other tasks, signals task1  task2,taks3 and task4, then deletes itself*/

void taskStart(void* pdata) /* TASKSTART METTRE EVENT ON 1*/
{
	INT8U err;

	OSTaskCreateExt(task1,
		NULL,
		(void *)&task1_stk[TASK_STACKSIZE - 1],
		TASK1_PRIORITY,
		TASK1_PRIORITY,
		task1_stk,
		TASK_STACKSIZE,
		NULL,
		0);
	OSTaskCreateExt(task2,
		NULL,
		(void *)&task2_stk[TASK_STACKSIZE - 1],
		TASK2_PRIORITY,
		TASK2_PRIORITY,
		task2_stk,
		TASK_STACKSIZE,
		NULL,
		0);
	OSTaskCreateExt(task3,
		NULL,
		(void *)&task3_stk[TASK_STACKSIZE - 1],
		TASK3_PRIORITY,
		TASK3_PRIORITY,
		task3_stk,
		TASK_STACKSIZE,
		NULL,
		0);
	OSTaskCreateExt(task4,
		NULL,
		(void *)&task4_stk[TASK_STACKSIZE - 1],
		TASK4_PRIORITY,
		TASK4_PRIORITY,
		task4_stk,
		TASK_STACKSIZE,
		NULL,
		0);

	
	while (1)
	{
		printf("Statistics from taskStart\n");
		OSTimeDlyHMSM(0, 0, 5, 0);
	}

	OSTaskDel(OS_PRIO_SELF);    /* to delete itself*/

}
/* The main function creates four tasks and starts multi-tasking */
int main(void)
{
	INT8U err;
	OSInit();
	BSP_Init();                                                 /* Initialize BSP functions			*/         
	CPU_Init();                                                 /* Initialize uC/CPU services	    */

	MsgQueue = OSQCreate(&MsgQueueTbl[0], MSG_QUEUE_SIZE);
	/* Create two flag group*/
	FlagGroupE = OSFlagCreate((OS_FLAGS)0x00, &err);
	FlagGroupM = OSFlagCreate((OS_FLAGS)0x00, &err);
	RandomSem = OSSemCreate(1);

	OSTaskCreateExt(taskStart,
		NULL,
		(void *)&taskStart_stk[TASK_STACKSIZE - 1],
		TASKSTART_PRIORITY,
		TASKSTART_PRIORITY,
		taskStart_stk,
		TASK_STACKSIZE,
		NULL,
		0);

	OSStart();
	return 0;
}



