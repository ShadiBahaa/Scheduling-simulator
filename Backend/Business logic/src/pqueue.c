#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define QUEUE_CAPACITY 30
#define QUEUE_POINTER_INITIAL_VAL -1
#include "pqueue.h"

typedef struct
{
    process_info_t queue[QUEUE_CAPACITY];
    queue_pointers_t front, rear;

} processes_queue_t;

static processes_queue_t *ready_processes, *blocked_processes, *running_processes;

void init_processes_queues(int numCores)
{
    // create 3 queues for the 3 states for each core accessible by the core number and can be seen by all functions
    if (ready_processes != NULL)free(ready_processes);
    if (blocked_processes != NULL)free(blocked_processes);
    if (running_processes != NULL)free(running_processes);
    ready_processes = (processes_queue_t *)malloc(numCores * sizeof(processes_queue_t));
    blocked_processes = (processes_queue_t *)malloc(numCores * sizeof(processes_queue_t));
    running_processes = (processes_queue_t *)malloc(numCores * sizeof(processes_queue_t));
    for (int i = 0; i < numCores; i++)
    {
        ready_processes[i].front = QUEUE_POINTER_INITIAL_VAL;
        ready_processes[i].rear = QUEUE_POINTER_INITIAL_VAL;
        blocked_processes[i].front = QUEUE_POINTER_INITIAL_VAL;
        blocked_processes[i].rear = QUEUE_POINTER_INITIAL_VAL;
        running_processes[i].front = QUEUE_POINTER_INITIAL_VAL;
        running_processes[i].rear = QUEUE_POINTER_INITIAL_VAL;
    }
}
queue_capacity_status_e check_queue_full(process_status_e process_status, int core_number)
{
    queue_capacity_status_e return_status = QUEUE_NOT_FULL;
    switch (process_status)
    {
    case RUN:
        if ((running_processes[core_number].front == running_processes[core_number].rear + 1) || (running_processes[core_number].front == 0 && running_processes[core_number].rear == QUEUE_CAPACITY - 1))
        {
            return_status = QUEUE_FULL;
        }
        break;
    case BLOCKED:
        if ((blocked_processes[core_number].front == blocked_processes[core_number].rear + 1) || (blocked_processes[core_number].front == 0 && blocked_processes[core_number].rear == QUEUE_CAPACITY - 1))
        {
            return_status = QUEUE_FULL;
        }
        break;
    case READY:
        if ((ready_processes[core_number].front == ready_processes[core_number].rear + 1) || (ready_processes[core_number].front == 0 && ready_processes[core_number].rear == QUEUE_CAPACITY - 1))
        {
            return_status = QUEUE_FULL;
        }
        break;
    default:
        break;
    }
    return return_status;
}
queue_capacity_status_e check_queue_empty(process_status_e process_status, int core_number)
{
    queue_capacity_status_e return_status = QUEUE_NOT_EMPTY;
    switch (process_status)
    {
    case RUN:
        if (running_processes[core_number].front == QUEUE_POINTER_INITIAL_VAL)
        {
            return_status = QUEUE_EMPTY;
        }
        break;
    case BLOCKED:
        if (blocked_processes[core_number].front == QUEUE_POINTER_INITIAL_VAL)
        {
            return_status = QUEUE_EMPTY;
        }
        break;
    case READY:
        if (ready_processes[core_number].front == QUEUE_POINTER_INITIAL_VAL)
        {
            return_status = QUEUE_EMPTY;
        }
        break;
    default:
        break;
    }
    return return_status;
}
process_enqueue_status_e enqueue_process(process_info_t process_info, int core_number)
{
    process_status_e process_status = process_info.process_status;
    process_enqueue_status_e return_status = PROCESS_ENQUEUE_SUCCESSFUL;
    if (check_queue_full(process_status, core_number) == QUEUE_FULL)
    {
        return_status = PROCESS_ENQUEUE_FAILED;
    }
    else
    {
        switch (process_status)
        {
        case RUN:
            if (running_processes[core_number].front == QUEUE_POINTER_INITIAL_VAL)
            {
                running_processes[core_number].front = 0;
            }
            running_processes[core_number].rear = (running_processes[core_number].rear + 1) % QUEUE_CAPACITY;
            running_processes[core_number].queue[running_processes[core_number].rear] = process_info;
            break;
        case BLOCKED:
            if (blocked_processes[core_number].front == QUEUE_POINTER_INITIAL_VAL)
            {
                blocked_processes[core_number].front = 0;
            }
            blocked_processes[core_number].rear = (blocked_processes[core_number].rear + 1) % QUEUE_CAPACITY;
            blocked_processes[core_number].queue[blocked_processes[core_number].rear] = process_info;
            break;
        case READY:
            if (ready_processes[core_number].front == QUEUE_POINTER_INITIAL_VAL)
            {
                ready_processes[core_number].front = 0;
            }
            ready_processes[core_number].rear = (ready_processes[core_number].rear + 1) % QUEUE_CAPACITY;
            ready_processes[core_number].queue[ready_processes[core_number].rear] = process_info;
            break;
        default:
            break;
        }
    }
    // printf("enqueue process %d\n", process_info.pid); //debug
    // printf("enqueue process %d\n", process_info.process_status); //debug
    return return_status;
}
process_enqueue_status_e enqueue_process_priority(process_info_t process_info, int core_number)
{
    process_status_e process_status = process_info.process_status;
    process_enqueue_status_e return_status = PROCESS_ENQUEUE_SUCCESSFUL;
    if (check_queue_full(process_status, core_number) == QUEUE_FULL)
    {
        return_status = PROCESS_ENQUEUE_FAILED;
    }
    else
    {
        int i = 0;
        switch (process_status)
        {
        case RUN:
            if (running_processes[core_number].front == QUEUE_POINTER_INITIAL_VAL)
            {
                running_processes[core_number].front = 0;
            }
            running_processes[core_number].rear = (running_processes[core_number].rear + 1) % QUEUE_CAPACITY;
            i = running_processes[core_number].rear;
            // Shift elements with lower priority to make space for the new element
            while (i != 0 && process_info.priority < running_processes[core_number].queue[(i - 1) % QUEUE_CAPACITY].priority) {
                running_processes[core_number].queue[i] = running_processes[core_number].queue[(i - 1) % QUEUE_CAPACITY];
                i = (i - 1) % QUEUE_CAPACITY;
            }
            // Insert the new element at the correct position based on priority
            running_processes[core_number].queue[i] = process_info;
            break;
        case BLOCKED:
            if (blocked_processes[core_number].front == QUEUE_POINTER_INITIAL_VAL)
            {
                blocked_processes[core_number].front = 0;
            }
            blocked_processes[core_number].rear = (blocked_processes[core_number].rear + 1) % QUEUE_CAPACITY;
            i = blocked_processes[core_number].rear;
            // Shift elements with lower priority to make space for the new element
            while (i != 0 && process_info.priority < blocked_processes[core_number].queue[(i - 1) % QUEUE_CAPACITY].priority) {
                blocked_processes[core_number].queue[i] = blocked_processes[core_number].queue[(i - 1) % QUEUE_CAPACITY];
                i = (i - 1) % QUEUE_CAPACITY;
            }
            // Insert the new element at the correct position based on priority
            blocked_processes[core_number].queue[i] = process_info;
            break;
        case READY:
            if (ready_processes[core_number].front == QUEUE_POINTER_INITIAL_VAL)
            {
                ready_processes[core_number].front = 0;
            }
            ready_processes[core_number].rear = (ready_processes[core_number].rear + 1) % QUEUE_CAPACITY;
            i = ready_processes[core_number].rear;
            // Shift elements with lower priority to make space for the new element
            while (i != 0 && process_info.priority < ready_processes[core_number].queue[(i - 1) % QUEUE_CAPACITY].priority) {
                ready_processes[core_number].queue[i] = ready_processes[core_number].queue[(i - 1) % QUEUE_CAPACITY];
                i = (i - 1) % QUEUE_CAPACITY;
            }
            // Insert the new element at the correct position based on priority
            ready_processes[core_number].queue[i] = process_info;
            break;
        default:
            break;
        }
    }
    // printf("enqueue process %d\n", process_info.pid); //debug
    // printf("enqueue process %d\n", process_info.process_status); //debug
    return return_status;
}
process_enqueue_status_e enqueue_process_duration(process_info_t process_info, int core_number)
{
    process_status_e process_status = process_info.process_status;
    process_enqueue_status_e return_status = PROCESS_ENQUEUE_SUCCESSFUL;
    if (check_queue_full(process_status, core_number) == QUEUE_FULL)
    {
        return_status = PROCESS_ENQUEUE_FAILED;
    }
    else
    {
        int i = 0;
        switch (process_status)
        {
        case RUN:
            if (running_processes[core_number].front == QUEUE_POINTER_INITIAL_VAL)
            {
                running_processes[core_number].front = 0;
            }
            running_processes[core_number].rear = (running_processes[core_number].rear + 1) % QUEUE_CAPACITY;
            i = running_processes[core_number].rear;
            // Shift elements with lower total_Duration to make space for the new element
            while (i != 0 && process_info.total_duration < running_processes[core_number].queue[(i - 1) % QUEUE_CAPACITY].total_duration) {
                running_processes[core_number].queue[i] = running_processes[core_number].queue[(i - 1) % QUEUE_CAPACITY];
                i = (i - 1) % QUEUE_CAPACITY;
            }
            // Insert the new element at the correct position based on total_Duration
            running_processes[core_number].queue[i] = process_info;
            break;
        case BLOCKED:
            if (blocked_processes[core_number].front == QUEUE_POINTER_INITIAL_VAL)
            {
                blocked_processes[core_number].front = 0;
            }
            blocked_processes[core_number].rear = (blocked_processes[core_number].rear + 1) % QUEUE_CAPACITY;
            i = blocked_processes[core_number].rear;
            // Shift elements with lower total_Duration to make space for the new element
            while (i != 0 && process_info.total_duration < blocked_processes[core_number].queue[(i - 1) % QUEUE_CAPACITY].total_duration) {
                blocked_processes[core_number].queue[i] = blocked_processes[core_number].queue[(i - 1) % QUEUE_CAPACITY];
                i = (i - 1) % QUEUE_CAPACITY;
            }
            // Insert the new element at the correct position based on total_Duration
            blocked_processes[core_number].queue[i] = process_info;
            break;
        case READY:
            if (ready_processes[core_number].front == QUEUE_POINTER_INITIAL_VAL)
            {
                ready_processes[core_number].front = 0;
            }
            ready_processes[core_number].rear = (ready_processes[core_number].rear + 1) % QUEUE_CAPACITY;
            i = ready_processes[core_number].rear;
            // Shift elements with lower total_Duration to make space for the new element
            while (i != 0 && process_info.total_duration < ready_processes[core_number].queue[(i - 1) % QUEUE_CAPACITY].total_duration) {
                ready_processes[core_number].queue[i] = ready_processes[core_number].queue[(i - 1) % QUEUE_CAPACITY];
                i = (i - 1) % QUEUE_CAPACITY;
            }
            // Insert the new element at the correct position based on total_Duration
            ready_processes[core_number].queue[i] = process_info;
            break;
        default:
            break;
        }
    }
    // printf("enqueue process %d\n", process_info.pid); //debug
    // printf("enqueue process %d\n", process_info.process_status); //debug
    return return_status;
}
process_dequeue_status_e dequeue_process(process_status_e process_status, process_info_t *process_info, int core_number)
{
    process_dequeue_status_e return_status = PROCESS_DEQUEUE_SUCCESSFUL;
    // process_info = NULL;
    if (check_queue_empty(process_status, core_number) == QUEUE_EMPTY)
    {
        // printf("queue empty\n"); //debug
        return_status = PROCESS_ENQUEUE_FAILED;
    }
    else
    {
        switch (process_status)
        {
        case RUN:
            *process_info = (running_processes[core_number].queue[running_processes[core_number].front]);
            if (running_processes[core_number].front == running_processes[core_number].rear)
                running_processes[core_number].front = running_processes[core_number].rear = QUEUE_POINTER_INITIAL_VAL;
            else
                running_processes[core_number].front = (running_processes[core_number].front + 1) % QUEUE_CAPACITY;
            break;
        case BLOCKED:
            *process_info = (blocked_processes[core_number].queue[blocked_processes[core_number].front]);
            if (blocked_processes[core_number].front == blocked_processes[core_number].rear)
                blocked_processes[core_number].front = blocked_processes[core_number].rear = QUEUE_POINTER_INITIAL_VAL;
            else
                blocked_processes[core_number].front = (blocked_processes[core_number].front + 1) % QUEUE_CAPACITY;
            break;
        case READY:
            *process_info = (ready_processes[core_number].queue[ready_processes[core_number].front]);
            if (ready_processes[core_number].front == ready_processes[core_number].rear)
                ready_processes[core_number].front = ready_processes[core_number].rear = QUEUE_POINTER_INITIAL_VAL;
            else
                ready_processes[core_number].front = (ready_processes[core_number].front + 1) % QUEUE_CAPACITY;
            break;
            break;
        default:
            break;
        }
    }
    return return_status;
}

// a peek function like the dequeue function but without removing the process from the queue but only returning a pointer to it not a copy
process_dequeue_status_e peek_process(process_status_e process_status, process_info_t **process_info, int core_number)
{
    process_dequeue_status_e return_status = PROCESS_DEQUEUE_SUCCESSFUL;
    if (check_queue_empty(process_status, core_number) == QUEUE_EMPTY)
    {
        return_status = PROCESS_ENQUEUE_FAILED;
    }
    else
    {
        switch (process_status)
        {
        case RUN:
            *process_info = &(running_processes[core_number].queue[running_processes[core_number].front]);
            break;
        case BLOCKED:
            *process_info = &(blocked_processes[core_number].queue[blocked_processes[core_number].front]);
            break;
        case READY:
            *process_info = &(ready_processes[core_number].queue[ready_processes[core_number].front]);
            break;
        default:
            break;
        }
    }
    return return_status;
}
void update_waiting_time(int currentTime, int core_number)
{
    // printf("update waiting time\n"); //debug
    queue_pointers_t queue_pointer;
    // printf("front %d\n", blocked_processes[core_number].front); //debug
    // printf("rear %d\n", blocked_processes[core_number].rear); //debug
    if (check_queue_empty(BLOCKED, core_number) == QUEUE_EMPTY)
    {
        // printf("blocked empty not updating\n"); //debug
        return;
    }

    for (queue_pointer = blocked_processes[core_number].front; queue_pointer <= blocked_processes[core_number].rear; queue_pointer = (queue_pointer + 1) % QUEUE_CAPACITY)
    {
        // printf("queue pointer %d\n", queue_pointer); //debug
        // printf("update waiting time for process %d\n", blocked_processes[core_number].queue[queue_pointer].pid); //debug
        process_info_t *process_info_ptr = &(blocked_processes[core_number].queue[queue_pointer]);
        process_info_ptr->waiting_time--;
        // printf("waiting time %d\n", process_info_ptr->waiting_time); //debug
        if (blocked_processes[core_number].queue[queue_pointer].waiting_time == 0)
        {
            // printf("process %d unblocked\n", blocked_processes[core_number].queue[queue_pointer].pid); //debug
            // printf("burst count %d\n", process_info_ptr->bursts_count); //debug
            // printf("next burst %d\n", process_info_ptr->next_burst); //debug
            // remove that specific process from blocked regardless of its position in the queue
            process_info_t *temp = (process_info_t *)malloc(sizeof(process_info_t));
            for (int i = blocked_processes[core_number].front; i <= blocked_processes[core_number].rear; i = (i + 1) % QUEUE_CAPACITY)
            {
                if (blocked_processes[core_number].queue[i].pid == process_info_ptr->pid)
                {
                    *temp = blocked_processes[core_number].queue[i];
                    // printf("found process %d\n", temp->pid); //debug
                    if (i == blocked_processes[core_number].front)
                    {
                        if (blocked_processes[core_number].front == blocked_processes[core_number].rear)
                            blocked_processes[core_number].front = blocked_processes[core_number].rear = QUEUE_POINTER_INITIAL_VAL;
                        else
                            blocked_processes[core_number].front = (blocked_processes[core_number].front + 1) % QUEUE_CAPACITY;
                    }
                    else if (i == blocked_processes[core_number].rear)
                    {
                        blocked_processes[core_number].rear = (blocked_processes[core_number].rear - 1) % QUEUE_CAPACITY;
                    }
                    else
                    {
                        for (int j = i; j < blocked_processes[core_number].rear; j = (j + 1) % QUEUE_CAPACITY)
                        {
                            blocked_processes[core_number].queue[j] = blocked_processes[core_number].queue[j + 1];
                        }
                        blocked_processes[core_number].rear = (blocked_processes[core_number].rear - 1) % QUEUE_CAPACITY;
                    }
                    break;
                }
            }
            free(temp);
            if (process_info_ptr->next_burst < process_info_ptr->bursts_count)
            {
                // process is not finished yet, set the time it finished at and enqueue it in the ready queue
                process_info_ptr->waiting_time = currentTime;
            }
            else
            {
                // process has finished
                process_info_ptr->waiting_time = 9999;
                process_info_ptr->execution_time = currentTime;
            }
            process_info_ptr->process_status = READY;

            // determin which core to enqueue the process in
            // if the process was running on a core, enqueue it in the same core it was running on if it is free
            int core_to_enqueue = process_info_ptr->original_core;
            if (check_queue_empty(RUN, core_to_enqueue) == QUEUE_EMPTY && check_queue_empty(READY, core_to_enqueue) == QUEUE_EMPTY)
            {
                enqueue_process(*process_info_ptr, core_to_enqueue);
            }
            else
            {
                // see if any core is free
                for (int i = 0; i < core_number; i++)
                {
                    if (check_queue_empty(RUN, i) == QUEUE_EMPTY && check_queue_empty(READY, i) == QUEUE_EMPTY)
                    {
                        core_to_enqueue = i;
                        break;
                    }
                }

                // if no core is free enqueue it in the same core it was running on
                enqueue_process(*process_info_ptr, core_to_enqueue);
            }
        }
    }
}



// a function to enquee a process in the front of the queue by looping through the queue and shifting all elements to the right
process_enqueue_status_e enqueue_process_front(process_info_t process_info, int core_number)
{
    process_status_e process_status = process_info.process_status;
    process_enqueue_status_e return_status = PROCESS_ENQUEUE_SUCCESSFUL;
    if (check_queue_full(process_status, core_number) == QUEUE_FULL)
    {
        return_status = PROCESS_ENQUEUE_FAILED;
    }
    else
    {
        switch (process_status)
        {
        case RUN:
            if (running_processes[core_number].front == QUEUE_POINTER_INITIAL_VAL)
            {
                running_processes[core_number].front = 0;
            }
            running_processes[core_number].rear = (running_processes[core_number].rear + 1) % QUEUE_CAPACITY;
            for (int i = running_processes[core_number].rear; i > running_processes[core_number].front; i = (i - 1) % QUEUE_CAPACITY)
            {
                running_processes[core_number].queue[i] = running_processes[core_number].queue[i - 1];
            }
            running_processes[core_number].queue[running_processes[core_number].front] = process_info;
            break;
        case BLOCKED:
            if (blocked_processes[core_number].front == QUEUE_POINTER_INITIAL_VAL)
            {
                blocked_processes[core_number].front = 0;
            }
            blocked_processes[core_number].rear = (blocked_processes[core_number].rear + 1) % QUEUE_CAPACITY;
            for (int i = blocked_processes[core_number].rear; i > blocked_processes[core_number].front; i = (i - 1) % QUEUE_CAPACITY)
            {
                blocked_processes[core_number].queue[i] = blocked_processes[core_number].queue[i - 1];
            }
            blocked_processes[core_number].queue[blocked_processes[core_number].front] = process_info;
            break;
        case READY:
            if (ready_processes[core_number].front == QUEUE_POINTER_INITIAL_VAL)
            {
                ready_processes[core_number].front = 0;
            }
            ready_processes[core_number].rear = (ready_processes[core_number].rear + 1) % QUEUE_CAPACITY;
            for (int i = ready_processes[core_number].rear; i > ready_processes[core_number].front; i = (i - 1) % QUEUE_CAPACITY)
            {
                ready_processes[core_number].queue[i] = ready_processes[core_number].queue[i - 1];
            }
            ready_processes[core_number].queue[ready_processes[core_number].front] = process_info;
            break;
        default:
            break;
        }
    }
    return return_status;
}


process_dequeue_status_e get_process_with_property(process_status_e process_status, process_info_t *process_info, int core_number, property_match_func_t match_func, void * value)
{
    process_dequeue_status_e return_status = PROCESS_DEQUEUE_SUCCESSFUL;
    if (check_queue_empty(process_status, core_number) == QUEUE_EMPTY)
    {
        return_status = PROCESS_ENQUEUE_FAILED;
    }
    else
    {
        switch (process_status)
        {
        case RUN:
            for (int i = running_processes[core_number].front; i <= running_processes[core_number].rear; i = (i + 1) % QUEUE_CAPACITY)
            {
                if (match_func(&(running_processes[core_number].queue[i]), value))
                {
                    *process_info = running_processes[core_number].queue[i];
                    if (i == running_processes[core_number].front)
                    {
                        if (running_processes[core_number].front == running_processes[core_number].rear)
                            running_processes[core_number].front = running_processes[core_number].rear = QUEUE_POINTER_INITIAL_VAL;
                        else
                            running_processes[core_number].front = (running_processes[core_number].front + 1) % QUEUE_CAPACITY;
                    }
                    else if (i == running_processes[core_number].rear)
                    {
                        running_processes[core_number].rear = (running_processes[core_number].rear - 1) % QUEUE_CAPACITY;
                    }
                    else
                    {
                        for (int j = i; j < running_processes[core_number].rear; j = (j + 1) % QUEUE_CAPACITY)
                        {
                            running_processes[core_number].queue[j] = running_processes[core_number].queue[j + 1];
                        }
                        running_processes[core_number].rear = (running_processes[core_number].rear - 1) % QUEUE_CAPACITY;
                    }
                    break;
                }
            }
            break;
        case BLOCKED:
            for (int i = blocked_processes[core_number].front; i <= blocked_processes[core_number].rear; i = (i + 1) % QUEUE_CAPACITY)
            {
                if (match_func(&(blocked_processes[core_number].queue[i]), value))
                {
                    *process_info = blocked_processes[core_number].queue[i];
                    if (i == blocked_processes[core_number].front)
                    {
                        if (blocked_processes[core_number].front == blocked_processes[core_number].rear)
                            blocked_processes[core_number].front = blocked_processes[core_number].rear = QUEUE_POINTER_INITIAL_VAL;
                        else
                            blocked_processes[core_number].front = (blocked_processes[core_number].front + 1) % QUEUE_CAPACITY;
                    }
                    else if (i == blocked_processes[core_number].rear)
                    {
                        blocked_processes[core_number].rear = (blocked_processes[core_number].rear - 1) % QUEUE_CAPACITY;
                    }
                    else
                    {
                        for (int j = i; j < blocked_processes[core_number].rear; j = (j + 1) % QUEUE_CAPACITY)
                        {
                            blocked_processes[core_number].queue[j] = blocked_processes[core_number].queue[j + 1];
                        }
                        blocked_processes[core_number].rear = (blocked_processes[core_number].rear - 1) % QUEUE_CAPACITY;
                    }
                    break;
                }
            }
            break;
        case READY:
            for (int i = ready_processes[core_number].front; i <= ready_processes[core_number].rear; i = (i + 1) % QUEUE_CAPACITY)
            {
                if (match_func(&(ready_processes[core_number].queue[i]), value))
                {
                    *process_info = ready_processes[core_number].queue[i];
                    if (i == ready_processes[core_number].front)
                    {
                        if (ready_processes[core_number].front == ready_processes[core_number].rear)
                            ready_processes[core_number].front = ready_processes[core_number].rear = QUEUE_POINTER_INITIAL_VAL;
                        else
                            ready_processes[core_number].front = (ready_processes[core_number].front + 1) % QUEUE_CAPACITY;
                    }
                    else if (i == ready_processes[core_number].rear)
                    {
                        ready_processes[core_number].rear = (ready_processes[core_number].rear - 1) % QUEUE_CAPACITY;
                    }
                    else
                    {
                        for (int j = i; j < ready_processes[core_number].rear; j = (j + 1) % QUEUE_CAPACITY)
                        {
                            ready_processes[core_number].queue[j] = ready_processes[core_number].queue[j + 1];
                        }
                        ready_processes[core_number].rear = (ready_processes[core_number].rear - 1) % QUEUE_CAPACITY;
                    }
                    break;
                }
            }
            break;
        default:
            break;
        }
    }
    return return_status;
}

// function to get the value of the least priority process in the queue
int get_least_priority_process(process_status_e process_status, int core_number)
{
    int least_priority = 9999;
    if (check_queue_empty(process_status, core_number) == QUEUE_EMPTY)
    {
        return least_priority;
    }
    else
    {
        switch (process_status)
        {
        case RUN:
            for (int i = running_processes[core_number].front; i <= running_processes[core_number].rear; i = (i + 1) % QUEUE_CAPACITY)
            {
                if (running_processes[core_number].queue[i].priority < least_priority)
                {
                    least_priority = running_processes[core_number].queue[i].priority;
                }
            }
            break;
        case BLOCKED:
            for (int i = blocked_processes[core_number].front; i <= blocked_processes[core_number].rear; i = (i + 1) % QUEUE_CAPACITY)
            {
                if (blocked_processes[core_number].queue[i].priority < least_priority)
                {
                    least_priority = blocked_processes[core_number].queue[i].priority;
                }
            }
            break;
        case READY:
            for (int i = ready_processes[core_number].front; i <= ready_processes[core_number].rear; i = (i + 1) % QUEUE_CAPACITY)
            {
                if (ready_processes[core_number].queue[i].priority < least_priority)
                {
                    least_priority = ready_processes[core_number].queue[i].priority;
                }
            }
            break;
        default:
            break;
        }
    }
    return least_priority;
}

// function to get the value of the least current burst time process in the queue
int get_least_current_burst_time_process(process_status_e process_status, int core_number)
{
    int least_current_burst_time = 9999;
    if (check_queue_empty(process_status, core_number) == QUEUE_EMPTY)
    {
        return least_current_burst_time;
    }
    else
    {
        switch (process_status)
        {
        case RUN:
            for (int i = running_processes[core_number].front; i <= running_processes[core_number].rear; i = (i + 1) % QUEUE_CAPACITY)
            {
                if (running_processes[core_number].queue[i].burst_info[running_processes[core_number].queue[i].next_burst].execution_time < least_current_burst_time)
                {
                    least_current_burst_time = running_processes[core_number].queue[i].burst_info[running_processes[core_number].queue[i].next_burst].execution_time;
                }
            }
            break;
        case BLOCKED:
            for (int i = blocked_processes[core_number].front; i <= blocked_processes[core_number].rear; i = (i + 1) % QUEUE_CAPACITY)
            {
                if (blocked_processes[core_number].queue[i].burst_info[blocked_processes[core_number].queue[i].next_burst].execution_time < least_current_burst_time)
                {
                    least_current_burst_time = blocked_processes[core_number].queue[i].burst_info[blocked_processes[core_number].queue[i].next_burst].execution_time;
                }
            }
            break;
        case READY:
            for (int i = ready_processes[core_number].front; i <= ready_processes[core_number].rear; i = (i + 1) % QUEUE_CAPACITY)
            {
                if (ready_processes[core_number].queue[i].burst_info[ready_processes[core_number].queue[i].next_burst].execution_time < least_current_burst_time)
                {
                    least_current_burst_time = ready_processes[core_number].queue[i].burst_info[ready_processes[core_number].queue[i].next_burst].execution_time;
                }
            }
            break;
        default:
            break;
        }
    }
    return least_current_burst_time;
}

// function to get the value of the least pass of the process in the queue
int get_least_pass_process(process_status_e process_status, int core_number)
{
    unsigned int least_pass = 999999;
    if (check_queue_empty(process_status, core_number) == QUEUE_EMPTY)
    {
        return least_pass;
    }
    else
    {
        switch (process_status)
        {
        case RUN:
            for (int i = running_processes[core_number].front; i <= running_processes[core_number].rear; i = (i + 1) % QUEUE_CAPACITY)
            {
                if (running_processes[core_number].queue[i].pass < least_pass)
                {
                    least_pass = running_processes[core_number].queue[i].pass;
                }
            }
            break;
        case BLOCKED:
            for (int i = blocked_processes[core_number].front; i <= blocked_processes[core_number].rear; i = (i + 1) % QUEUE_CAPACITY)
            {
                if (blocked_processes[core_number].queue[i].pass < least_pass)
                {
                    least_pass = blocked_processes[core_number].queue[i].pass;
                }
            }
            break;
        case READY:
            for (int i = ready_processes[core_number].front; i <= ready_processes[core_number].rear; i = (i + 1) % QUEUE_CAPACITY)
            {
                if (ready_processes[core_number].queue[i].pass < least_pass)
                {
                    least_pass = ready_processes[core_number].queue[i].pass;
                }
            }
            break;
        default:
            break;
        }
    }
    return least_pass;
}

// function to get a random process id from the queue based on the tickets of each process
int get_random_process_id(process_status_e process_status, int core_number)
{
    int random_process_id = -1;
    int total_tickets = 0;
    if (check_queue_empty(process_status, core_number) == QUEUE_EMPTY)
    {
        return random_process_id;
    }
    else
    {
        switch (process_status)
        {
        case RUN:
            for (int i = running_processes[core_number].front; i <= running_processes[core_number].rear; i = (i + 1) % QUEUE_CAPACITY)
            {
                total_tickets += running_processes[core_number].queue[i].tickets;
            }
            break;
        case BLOCKED:
            for (int i = blocked_processes[core_number].front; i <= blocked_processes[core_number].rear; i = (i + 1) % QUEUE_CAPACITY)
            {
                total_tickets += blocked_processes[core_number].queue[i].tickets;
            }
            break;
        case READY:
            for (int i = ready_processes[core_number].front; i <= ready_processes[core_number].rear; i = (i + 1) % QUEUE_CAPACITY)
            {
                total_tickets += ready_processes[core_number].queue[i].tickets;
            }
            break;
        default:
            break;
        }
    }
    int random_ticket = rand() % total_tickets;
    // printf("random ticket %d\n", random_ticket); //debug
    int current_ticket = 0;
    switch (process_status)
    {
    case RUN:
        for (int i = running_processes[core_number].front; i <= running_processes[core_number].rear; i = (i + 1) % QUEUE_CAPACITY)
        {
            current_ticket += running_processes[core_number].queue[i].tickets;
            if (current_ticket > random_ticket)
            {
                random_process_id = running_processes[core_number].queue[i].pid;
                break;
            }
        }
        break;
    case BLOCKED:
        for (int i = blocked_processes[core_number].front; i <= blocked_processes[core_number].rear; i = (i + 1) % QUEUE_CAPACITY)
        {
            current_ticket += blocked_processes[core_number].queue[i].tickets;
            if (current_ticket > random_ticket)
            {
                random_process_id = blocked_processes[core_number].queue[i].pid;
                break;
            }
        }
        break;
    case READY:
        for (int i = ready_processes[core_number].front; i <= ready_processes[core_number].rear; i = (i + 1) % QUEUE_CAPACITY)
        {
            current_ticket += ready_processes[core_number].queue[i].tickets;
            if (current_ticket > random_ticket)
            {
                random_process_id = ready_processes[core_number].queue[i].pid;
                break;
            }
        }
        break;
    default:
        break;
    }
    return random_process_id;
}


// a matching function to see if the process given matches the priority or not
int priority_match(process_info_t* process_info, void * priority)
{
    if (process_info->priority == *(int *)priority)
        return 1;
    else
        return 0;
}

// a matching function to see if the process given matches the current burst time or not
int current_burst_time_match(process_info_t* process_info, void * current_burst_time)
{

    if (process_info->burst_info[process_info->next_burst].execution_time == *(int *)current_burst_time)
        return 1;
    else
        return 0;
}

// a matching function to see if the process given matches the pass or not
int pass_match(process_info_t* process_info, void * pass)
{
    if (process_info->pass == *(unsigned int *)pass)
        return 1;
    else
        return 0;
}

// a matching function to see if the process given matches the pid or not
int pid_match(process_info_t* process_info, void * pid)
{
    if (process_info->pid == *(int *)pid)
        return 1;
    else
        return 0;
}

// function to get the size of the queue
int get_queue_size(process_status_e process_status, int core_number)
{
    int queue_size = 0;
    if (check_queue_empty(process_status, core_number) == QUEUE_EMPTY)
    {
        return queue_size;
    }
    else
    {
        switch (process_status)
        {
        case RUN:
            for (int i = running_processes[core_number].front; i <= running_processes[core_number].rear; i = (i + 1) % QUEUE_CAPACITY)
            {
                queue_size++;
            }
            break;
        case BLOCKED:
            for (int i = blocked_processes[core_number].front; i <= blocked_processes[core_number].rear; i = (i + 1) % QUEUE_CAPACITY)
            {
                queue_size++;
            }
            break;
        case READY:
            for (int i = ready_processes[core_number].front; i <= ready_processes[core_number].rear; i = (i + 1) % QUEUE_CAPACITY)
            {
                queue_size++;
            }
            break;
        default:
            break;
        }
    }
    return queue_size;
}

process_info_t get_front(process_status_e process_status, process_core_t core_number)
{
    process_info_t process_info;
    process_info.pid = PROCESS_NOT_VALID;
    // process_info = NULL;
    if (check_queue_empty(process_status, core_number) == QUEUE_EMPTY)
    {
    }
    else
    {
        switch (process_status)
        {
        case RUN:
            process_info = running_processes[core_number].queue[running_processes[core_number].front];
            break;
        case BLOCKED:
            process_info = blocked_processes[core_number].queue[blocked_processes[core_number].front];
            break;
        case READY:
            process_info = ready_processes[core_number].queue[ready_processes[core_number].front];
            break;
        default:
            break;
        }
    }
    return process_info;
}