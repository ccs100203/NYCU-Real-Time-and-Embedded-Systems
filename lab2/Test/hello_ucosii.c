/*************************************************************************
* Copyright (c) 2004 Altera Corporation, San Jose, California, USA.      *
* All rights reserved. All use of this software and documentation is     *
* subject to the License Agreement located at the end of this file below.*
**************************************************************************
* Description:                                                           *
* The following is a simple hello world program running MicroC/OS-II.The * 
* purpose of the design is to be a very simple application that just     *
* demonstrates MicroC/OS-II running on NIOS II.The design doesn't account*
* for issues such as checking system call return codes. etc.             *
*                                                                        *
* Requirements:                                                          *
*   -Supported Example Hardware Platforms                                *
*     Standard                                                           *
*     Full Featured                                                      *
*     Low Cost                                                           *
*   -Supported Development Boards                                        *
*     Nios II Development Board, Stratix II Edition                      *
*     Nios Development Board, Stratix Professional Edition               *
*     Nios Development Board, Stratix Edition                            *
*     Nios Development Board, Cyclone Edition                            *
*   -System Library Settings                                             *
*     RTOS Type - MicroC/OS-II                                           *
*     Periodic System Timer                                              *
*   -Know Issues                                                         *
*     If this design is run on the ISS, terminal output will take several*
*     minutes per iteration.                                             *
**************************************************************************/


#include <stdio.h>
#include "includes.h"

/* Definition of Task Stacks */
#define   TASK_STACKSIZE       2048
OS_STK    task1_stk[TASK_STACKSIZE];
OS_STK    task2_stk[TASK_STACKSIZE];
OS_STK    task3_stk[TASK_STACKSIZE];

/* Definition of Task Priorities */

#define TASK1_PRIORITY      1
#define TASK2_PRIORITY      2
#define TASK3_PRIORITY      3

/* lab1 */
#define  OS_CPU_SR alt_irq_context  
    OS_CPU_SR  cpu_sr = 0;                                  /* Storage for CPU status register         */
#define  OS_ENTER_CRITICAL() \
         cpu_sr = alt_irq_disable_all ()
#define  OS_EXIT_CRITICAL() \
         alt_irq_enable_all (cpu_sr);

/* Prints "Hello World" and sleeps for three seconds */
void task1(void* pdata)
{
  while (1)
  { 
    printf("Hello from task1\n");
    OSTimeDlyHMSM(0, 0, 1, 0);
  }
}
/* Prints "Hello World" and sleeps for three seconds */
void task2(void* pdata)
{
  while (1)
  { 
    printf("Hello from task2\n");
    OSTimeDlyHMSM(0, 0, 3, 0);
  }
}

/* lab1 */ 
struct Output_buffer {
    INT32U time;
    char event;
    INT8U task1;
    INT8U task2;
};

/* lab1 */
extern struct Output_buffer output_buffer[10];
extern int output_tail;

/* lab2 */
extern INT32U deadline_list[65];
typedef struct task_args {
	int comp;
	int period;
} TASK_ARGS;


void mytask(void *args)
{
	int start;
	int end;
	int toDelay;

	/* lab2 */
	start = 0;

	OS_ENTER_CRITICAL();
	OSTCBCur->compTime = ((TASK_ARGS *)args)->comp;
	OSTCBCur->period = ((TASK_ARGS *)args)->period;
	OS_EXIT_CRITICAL();

	while (1) {

		OS_ENTER_CRITICAL();
		while (OSTCBCur->compTime > 0) {
			OS_EXIT_CRITICAL();

			OS_ENTER_CRITICAL();
			int i = 0;
			for (; i<output_tail; ++i) {
				if (output_buffer[i].event == 'p') {
					printf("%d\t%s\t%d\t%d\n", output_buffer[i].time, "Preempt", 
					output_buffer[i].task1, output_buffer[i].task2);
				} else if(output_buffer[i].event == 'c') {
					printf("%d\t%s\t%d\t%d\n", output_buffer[i].time, "Complete", 
					output_buffer[i].task1, output_buffer[i].task2);
				} else if(output_buffer[i].event == 'd') {
					printf("%d\t%s\t%d\n",  output_buffer[i].time, "Deadline Miss",
					output_buffer[i].task1);
				}
			}
			output_tail = 0;
			OS_EXIT_CRITICAL();

			OS_ENTER_CRITICAL();
		}
		OS_EXIT_CRITICAL();

		end = OSTimeGet();

		OS_ENTER_CRITICAL();
		toDelay = (OSTCBCur->period) - (end-start);
		start += (OSTCBCur->period);
		OSTCBCur->compTime = ((TASK_ARGS *)args)->comp;

		/* lab2 */
		INT32U deadline = (INT32U)OSTCBCur->period + (INT32U)start;
		/* update deadline */
		deadline_list[OSTCBCur->OSTCBPrio] = deadline;
		OS_EXIT_CRITICAL();

		OSTimeDly(toDelay);
	}
}

#define TASK_NUM 3

/* The main function creates two task and starts multi-tasking */
int main(void)
{
	TASK_ARGS args[TASK_NUM] = {
		{1, 4},
		{2, 5},
		{2, 10}
	};
  
 	 OSTaskCreateExt(mytask,
                  (void *)args,
                  (void *)&task1_stk[TASK_STACKSIZE-1],
                  TASK1_PRIORITY,
                  TASK1_PRIORITY,
                  task1_stk,
                  TASK_STACKSIZE,
                  NULL,
                  0);
              
               
  	OSTaskCreateExt(mytask,
                  (void *)(args+1),
                  (void *)&task2_stk[TASK_STACKSIZE-1],
                  TASK2_PRIORITY,
                  TASK2_PRIORITY,
                  task2_stk,
                  TASK_STACKSIZE,
                  NULL,
                  0);

	OSTaskCreateExt(mytask,
                  (void *)(args+2),
                  (void *)&task3_stk[TASK_STACKSIZE-1],
                  TASK3_PRIORITY,
                  TASK3_PRIORITY,
                  task3_stk,
                  TASK_STACKSIZE,
                  NULL,
                  0);

	/* lab2 */
	/* initialize deadline_list to the period*/
	memset(deadline_list, 0xFF, sizeof(deadline_list)); 
	int i = 0;
	for(; i < TASK_NUM; ++i) {
		deadline_list[i+1] = args[i].period;
	}
	OSStart();
	return 0;
}

/******************************************************************************
*                                                                             *
* License Agreement                                                           *
*                                                                             *
* Copyright (c) 2004 Altera Corporation, San Jose, California, USA.           *
* All rights reserved.                                                        *
*                                                                             *
* Permission is hereby granted, free of charge, to any person obtaining a     *
* copy of this software and associated documentation files (the "Software"),  *
* to deal in the Software without restriction, including without limitation   *
* the rights to use, copy, modify, merge, publish, distribute, sublicense,    *
* and/or sell copies of the Software, and to permit persons to whom the       *
* Software is furnished to do so, subject to the following conditions:        *
*                                                                             *
* The above copyright notice and this permission notice shall be included in  *
* all copies or substantial portions of the Software.                         *
*                                                                             *
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR  *
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,    *
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE *
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER      *
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING     *
* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER         *
* DEALINGS IN THE SOFTWARE.                                                   *
*                                                                             *
* This agreement shall be governed in all respects by the laws of the State   *
* of California and by the laws of the United States of America.              *
* Altera does not recommend, suggest or require that this reference design    *
* file be used in conjunction or combination with any other product.          *
******************************************************************************/
