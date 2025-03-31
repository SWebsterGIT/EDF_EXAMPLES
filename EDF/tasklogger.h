#ifndef TASK_LOGGER_H
#define TASK_LOGGER_H

#include <stdint.h>
#include <stdbool.h>


void write_task_data_to_buffer(uint32_t timestamp, char* task_name, uint32_t core_num);

void write_CBS_data_to_buffer(uint32_t timestamp, char* task_name, uint32_t core_num, uint16_t budget_remaining);

void print_task_log_buffer_task_function(void *pvParameters);  // task function

#endif /* TASK_LOGGER_H */