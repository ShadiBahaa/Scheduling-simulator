#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include "pqueue.h"
#include "utilities.h"
#include "round_robin.h"
#include "mlfq.h"


#define MAX_MLFQ_PROCESSES_PER_QUEUE 26
static counter_t MAX_MLFQ_QUEUES = 0;
process_info_t *to_schedule = NULL;
static counter_t number_of_processes = 0;
extern process_info_t *finished_processes;
static process_core_t cores_number = 0;
static time_t *time_quanta;
volatile boolean_t mlfq_is_on = False;
extern FILE *logFile;
static time_t boost = 0;
static time_t current_time = 0;

typedef struct mlfq_node
{
    process_info_t process_info;
    struct mlfq_node *next;
} mlfq_node_t;

typedef mlfq_node_t *mlfq_node_ptr_t;

static void enqueue_mlfq_process(process_info_t process_info, mlfq_node_ptr_t *MLFQ_queues_heads, mlfq_node_ptr_t *MLFQ_queues_tails);
static process_info_t dequeue_mlfq_process(process_priority_t process_priority, mlfq_node_ptr_t *MLFQ_queues_heads, mlfq_node_ptr_t *MLFQ_queues_tails);
static void delete_specific_node(mlfq_node_ptr_t mlfq_node_ptr, mlfq_node_ptr_t *MLFQ_queues_heads, mlfq_node_ptr_t *MLFQ_queues_tails);
static boolean_t queue_is_empty(process_priority_t process_priority, mlfq_node_ptr_t *MLFQ_queues_heads);
static void schedule_mlfq(process_priority_t process_priority, mlfq_node_ptr_t *MLFQ_queues_heads, mlfq_node_ptr_t *MLFQ_queues_tails);

void run_mlfq_simulator(void){
    run_round_robin_simulator();
}
void run_mlfq_scheduler(void){
    run_round_robin_scheduler();
}
void send_mlfq_data(process_info_t *processes, counter_t processes_count)
{
    to_schedule = malloc(sizeof(process_info_t) * processes_count);
    to_schedule = processes;
    number_of_processes = processes_count;
}

static void enqueue_mlfq_process(process_info_t process_info, mlfq_node_ptr_t *MLFQ_queues_heads, mlfq_node_ptr_t *MLFQ_queues_tails)
{
    process_priority_t process_priority = process_info.priority;
    if (process_priority == -1)
    {
        process_info.priority = 0;
        process_priority = 0;
    }
    mlfq_node_ptr_t mlfq_node_ptr = malloc(sizeof(mlfq_node_t));
    mlfq_node_ptr->process_info = process_info;
    mlfq_node_ptr->next = NULL;
    if (MLFQ_queues_heads[process_priority] == NULL)
    {
        MLFQ_queues_heads[process_priority] = mlfq_node_ptr;
        MLFQ_queues_tails[process_priority] = mlfq_node_ptr;
    }
    else
    {
        MLFQ_queues_tails[process_priority]->next = mlfq_node_ptr;
        MLFQ_queues_tails[process_priority] = MLFQ_queues_tails[process_priority]->next;
    }
}
static process_info_t dequeue_mlfq_process(process_priority_t process_priority, mlfq_node_ptr_t *MLFQ_queues_heads, mlfq_node_ptr_t *MLFQ_queues_tails)
{
    process_info_t process_info;
    process_info = (MLFQ_queues_heads[process_priority])->process_info;
    mlfq_node_ptr_t tmp = MLFQ_queues_heads[process_priority];
    MLFQ_queues_heads[process_priority] = MLFQ_queues_heads[process_priority]->next;
    free(tmp);
    if (MLFQ_queues_heads[process_priority] == NULL)
    {
        MLFQ_queues_tails[process_priority] = NULL;
    }
    return process_info;
}
static void delete_specific_node(mlfq_node_ptr_t mlfq_node_ptr, mlfq_node_ptr_t *MLFQ_queues_heads, mlfq_node_ptr_t *MLFQ_queues_tails)
{
    mlfq_node_ptr_t current = MLFQ_queues_heads[(mlfq_node_ptr->process_info).priority];
    mlfq_node_ptr_t previous = NULL;
    while (current != mlfq_node_ptr)
    {
        previous = current;
        current = current->next;
    }
    if (current == MLFQ_queues_heads[(mlfq_node_ptr->process_info).priority])
    {
        MLFQ_queues_heads[(mlfq_node_ptr->process_info).priority] = MLFQ_queues_heads[(mlfq_node_ptr->process_info).priority]->next;
        if (MLFQ_queues_heads[(mlfq_node_ptr->process_info).priority] == NULL)
        {
            MLFQ_queues_tails[(mlfq_node_ptr->process_info).priority] = NULL;
        }
        else if (MLFQ_queues_heads[(mlfq_node_ptr->process_info).priority]->next == NULL)
        {
            MLFQ_queues_tails[(mlfq_node_ptr->process_info).priority] = MLFQ_queues_heads[(mlfq_node_ptr->process_info).priority];
        }
    }
    else
    {
        previous->next = current->next;
    }
    free(mlfq_node_ptr);
    mlfq_node_ptr = NULL;
}
static boolean_t queue_is_empty(process_priority_t process_priority, mlfq_node_ptr_t *MLFQ_queues_heads)
{
    return MLFQ_queues_heads[process_priority] == NULL;
}
static void schedule_mlfq(process_priority_t process_priority, mlfq_node_ptr_t *MLFQ_queues_heads, mlfq_node_ptr_t *MLFQ_queues_tails)
{
    while (!queue_is_empty(process_priority, MLFQ_queues_heads))
    {
        process_info_t process_info = dequeue_mlfq_process(process_priority, MLFQ_queues_heads, MLFQ_queues_tails);
        enqueue_round_robin_process(&process_info);
    }
}
void init_mlfq(process_core_t cores_count, time_t *time_slices, counter_t time_slices_count, time_t boost_time)
{
    cores_number = cores_count;
    MAX_MLFQ_QUEUES = time_slices_count;
    time_quanta = malloc(sizeof(time_t) * MAX_MLFQ_QUEUES);
    time_quanta = time_slices;
    boost = boost_time;
}
void run_mlfq(void)
{
    mlfq_node_ptr_t *MLFQ_queues_heads;
    mlfq_node_ptr_t *MLFQ_queues_tails;
    MLFQ_queues_heads = calloc(MAX_MLFQ_QUEUES, sizeof(mlfq_node_ptr_t));
    MLFQ_queues_tails = calloc(MAX_MLFQ_QUEUES, sizeof(mlfq_node_ptr_t));
    counter_t queues_counter;
    for (queues_counter = 0; queues_counter < MAX_MLFQ_QUEUES; ++queues_counter)
    {
        MLFQ_queues_heads[queues_counter] = NULL;
        MLFQ_queues_tails[queues_counter] = NULL;
    }
    counter_t process_counter;
    for (process_counter = 0; process_counter < number_of_processes; ++process_counter)
    {
        to_schedule[process_counter].priority = 0;
    }
    process_priority_t current_priority = -1;
    while (True)
    {
        ++current_priority;
        if (current_priority == MAX_MLFQ_QUEUES)
        {
            boolean_t done = True;
            for (process_counter = 0; process_counter < number_of_processes; ++process_counter)
            {
                if (to_schedule[process_counter].pid != PROCESS_NOT_VALID)
                {
                    done = False;
                    break;
                }
            }
            if (done)
                break;
            else
            {
                current_priority = 0;
            }
        }
        fprintf(logFile, "current queue : %d\n", current_priority);
        fprintf(logFile, "000000000000000000000000000000000000000000000\n");
        for (process_counter = 0; process_counter < number_of_processes; ++process_counter)
        {
            if ((to_schedule[process_counter].pid != PROCESS_NOT_VALID) && (to_schedule[process_counter].priority == current_priority))
            {
                //printf("counter : %lu , process pid: %c\n", process_counter, to_schedule[process_counter].pid);
                enqueue_mlfq_process(to_schedule[process_counter], MLFQ_queues_heads, MLFQ_queues_tails);
            }
        }
        if (queue_is_empty(current_priority, MLFQ_queues_heads))
        {
            continue;
        }
        mlfq_node_ptr_t current_queue_head = MLFQ_queues_heads[current_priority];
        init_processes_queues(cores_number);
        init_round_robin_processes(cores_number, MAX_MLFQ_PROCESSES_PER_QUEUE);
        schedule_mlfq(current_priority, MLFQ_queues_heads, MLFQ_queues_tails);
        if (current_queue_head == NULL)
            //printf("freed\n");
        //printf("second\n");
        mlfq_is_on = True;
        init_round_robin_scheduler(time_quanta[current_priority]);
        //printf("Stuck here\n");
        while (mlfq_is_on)
            ;
        for (process_counter = 0; process_counter < MAX_MLFQ_PROCESSES_PER_QUEUE; ++process_counter)
        {
            if (finished_processes[process_counter].priority != -1)
            {
                //printf("True! doesn't equal -1\n");
                if (finished_processes[process_counter].time_slice_completion == TIME_SLICE_COMPLETE)
                {
                    //printf("completed slice: pid: %c\n", finished_processes[process_counter].pid);
                    if (finished_processes[process_counter].priority + 1 < MAX_MLFQ_QUEUES)
                        finished_processes[process_counter].priority++;
                }
                finished_processes[process_counter].time_slice_completion = TIME_SLICE_NOT_COMPLETE;
                counter_t process_counter_2;
                for (process_counter_2 = 0; process_counter_2 < number_of_processes; ++process_counter_2)
                {
                    if (to_schedule[process_counter_2].pid == finished_processes[process_counter].pid)
                    {
                        if (finished_processes[process_counter].turnaround_time == -1)
                        {
                            //printf("Turnaround equal -1\n");
                            to_schedule[process_counter_2] = finished_processes[process_counter];
                        }
                        else
                        {
                            to_schedule[process_counter_2].pid = PROCESS_NOT_VALID;
                        }
                    }
                }
            }
        }
        current_time++;
        if (boost != 0 && (current_time % boost == 0))
        {
            for (process_counter = 0; process_counter < number_of_processes; ++process_counter)
            {
                to_schedule[process_counter].priority = 0;
                current_priority = -1;
            }
        }
    }
}