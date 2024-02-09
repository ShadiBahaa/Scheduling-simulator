#ifndef _PQUEUE_H_
#define _PQUEUE_H_

#define PROCESS_NOT_VALID -1

typedef signed char process_id_t;
typedef signed char process_core_t;
typedef unsigned char burst_type_t;
typedef signed long process_time_t;
typedef signed short process_priority_t;
typedef unsigned int process_stride_t;
typedef unsigned int process_pass_t;
typedef unsigned int process_tickets_t;
typedef signed short queue_pointers_t;
typedef signed short bursts_counter_t;
typedef void *(*process_handler_t)(void *);
typedef void *process_args_t;
typedef void *process_return_t;
typedef signed long int process_index_t;

typedef struct 
{
    burst_type_t burst_type;
    process_time_t execution_time;
} burst_info_t;

typedef enum{
    IO_BURST = 0,
    CPU_BURST
} burst_type_e;

typedef enum{
    TIME_SLICE_NOT_COMPLETE = 0,
    TIME_SLICE_COMPLETE = 1,
}time_slice_completion_e;

typedef enum
{
    BLOCKED = -1,
    READY = 0,
    RUN = 1,
    NOT_VALID = 2
} process_status_e;

typedef enum
{
    QUEUE_FULL = 0,
    QUEUE_NOT_FULL = 1,
    QUEUE_EMPTY = 2,
    QUEUE_NOT_EMPTY = 3
} queue_capacity_status_e;

typedef enum
{
    PROCESS_ENQUEUE_SUCCESSFUL = 0,
    PROCESS_ENQUEUE_FAILED = -1
} process_enqueue_status_e;

typedef enum
{
    PROCESS_DEQUEUE_SUCCESSFUL = 0,
    PROCESS_DEQUEUE_FAILED = -1
} process_dequeue_status_e;

typedef struct
{
    process_id_t pid;
    process_index_t process_index;
    process_status_e process_status;
    process_time_t execution_time;
    process_time_t arrival_time;
    process_time_t waiting_time;
    process_time_t total_duration;
    process_priority_t priority;
    process_tickets_t tickets;
    process_stride_t stride;
    process_pass_t pass;
    burst_info_t *burst_info;
    bursts_counter_t bursts_count;
    bursts_counter_t next_burst;
    process_handler_t process_handler;
    process_args_t proccess_args;
    process_return_t process_return;
    process_core_t original_core;
    process_core_t current_core;
    time_slice_completion_e time_slice_completion;
    process_time_t time_consumed;
    process_time_t response_time;
    process_time_t turnaround_time;
} process_info_t;

// define the function pointer type
typedef int (*property_match_func_t)(process_info_t* process_info, void * value);



void init_processes_queues(int numCores);
queue_capacity_status_e check_queue_full(process_status_e process_status, int core_number);
queue_capacity_status_e check_queue_empty(process_status_e process_status, int core_number);
int get_queue_size(process_status_e process_status, int core_number);
process_enqueue_status_e enqueue_process(process_info_t process_info, int core_number);
process_enqueue_status_e enqueue_process_priority(process_info_t process_info, int core_number);
process_enqueue_status_e enqueue_process_duration(process_info_t process_info, int core_number);
process_dequeue_status_e dequeue_process(process_status_e process_status, process_info_t *process_info, int core_number);
process_dequeue_status_e peek_process(process_status_e process_status, process_info_t **process_info, int core_number);
void update_waiting_time(int currentTime, int core_number);
process_info_t get_front(process_status_e process_status, process_core_t core_number);


process_dequeue_status_e get_process_with_property(process_status_e process_status, process_info_t *process_info, int core_number, property_match_func_t match_func, void * value);
int get_least_priority_process(process_status_e process_status, int core_number);
int get_least_current_burst_time_process(process_status_e process_status, int core_number);
int get_least_pass_process(process_status_e process_status, int core_number);
int get_random_process_id(process_status_e process_status, int core_number);
int priority_match(process_info_t* process_info, void * priority);
int current_burst_time_match(process_info_t* process_info, void * current_burst_time);
int pass_match(process_info_t* process_info, void * pass);
int pid_match(process_info_t* process_info, void * pid);
process_enqueue_status_e enqueue_process_front(process_info_t process_info, int core_number);

#endif