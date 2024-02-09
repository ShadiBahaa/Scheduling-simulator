#ifndef _UTILITIES_H_
#define _UTILITIES_H_

typedef signed long int iterator_t;
typedef unsigned long int counter_t;

#define True 1
#define False 0

typedef unsigned char boolean_t;
typedef enum{
    ARRIVAL_TIME = 0,
    PROCESS_ID,
    EXCECUTION_TIME,
    WAITING_TIME,
    RESPONSE_TIME,
    TURNAROUND_TIME,
    CURRENT_CORE,
    TIME_CONSUMED,
    PROCESS_PRIORITY
}comparison_condition_e;

typedef enum{
    LESS_THAN = -1,
    EQUAL = 0,
    GREATER_THAN = 1
} comparison_status_e;

void merge_sort(process_info_t * arr, iterator_t left, iterator_t right, comparison_condition_e comparison_condition);

#endif