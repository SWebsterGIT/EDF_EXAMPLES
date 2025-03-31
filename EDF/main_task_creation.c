/*
 * FreeRTOS V202212.00
 * Copyright (C) 2020 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * https://www.FreeRTOS.org
 * https://github.com/FreeRTOS
 *
 */

/******************************************************************************
 

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"

/* Library includes. */
#include <stdio.h>
#include "hardware/gpio.h"

#include "tasklogger.h"

// #define DEBUG_PRINT
#ifdef DEBUG_PRINT
    #define debug_flagged_printf(...) printf(__VA_ARGS__)
#else
    #define debug_flagged_printf(...) do {} while (0)  // No-op
#endif


#define TEST_CASE_NUMBER 1

#if (TEST_CASE_NUMBER == 1)

	#define TASK_A_PERIOD	(1000)
	#define TASK_A_COST		(200)
	#define TASK_A_NAME     "1000ms"

	#define TASK_B_PERIOD 	(2000)
	#define TASK_B_COST		(200)
	#define TASK_B_NAME     "2000ms"

	#define TASK_C_PERIOD 	(5000)
	#define TASK_C_COST		(200)
	#define TASK_C_NAME     "5000ms"


#elif (TEST_CASE_NUMBER == 2)

	#define TASK_A_PERIOD	(1000)
	#define TASK_A_COST		(200)
	#define TASK_A_NAME     "1000ms"


	#define TASK_B_PERIOD 	(2000)
	#define TASK_B_COST		(200)
	#define TASK_B_NAME     "2000ms"


	#define TASK_C_PERIOD 	(5000)
	#define TASK_C_COST		(800)
	#define TASK_C_NAME     "5000ms"



#elif (TEST_CASE_NUMBER == 3)

	#define TASK_A_PERIOD	(3000)
	#define TASK_A_COST		(500)
	#define TASK_A_NAME     "3000ms"

	#define TASK_B_PERIOD 	(5000)
	#define TASK_B_COST		(3000)
	#define TASK_B_NAME     "5000ms"


	#define TASK_C_PERIOD 	(100000)
	#define TASK_C_COST		(0)
	#define TASK_C_NAME     "HIDE"

#elif (TEST_CASE_NUMBER == 4)

	#define TASK_A_PERIOD	(2000)
	#define TASK_A_COST		(400)
	#define TASK_A_NAME     "2000ms"

	#define TASK_B_PERIOD 	(3700)
	#define TASK_B_COST		(800)
	#define TASK_B_NAME     "3700ms"


	#define TASK_C_PERIOD 	(7300)
	#define TASK_C_COST		(600)
	#define TASK_C_NAME     "7300ms"


#elif (TEST_CASE_NUMBER == 5)

	#define TASK_A_PERIOD	(2000)
	#define TASK_A_COST		(400)
	#define TASK_A_NAME     "2000ms"


	#define TASK_B_PERIOD 	(4000)
	#define TASK_B_COST		(400)
	#define TASK_B_NAME     "4000ms"


	#define TASK_C_PERIOD 	(10000)
	#define TASK_C_COST		(800)
	#define TASK_C_NAME     "10000ms"

	#define CBS_1_PERIOD    (6000)
	#define CBS_1_BUDGET    (400)


#elif (TEST_CASE_NUMBER == 6)

	#define TASK_A_PERIOD	(2000)
	#define TASK_A_COST		(400)
	#define TASK_A_NAME     "2000ms"


	#define TASK_B_PERIOD 	(4000)
	#define TASK_B_COST		(400)
	#define TASK_B_NAME     "4000ms"


	#define TASK_C_PERIOD 	(10000)
	#define TASK_C_COST		(800)
	#define TASK_C_NAME     "10000ms"

	#define CBS_1_PERIOD    (1000)
	#define CBS_1_BUDGET    (400)


#endif


// Task Periods
#define main100_MS_PERIOD (1000 / portTICK_PERIOD_MS)
#define main200_MS_PERIOD (2000 / portTICK_PERIOD_MS)
#define main500_MS_PERIOD (5000 / portTICK_PERIOD_MS)

// The LED toggled 
#define mainTASK_LED (PICO_DEFAULT_LED_PIN)

/*-----------------------------------------------------------*/

/*
 * Called by main function in main.c - also starts the scheduler - never returns
 */
void main_task_creation(void);

/*------------------------ Static Functions -----------------------------------*/

static void vBusyWait(uint32_t delay_time_ms);

// Task Functions
static void Task_A_Periodic_Function(void *pvParameters);
static void Task_B_Periodic_Function(void *pvParameters); // TODO: try doing this with one function and just using paramters
static void Task_C_Periodic_Function(void *pvParameters); // TODO: try doing this with one function and just using paramters



// CBS STUFF
QueueHandle_t cbs_queue_handle_1;

#define CBS_QUEUE_LENGTH 20

#define CBS_MS_TO_NUMBER_OF_LOOPS_CONVERSION (8200) 
static void CBS_BusyWait(uint32_t delay_time_ms)
{
	uint32_t delay_num_loops = delay_time_ms * CBS_MS_TO_NUMBER_OF_LOOPS_CONVERSION;

	for (volatile uint32_t loop_count = 0; loop_count < delay_num_loops; loop_count++)
	{
		asm("nop");
	}

	return;
}

static void CBS_WAIT_200ms(void)
{
	CBS_BusyWait(200);
}

static void CBS_WAIT_400ms(void)
{
	CBS_BusyWait(400);
}


typedef void (*CBS_FUNCTION)(void);


static void cbs_server_dispatch_function(void *pvParameters)
{
    CBS_FUNCTION receivedFunction;

    for(;;)
    {
        // wait on the server queue
        if (xQueueReceive(cbs_queue_handle_1, &receivedFunction, portMAX_DELAY - 4) == pdPASS)
        {

            // Process the message - do some work
            // CBS_BusyWait(400);

			// Extension: Call a function pointer passed in queue
			if (receivedFunction)
			{
				receivedFunction();
			}
        }

    }
}

void main_task_creation(void)
{
	printf(" Starting main_task_creation.\n");

	// All task deadlines are set equal to their periods

	printf(" Created task A.\n");
	xEDFTaskCreate(Task_A_Periodic_Function, TASK_A_NAME, configMINIMAL_STACK_SIZE, NULL, TASK_A_PERIOD, NULL);

	printf(" Created task B.\n");
	xEDFTaskCreate(Task_B_Periodic_Function, TASK_B_NAME, configMINIMAL_STACK_SIZE, NULL, TASK_B_PERIOD, NULL);

	printf(" Created task C.\n");
	xEDFTaskCreate(Task_C_Periodic_Function, TASK_C_NAME, configMINIMAL_STACK_SIZE, NULL, TASK_C_PERIOD, NULL);

#if (TEST_CASE_NUMBER > 4)

#ifdef CBS_SERVER
	TaskHandle_t pxCreatedCBS_handle;
	cbs_queue_handle_1 = xQueueCreate( CBS_QUEUE_LENGTH, sizeof(CBS_FUNCTION));
	xEDFTaskCreate(cbs_server_dispatch_function, "CBS1", configMINIMAL_STACK_SIZE, NULL, CBS_1_PERIOD, &pxCreatedCBS_handle);

	init_CBS_server(pxCreatedCBS_handle, CBS_1_PERIOD, CBS_1_BUDGET);
#endif

#endif


	xTaskCreate(print_task_log_buffer_task_function, "Logger", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
	

	/* Start the tasks and timer running. */
	vTaskStartScheduler();

	/* If all is well, the scheduler will now be running, and the following
	line will never be reached.  If the following line does execute, then
	there was insufficient FreeRTOS heap memory available for the Idle and/or
	timer tasks to be created.  See the memory management section on the
	FreeRTOS web site for more details on the FreeRTOS heap
	http://www.freertos.org/a00111.html. */
	for (;;)
		;
}


/*---------------------------- Task Function Definitions -------------------------------*/

static void Task_A_Periodic_Function(void *pvParameters)
{
	TickType_t last_wake_time;

	/* Remove compiler warning about unused parameter. */
	(void)pvParameters;

	/* Initialise xNextWakeTime - this only needs to be done once. */
	last_wake_time = xTaskGetTickCount();

	for (;;)
	{

		/* Place this task in the blocked state until it is time to run again. */
		xEDFTaskDelayPeriodic(&last_wake_time, TASK_A_PERIOD);

		vBusyWait(TASK_A_COST);
	}
}

static void Task_B_Periodic_Function(void *pvParameters)
{
	TickType_t last_wake_time;

	/* Remove compiler warning about unused parameter. */
	(void)pvParameters;

	/* Initialise xNextWakeTime - this only needs to be done once. */
	last_wake_time = xTaskGetTickCount();

	for (;;)
	{

		/* Place this task in the blocked state until it is time to run again. */
		xEDFTaskDelayPeriodic(&last_wake_time, TASK_B_PERIOD);

		gpio_xor_mask(1u << mainTASK_LED);

		vBusyWait(TASK_B_COST);

		
	}
}

static void Task_C_Periodic_Function(void *pvParameters)
{
	TickType_t last_wake_time;

	/* Remove compiler warning about unused parameter. */
	(void)pvParameters;

	/* Initialise xNextWakeTime - this only needs to be done once. */
	last_wake_time = xTaskGetTickCount();

	for (;;)
	{
		/* Place this task in the blocked state until it is time to run again. */
		xEDFTaskDelayPeriodic(&last_wake_time, TASK_C_PERIOD);

		vBusyWait(TASK_C_COST/2);

#if (TEST_CASE_NUMBER == 5)
	CBS_FUNCTION func = CBS_WAIT_400ms;
	xQueueSend( cbs_queue_handle_1, &func, 0U );
#endif

#if (TEST_CASE_NUMBER == 6)
	CBS_FUNCTION func = CBS_WAIT_400ms;
	xQueueSend( cbs_queue_handle_1, &func, 0U );
#endif

		vBusyWait(TASK_C_COST/2);
	}
}

/*-----------------------------------------------------------*/

bool primed_to_print = false; // TODO remove?

/*
Causes a busy-wait without yielding to the scheduler.

Thus this method does not provide high precision delays on the ms scale.
Note that the default tickrate period in this example is 1ms.

Parameters:
delay_time_ms - relative delay time in ms.
*/
#define MS_TO_NUMBER_OF_LOOPS_CONVERSION (8060)
static void vBusyWait(uint32_t delay_time_ms)
{
	uint32_t delay_num_loops = delay_time_ms * MS_TO_NUMBER_OF_LOOPS_CONVERSION;

	for (volatile uint32_t loop_count = 0; loop_count < delay_num_loops; loop_count++) 
	{

		if (loop_count == delay_num_loops / 2)
		{
			char* task_name = pcTaskGetName(NULL); // Use NULL for current task
			debug_flagged_printf("\ncore %d, halfway through %s at time: %d\n", get_core_num(), task_name, xTaskGetTickCount());

		}
		asm("nop");
	}

	return;
}
/*-----------------------------------------------------------*/

