// Tracehooks.h
#ifndef TRACEHOOKS_H
#define TRACEHOOKS_H

#include <stdio.h>
#include <stdbool.h>

#include "tasklogger.h"


#define traceTASK_SWITCHED_IN() write_task_data_to_buffer(xTickCount, pxCurrentTCB->pcTaskName, portGET_CORE_ID()); \
if (pxCurrentTCB->is_CBS_server) {traceWRITE_CBS_LOG(pxCurrentTCB->CBS_server_instance.current_budget);} \

#define traceWRITE_CBS_LOG(budget) write_CBS_data_to_buffer(xTickCount, pxCurrentTCB->pcTaskName, portGET_CORE_ID(), budget)

#define traceWRITE_CBS_LOG_EXTERNAL_TASK(pxCBS_TCB, budget) write_CBS_data_to_buffer(xTickCount, pxCBS_TCB->pcTaskName, portGET_CORE_ID(), budget)

#define traceTASK_SWITCHED_OUT() if (pxCurrentTCB->is_CBS_server) {traceWRITE_CBS_LOG(pxCurrentTCB->CBS_server_instance.current_budget);}


#endif // TRACEHOOKS_H





