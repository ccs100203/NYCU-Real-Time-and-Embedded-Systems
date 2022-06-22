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

#define TASK1_PRIORITY      3
#define TASK2_PRIORITY      4
#define TASK3_PRIORITY      5

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
#ifndef   OS_uCOS_II_H
struct Output_buffer {
    INT32U time;
    char event;
    INT8U task1;
    INT8U task2;
    INT8U src;      /* lab3 */
};
#endif

// /* lab1 */
// extern struct Output_buffer output_buffer[10];
// extern int output_tail;

/* lab3 */
OS_EVENT *R1, *R2;
INT8U err;

#define SCEN2

void mytask1()
{
	int start;
	int end;
	int toDelay;
	start = OSTimeGet();
#ifdef SCEN2
	OSTimeDly(5 - start);
#else
	OSTimeDly(8 - start);
#endif
	

	OS_ENTER_CRITICAL();
#ifdef SCEN2
	OSTCBCur->compTime = 11;
#else
	OSTCBCur->compTime = 6;
#endif
	OSTCBCur->period = 0xff;
	OS_EXIT_CRITICAL();

	while (1) {

		OS_ENTER_CRITICAL();
		
		int locked1 = 0;
		int locked2 = 0;
		int step = 1;

		while (OSTCBCur->compTime > 0) {
			OS_EXIT_CRITICAL();

			OS_ENTER_CRITICAL();
			int i = 0;
			for (; i<output_tail; ++i) {
				if (output_buffer[i].event == 'l') {
					printf("%d\t%s\tR%d\t(Prio=%d changes to=%d)\n",
						output_buffer[i].time,
						"lock",
						output_buffer[i].src,
						output_buffer[i].task1,
						output_buffer[i].task2
					);
				} else if (output_buffer[i].event == 'u') {
					printf("%d\t%s\tR%d\t(Prio=%d changes to=%d)\n",
						output_buffer[i].time,
						"unlock",
						output_buffer[i].src,
						output_buffer[i].task1,
						output_buffer[i].task2
					);
				} else if(output_buffer[i].event == 'c') {
					printf("%d\t%s\t%d\t%d\n", output_buffer[i].time, "Complete", 
					output_buffer[i].task1, output_buffer[i].task2);
				}
			}
			output_tail = 0;

#ifdef SCEN2
			/* lock R2 */
			if(step == 1 && OSTCBCur->compTime <= 9) {
				++step;
				OSMutexPend(R2, 0xffff, &err);
			}

			/* lock R1 */
			if(step == 2 && OSTCBCur->compTime <= 6) {
				++step;
				OSMutexPend(R1, 0xffff, &err);
			}

			/* unlock R1 */
			if(step == 3 && OSTCBCur->compTime <= 3) {
				++step;
				OSMutexPost(R1);
			}
#else
			/* lock R1 */
			if(locked1 == 0 && OSTCBCur->compTime <= 4) {
				locked1 = 1;
				OSMutexPend(R1, 0xffff, &err);
			}

			/* lock R2 */
			if(locked2 == 0 && OSTCBCur->compTime <= 2) {
				locked2 = 1;
				OSMutexPend(R2, 0xffff, &err);
			}
#endif

			OS_EXIT_CRITICAL();

			OS_ENTER_CRITICAL();
		}

#ifdef SCEN2
		OSMutexPost(R2);
#else
		OSMutexPost(R2);
		OSMutexPost(R1);
#endif
		OS_EXIT_CRITICAL();

		end = OSTimeGet();

		OS_ENTER_CRITICAL();
		toDelay = (OSTCBCur->period) - (end-start);
		start += (OSTCBCur->period);
		OSTCBCur->compTime = 6;
		OS_EXIT_CRITICAL();

		OSTimeDly(toDelay);
	}
}

void mytask2()
{
	int start;
	int end;
	int toDelay;
	start = OSTimeGet();

#ifndef SCEN2
	OSTimeDly(4 - start);
#endif

	OS_ENTER_CRITICAL();
#ifdef SCEN2
	OSTCBCur->compTime = 12;
#else
	OSTCBCur->compTime = 6;
#endif
	OSTCBCur->period = 0xff;
	OS_EXIT_CRITICAL();

	while (1) {
		OS_ENTER_CRITICAL();
		int locked2 = 0;
		int step = 1;

		while (OSTCBCur->compTime > 0) {
			OS_EXIT_CRITICAL();

			OS_ENTER_CRITICAL();
			int i = 0;
			for (; i<output_tail; ++i) {
				if (output_buffer[i].event == 'l') {
					printf("%d\t%s\tR%d\t(Prio=%d changes to=%d)\n",
						output_buffer[i].time,
						"lock",
						output_buffer[i].src,
						output_buffer[i].task1,
						output_buffer[i].task2
					);
				} else if (output_buffer[i].event == 'u') {
					printf("%d\t%s\tR%d\t(Prio=%d changes to=%d)\n",
						output_buffer[i].time,
						"unlock",
						output_buffer[i].src,
						output_buffer[i].task1,
						output_buffer[i].task2
					);
				} else if(output_buffer[i].event == 'c') {
					printf("%d\t%s\t%d\t%d\n", output_buffer[i].time, "Complete", 
					output_buffer[i].task1, output_buffer[i].task2);
				} else if (output_buffer[i].event == 'p') {
					// printf("%d\t%s\t%d\t%d\n", output_buffer[i].time, "Preempt", 
					// output_buffer[i].task1, output_buffer[i].task2);
				}
			}
			output_tail = 0;

#ifdef SCEN2
			/* lock R1 */
			if(step == 1 && OSTCBCur->compTime <= 10) {
				++step;
				OSMutexPend(R1, 0xffff, &err);
			}

			/* lock R2 */
			if(step == 2 && OSTCBCur->compTime <= 4) {
				++step;
				OSMutexPend(R2, 0xffff, &err);
			}

			/* unlock R2 */
			if(step == 3 && OSTCBCur->compTime <= 2) {
				++step;
				OSMutexPost(R2);
			}
#else
			/* lock R2 */
			if(locked2 == 0 && OSTCBCur->compTime <= 4) {
				locked2 = 1;
				OSMutexPend(R2, 0xffff, &err);
			}
#endif

			OS_EXIT_CRITICAL();

			OS_ENTER_CRITICAL();
		}

#ifdef SCEN2
		OSMutexPost(R1);
#else
		OSMutexPost(R2);
#endif

		OS_EXIT_CRITICAL();

		end = OSTimeGet();

		OS_ENTER_CRITICAL();
		toDelay = (OSTCBCur->period) - (end-start);
		start += (OSTCBCur->period);
		OSTCBCur->compTime = 6;
		OS_EXIT_CRITICAL();

		OSTimeDly(toDelay);
	}
}

void mytask3()
{
	int start;
	int end;
	int toDelay;
	start = OSTimeGet();

	OS_ENTER_CRITICAL();
	OSTCBCur->compTime = 9;
	OSTCBCur->period = 0xff;
	OS_EXIT_CRITICAL();

	while (1) {
		OS_ENTER_CRITICAL();
		int locked1 = 0;
		while (OSTCBCur->compTime > 0) {
			OS_EXIT_CRITICAL();

			OS_ENTER_CRITICAL();
			int i = 0;
			for (; i<output_tail; ++i) {
				if (output_buffer[i].event == 'l') {
					printf("%d\t%s\tR%d\t(Prio=%d changes to=%d)\n",
						output_buffer[i].time,
						"lock",
						output_buffer[i].src,
						output_buffer[i].task1,
						output_buffer[i].task2
					);
				} else if (output_buffer[i].event == 'u') {
					printf("%d\t%s\tR%d\t(Prio=%d changes to=%d)\n",
						output_buffer[i].time,
						"unlock",
						output_buffer[i].src,
						output_buffer[i].task1,
						output_buffer[i].task2
					);
				} else if(output_buffer[i].event == 'c') {
					printf("%d\t%s\t%d\t%d\n", output_buffer[i].time, "Complete", 
					output_buffer[i].task1, output_buffer[i].task2);
				}
			}
			output_tail = 0;

			/* lock R2 */
			if(locked1 == 0 && OSTCBCur->compTime <= 7) {
				locked1 = 1;
				OSMutexPend(R1, 0xffff, &err);
			}
			OS_EXIT_CRITICAL();

			OS_ENTER_CRITICAL();
		}
		OSMutexPost(R1);
		OS_EXIT_CRITICAL();

		end = OSTimeGet();

		OS_ENTER_CRITICAL();
		toDelay = (OSTCBCur->period) - (end-start);
		start += (OSTCBCur->period);
		OSTCBCur->compTime = 9;
		OS_EXIT_CRITICAL();

		OSTimeDly(toDelay);
	}
}


/* The main function creates two task and starts multi-tasking */
int main(void)
{
	printf("Program Start\n");
	output_tail = 0;
	/* lab3 */
	R1 = OSMutexCreate(1, &err);
	R2 = OSMutexCreate(2, &err);
  
	OSTaskCreateExt(mytask1,
                  NULL,
                  (void *)&task1_stk[TASK_STACKSIZE-1],
                  TASK1_PRIORITY,
                  TASK1_PRIORITY,
                  task1_stk,
                  TASK_STACKSIZE,
                  NULL,
                  0);
              
               
  	OSTaskCreateExt(mytask2,
                  NULL,
                  (void *)&task2_stk[TASK_STACKSIZE-1],
                  TASK2_PRIORITY,
                  TASK2_PRIORITY,
                  task2_stk,
                  TASK_STACKSIZE,
                  NULL,
                  0);

#ifndef SCEN2
	OSTaskCreateExt(mytask3,
                  NULL,
                  (void *)&task3_stk[TASK_STACKSIZE-1],
                  TASK3_PRIORITY,
                  TASK3_PRIORITY,
                  task3_stk,
                  TASK_STACKSIZE,
                  NULL,
                  0);
#endif

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
