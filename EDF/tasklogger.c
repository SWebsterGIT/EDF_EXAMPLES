#include "tasklogger.h"
#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"

#include <string.h>

#define TASK_SWITCH_LOG_BUFFER_SIZE 200

typedef struct {
    uint32_t timestamp;
    char task_name[16];
    uint32_t core_num;
} TaskSwitchLog;

static volatile TaskSwitchLog task_switch_log_buffer[TASK_SWITCH_LOG_BUFFER_SIZE];
static volatile uint16_t tsl_buffer_write_index = 0; 
static volatile uint16_t tsl_buffer_read_index = 0; 
static volatile bool tsl_buffer_full_flag = false; 


#define CBS_LOG_BUFFER_SIZE 200

typedef struct {
    uint32_t timestamp;
    char task_name[16];
    uint32_t core_num;
    uint16_t budget_remaining;

} CBS_LOG;

static volatile CBS_LOG cbs_log_buffer[CBS_LOG_BUFFER_SIZE];
static volatile uint16_t cbs_log_buffer_write_index = 0; 
static volatile uint16_t cbs_log_buffer_read_index = 0; 
static volatile bool cbs_log_buffer_full_flag = false; 


void write_task_data_to_buffer(uint32_t timestamp, char* task_name, uint32_t core_num)
{
    // check if buffer is full: if so drop new data (this is a problem)
    if (tsl_buffer_full_flag) {return;}

    task_switch_log_buffer[tsl_buffer_write_index].timestamp = timestamp;
    task_switch_log_buffer[tsl_buffer_write_index].core_num = core_num;

    memcpy((void*)task_switch_log_buffer[tsl_buffer_write_index].task_name,task_name,16);

    /* Barriers ensure the code is completely within the specified behaviour. */
    __asm volatile ( "dsb" ::: "memory" );
    __asm volatile ( "isb" );

    tsl_buffer_write_index = (tsl_buffer_write_index + 1) % TASK_SWITCH_LOG_BUFFER_SIZE;

    if (tsl_buffer_write_index == tsl_buffer_read_index)
    {
        tsl_buffer_full_flag = true;
    }

    return;
}

void write_CBS_data_to_buffer(uint32_t timestamp, char* task_name, uint32_t core_num, uint16_t budget_remaining)
{
    // check if buffer is full: if so drop new data (this is a problem)
    if (cbs_log_buffer_full_flag) {return;}

    cbs_log_buffer[cbs_log_buffer_write_index].timestamp = timestamp;
    cbs_log_buffer[cbs_log_buffer_write_index].core_num = core_num;
    cbs_log_buffer[cbs_log_buffer_write_index].budget_remaining = budget_remaining;

    memcpy((void*)cbs_log_buffer[cbs_log_buffer_write_index].task_name,task_name,16);

    /* Barriers ensure the code is completely within the specified behaviour. */
    __asm volatile ( "dsb" ::: "memory" );
    __asm volatile ( "isb" );

    cbs_log_buffer_write_index = (cbs_log_buffer_write_index + 1) % CBS_LOG_BUFFER_SIZE;

    if (cbs_log_buffer_write_index == cbs_log_buffer_read_index)
    {
        cbs_log_buffer_full_flag = true;
    }

    return;
}


void print_task_log_buffer_task_function(void *pvParameters) 
{
    for(;;) 
    {
        while ((tsl_buffer_read_index != tsl_buffer_write_index) || (tsl_buffer_full_flag)) 
        {
            TaskSwitchLog entry = task_switch_log_buffer[tsl_buffer_read_index];
            
            __asm volatile ( "dsb" ::: "memory" );
            __asm volatile ( "isb" );

            tsl_buffer_read_index = (tsl_buffer_read_index + 1) % TASK_SWITCH_LOG_BUFFER_SIZE;

            tsl_buffer_full_flag = false;

            printf("A,%s,%d,%d\n\n", entry.task_name, entry.timestamp, entry.core_num);
        }

        while ((cbs_log_buffer_read_index != cbs_log_buffer_write_index) || (cbs_log_buffer_full_flag)) 
        {
            CBS_LOG entry = cbs_log_buffer[cbs_log_buffer_read_index];
            
            __asm volatile ( "dsb" ::: "memory" );
            __asm volatile ( "isb" );

            cbs_log_buffer_read_index = (cbs_log_buffer_read_index + 1) % CBS_LOG_BUFFER_SIZE;

            cbs_log_buffer_full_flag = false;

            printf("B,%s,%d,%d,%d\n\n", entry.task_name, entry.timestamp, entry.core_num, entry.budget_remaining);
        }

    }
}