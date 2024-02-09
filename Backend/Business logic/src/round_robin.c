#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include "pqueue.h"
#include "utilities.h"
#include "round_robin.h"
#include "data_processor.h"

boolean_t timer_on = False;
static time_t slice_time = 0;
static time_t total_time = 0;
// static timer_t timer_id;
// static struct sigevent signal_event;
static struct itimerspec timer_specs;
// static signal_id_t alarm_signal = SIGALRM;
static process_core_t cores_number = 0;
process_info_t *finished_processes = NULL;
static counter_t process_counter = 0;
static boolean_t simulator_works = True;
static boolean_t first_time = True;
extern boolean_t mlfq_is_on;
extern FILE *logFile;
static time_t last_total_time = 0;
extern SimulationResult *simulation_result_round_robin;

static time_slice_completion_e get_time_slice_completion_status(process_info_t *process_info);
static process_info_t schedule_next_process_from_current_core(process_core_t process_core);
static process_info_t schedule_next_process_from_other_cores(process_core_t process_core);
static process_info_t update_blocked_processes_round_robin(process_core_t process_core);
static process_info_t update_running_processes_round_robin(process_core_t process_core);
static void run_and_update_scheduled_process(process_info_t process_info);
static void continue_process_execution(process_info_t process_info);
static boolean_t end_processes(void);
static void distribute_processes(process_status_e process_status);
static void round_robin_handler(signal_id_t signal_id);

void init_round_robin_processes(process_core_t cores_count, counter_t processes_num)
{
    static boolean_t init = False;
    cores_number = cores_count;
    process_counter = processes_num;
    if (finished_processes == NULL)
    {
        finished_processes = (process_info_t *)calloc(processes_num, sizeof(process_info_t));
    }

    counter_t process_counter = 0;
    if (!init)
    {
        for (process_counter = 0; process_counter < processes_num; ++process_counter)
        {
            finished_processes[process_counter].priority = -1;
        }
        init = True;
    }
}
void init_round_robin_scheduler(time_t slice_time_in_sec)
{
    slice_time = slice_time_in_sec;
    timer_on = True;
    if (!simulator_works)
    {
        // signal(alarm_signal, round_robin_handler);

        // signal_event.sigev_notify = SIGEV_SIGNAL;
        // signal_event.sigev_signo = alarm_signal;
        // signal_event.sigev_value.sival_ptr = &timer_id;

        // if (timer_create(CLOCK_REALTIME, &signal_event, &timer_id) == TIMER_CREATION_FAILED)
        // {
        //     perror("TIMER CREATION ERROR: Failed to initiate timer\n");
        //     return;
        // }

        // timer_specs.it_value.tv_sec = 1;
        // timer_specs.it_interval.tv_sec = 1;
        // timer_specs.it_value.tv_nsec = 0;
        // timer_specs.it_interval.tv_nsec = 0;

        // if (timer_settime(timer_id, 0, &timer_specs, NULL) == TIMER_SPECS_SETTING_FAILED)
        // {
        //     perror("TIMER SETTING ERROR: Failed to set timer specifications\n");
        //     return;
        // }
    }
    else
    {
        round_robin_handler(0);
    }
}
/* void init_round_robin(time_t slice_time_in_sec, process_core_t cores_count, counter_t processes_num)
{
    cores_number = cores_count;
    process_counter = processes_num;
    slice_time = slice_time_in_sec;
    if (finished_processes != NULL)
    {
        free(finished_processes);
        finished_processes = NULL;
    }
    finished_processes = (process_info_t *)calloc(sizeof(process_info_t), (size_t)processes_num);
    counter_t process_counter = 0;
    for (process_counter = 0; process_counter < processes_num; ++process_counter)
    {
        finished_processes[process_counter].process_priority = -1;
    }
    iterator_t iterator;
    process_core_t current_core;
    signal(alarm_signal, round_robin_handler);
    signal_event.sigev_notify = SIGEV_SIGNAL;
    signal_event.sigev_signo = alarm_signal;
    signal_event.sigev_value.sival_ptr = &timer_id;

    if (timer_create(CLOCK_REALTIME, &signal_event, &timer_id) == TIMER_CREATION_FAILED)
    {
        perror("TIMER CREATION ERROR: Failed to initiate timer\n");
        return;
    }

    timer_specs.it_value.tv_sec = 1;
    timer_specs.it_interval.tv_sec = 1;
    timer_specs.it_value.tv_nsec = 0;
    timer_specs.it_interval.tv_nsec = 0;

    if (timer_settime(timer_id, 0, &timer_specs, NULL) == TIMER_SPECS_SETTING_FAILED)
    {
        perror("TIMER SETTING ERROR: Failed to set timer specifications\n");
        return;
    }
    timer_on = True;
} */
static time_slice_completion_e get_time_slice_completion_status(process_info_t *process_info)
{
    time_slice_completion_e time_slice_completion = TIME_SLICE_NOT_COMPLETE;
    if (process_info->time_consumed == slice_time)
    {
        // fprintf(log_file,log_file,"Time slice consumed\n");
        process_info->time_consumed = 0;
        if (process_info->process_status == RUN)
            process_info->time_slice_completion = TIME_SLICE_COMPLETE;
        finished_processes[process_info->pid - 'A'] = *process_info;
        time_slice_completion = TIME_SLICE_COMPLETE;
    }
    return time_slice_completion;
}
static process_info_t schedule_next_process_from_current_core(process_core_t process_core)
{
    process_info_t current_process = get_front(READY, process_core);
    if (current_process.pid == PROCESS_NOT_VALID)
    {
        // fprintf(log_file,"No ready process in the current core available now\n");
        return current_process;
    }
    if (current_process.arrival_time <= total_time)
    {
        // fprintf(log_file,"Ready process with arrival time %u at the current core\n",current_process.arrival_time);
        dequeue_process(READY, &current_process, process_core);
        finished_processes[current_process.pid - 'A'] = current_process;
        return current_process;
    }
    process_info_t tmp_process = current_process;
    dequeue_process(READY, &current_process, process_core);
    enqueue_process(current_process, process_core);
    finished_processes[current_process.pid - 'A'] = current_process;
    while (True)
    {
        dequeue_process(READY, &current_process, process_core);
        if (current_process.pid == tmp_process.pid)
        {
            // fprintf(log_file,"No ready process in the current core available now\n");
            enqueue_process(current_process, process_core);
            current_process.pid = PROCESS_NOT_VALID;
            return current_process;
        }
        if (current_process.arrival_time <= total_time)
        {
            // fprintf(log_file,"Ready process with arrival time %u at the current core\n",current_process.arrival_time);
            finished_processes[current_process.pid - 'A'] = current_process;
            return current_process;
        }
        enqueue_process(current_process, process_core);
    }
    return current_process;
}
static process_info_t schedule_next_process_from_other_cores(process_core_t process_core)
{
    process_info_t process_info;
    process_info.pid = PROCESS_NOT_VALID;
    process_core_t current_processor;
    for (current_processor = 0; current_processor < cores_number; ++current_processor)
    {
        if (process_core != current_processor)
        {
            process_info = schedule_next_process_from_current_core(current_processor);
            if (process_info.pid != PROCESS_NOT_VALID)
            {
                // fprintf(log_file,"Found another ready process at core %u with pid %d\n",current_processor,process_info.pid);
                break;
            }
            finished_processes[process_info.pid - 'A'] = process_info;
        }
    }
    process_info.current_core = process_core;
    finished_processes[process_info.pid - 'A'] = process_info;
    return process_info;
}
static process_info_t update_blocked_processes_round_robin(process_core_t process_core)
{
    process_info_t finished_process;
    finished_process.pid = PROCESS_NOT_VALID;
    process_info_t current_process;
    process_info_t tmp_process = get_front(BLOCKED, process_core);
    boolean_t flag = False;
    boolean_t done = False;
    while (True)
    {
        if (dequeue_process(BLOCKED, &current_process, process_core) == PROCESS_DEQUEUE_SUCCESSFUL)
        {
            if (flag == False)
            {
                flag = True;
                if (current_process.arrival_time > total_time)
                {
                    enqueue_process(current_process, process_core);
                    continue;
                }
            }
            else
            {
                if (tmp_process.pid == current_process.pid)
                {
                    enqueue_process(current_process, process_core);
                    break;
                }
                if (current_process.arrival_time > total_time)
                {
                    enqueue_process(current_process, process_core);
                    continue;
                }
            }
            //printf("pid : %c, next burst : %lu , total time : %lu , arrival time:%lu \n", current_process.pid, current_process.next_burst, total_time, current_process.arrival_time);
            bursts_counter_t burst_index = current_process.next_burst;
            bursts_counter_t burst_count = current_process.bursts_count;
            current_process.burst_info[burst_index].execution_time--;
            current_process.time_consumed++;
            finished_processes[current_process.pid - 'A'] = current_process;
            if (get_time_slice_completion_status(&current_process) == TIME_SLICE_COMPLETE)
            {

                if (current_process.burst_info[burst_index].execution_time == 0)
                {
                    // fprintf(log_file,"The CPU burst now finished\n");
                    current_process.next_burst++;
                    burst_index++;
                    finished_processes[current_process.pid - 'A'] = current_process;
                    if (current_process.next_burst == burst_count)
                    {
                        if (current_process.execution_time == 0)
                        {
                            current_process.turnaround_time = 0;
                        }
                        else
                        {
                            current_process.turnaround_time = total_time - current_process.arrival_time;
                        }
                        setProcessTurnaroundTime(simulation_result_round_robin, current_process.process_index, total_time);
                        finished_process = current_process;
                        finished_processes[current_process.pid - 'A'] = current_process;
                    }
                    else if (current_process.burst_info[current_process.next_burst].burst_type == IO_BURST)
                    {
                        // fprintf(log_file,"A new IO burst is ready to execute\n");
                        current_process.process_status = BLOCKED;
                        finished_processes[current_process.pid - 'A'] = current_process;
                        enqueue_process(current_process, current_process.current_core);
                    }
                    else
                    {
                        // fprintf(log_file,"A new CPU burst is ready to execute\n");
                        // current_process.time_consumed = 0;
                        current_process.process_status = READY;
                        finished_processes[current_process.pid - 'A'] = current_process;
                        enqueue_process(current_process, current_process.current_core);
                    }
                }
                else
                {
                    // current_process.time_consumed = 0;
                    current_process.process_status = BLOCKED;
                    finished_processes[current_process.pid - 'A'] = current_process;
                    enqueue_process(current_process, current_process.current_core);
                }
            }
            else
            {
                if (current_process.burst_info[burst_index].execution_time == 0)
                {
                    // fprintf(log_file,"The CPU burst now finished\n");
                    current_process.next_burst++;
                    finished_processes[current_process.pid - 'A'] = current_process;
                    burst_index++;

                    if (current_process.next_burst == burst_count)
                    {
                        if (current_process.execution_time == 0)
                        {
                            current_process.turnaround_time = 0;
                        }
                        else
                        {
                            current_process.turnaround_time = total_time - current_process.arrival_time;
                        }
                        setProcessTurnaroundTime(simulation_result_round_robin, current_process.process_index, total_time);

                        finished_processes[current_process.pid - 'A'] = current_process;
                        finished_process = current_process;
                    }
                    else if (current_process.burst_info[current_process.next_burst].burst_type == IO_BURST)
                    {
                        // fprintf(log_file,"A new IO burst is ready to execute\n");
                        current_process.process_status = BLOCKED;
                        finished_processes[current_process.pid - 'A'] = current_process;
                        enqueue_process(current_process, current_process.current_core);
                    }
                    else
                    {
                        // fprintf(log_file,"A new CPU burst is ready to execute\n");
                        // current_process.time_consumed = 0;
                        current_process.process_status = READY;
                        current_process.time_consumed = 0;
                        finished_processes[current_process.pid - 'A'] = current_process;
                        enqueue_process(current_process, current_process.current_core);
                    }
                }
                else
                {
                    // current_process.time_consumed = 0;
                    current_process.process_status = BLOCKED;
                    finished_processes[current_process.pid - 'A'] = current_process;
                    enqueue_process(current_process, current_process.current_core);
                }
            }
        }
        else
        {
            break;
        }
    }

    return finished_process;
}
static process_info_t update_running_processes_round_robin(process_core_t process_core)
{
    process_info_t finished_process;
    finished_process.pid = PROCESS_NOT_VALID;
    process_info_t current_process;
    if (dequeue_process(RUN, &current_process, process_core) == PROCESS_DEQUEUE_SUCCESSFUL)
    {
        // fprintf(log_file,"There is a running process\n");
        bursts_counter_t burst_index = current_process.next_burst;
        bursts_counter_t burst_count = current_process.bursts_count;
        /*if (burst_index == burst_count)
            return finished_process;*/
        // printf("%u\n", burst_count);
        current_process.burst_info[burst_index].execution_time--;
        current_process.time_consumed++;
        finished_processes[current_process.pid - 'A'] = current_process;
        if (get_time_slice_completion_status(&current_process) == TIME_SLICE_COMPLETE)
        {
            if (current_process.burst_info[burst_index].execution_time == 0)
            {
                // fprintf(log_file,"The CPU burst now finished\n");
                current_process.next_burst++;
                burst_index++;
                //printf("I reached here with burst number %lu and burst count %lu !\n", current_process.next_burst, current_process.bursts_count);
                finished_processes[current_process.pid - 'A'] = current_process;
                if (current_process.next_burst == burst_count)
                {
                    if (current_process.execution_time == 0)
                    {
                        current_process.turnaround_time = 0;
                    }
                    else
                    {
                        current_process.turnaround_time = total_time - current_process.arrival_time;
                    }
                    setProcessTurnaroundTime(simulation_result_round_robin, current_process.process_index, total_time);
                    finished_processes[current_process.pid - 'A'] = current_process;
                    finished_process = current_process;
                }
                else if (current_process.burst_info[current_process.next_burst].burst_type == IO_BURST)
                {
                    // fprintf(log_file,"A new IO burst is ready to execute\n");
                    current_process.process_status = BLOCKED;
                    finished_processes[current_process.pid - 'A'] = current_process;
                    enqueue_process(current_process, current_process.current_core);
                }
                else
                {
                    // fprintf(log_file,"A new CPU burst is ready to execute\n");
                    // current_process.time_consumed = 0;
                    current_process.process_status = READY;
                    finished_processes[current_process.pid - 'A'] = current_process;
                    enqueue_process(current_process, current_process.current_core);
                }
            }
            else
            {
                // current_process.time_consumed = 0;
                current_process.process_status = READY;
                finished_processes[current_process.pid - 'A'] = current_process;
                enqueue_process(current_process, current_process.current_core);
            }
        }
        else
        {
            if (current_process.burst_info[burst_index].execution_time == 0)
            {
                // fprintf(log_file,"The CPU burst now finished\n");
                current_process.next_burst++;
                burst_index++;
                finished_processes[current_process.pid - 'A'] = current_process;
                if (current_process.next_burst == burst_count)
                {
                    if (current_process.execution_time == 0)
                    {
                        current_process.turnaround_time = 0;
                    }
                    else
                    {
                        current_process.turnaround_time = total_time - current_process.arrival_time;
                    }
                    finished_processes[current_process.pid - 'A'] = current_process;
                    setProcessTurnaroundTime(simulation_result_round_robin, current_process.process_index, total_time);

                    finished_process = current_process;
                }
                else if (current_process.burst_info[current_process.next_burst].burst_type == IO_BURST)
                {
                    // fprintf(log_file,"A new IO burst is ready to execute\n");
                    current_process.process_status = BLOCKED;
                    current_process.time_consumed = 0;
                    finished_processes[current_process.pid - 'A'] = current_process;
                    enqueue_process(current_process, current_process.current_core);
                }
                else
                {
                    // fprintf(log_file,"A new CPU burst is ready to execute\n");
                    // current_process.time_consumed = 0;
                    current_process.process_status = RUN;
                    finished_processes[current_process.pid - 'A'] = current_process;
                    enqueue_process(current_process, current_process.current_core);
                }
            }
            else
            {
                // current_process.time_consumed = 0;
                current_process.process_status = RUN;
                finished_processes[current_process.pid - 'A'] = current_process;
                enqueue_process(current_process, current_process.current_core);
            }
        }
    }
    return finished_process;
}
void static run_and_update_scheduled_process(process_info_t process_info)
{
    if (process_info.response_time == -1)
    {
        process_info.response_time = total_time - process_info.arrival_time;
        setProcessResponseTime(simulation_result_round_robin, process_info.process_index, (int)total_time);
        //printf("yes entred with index %ld\n",total_time);
    }
    process_info.process_status = RUN;
    finished_processes[process_info.pid - 'A'] = process_info;
    // fprintf(log_file,"The process runs successfully\n");
    enqueue_process(process_info, process_info.current_core);
}
void static continue_process_execution(process_info_t process_info)
{
    // fprintf(log_file,"Completing executing pid %d\n",process_info.pid);
    // process_info.time_consumed ++ ;
    enqueue_process(process_info, process_info.current_core);
}
static boolean_t end_processes(void)
{

    process_core_t current_core;

    for (current_core = 0; current_core < cores_number; ++current_core)
    {
        if (check_queue_empty(RUN, current_core) == QUEUE_NOT_EMPTY || check_queue_empty(READY, current_core) == QUEUE_NOT_EMPTY || check_queue_empty(BLOCKED, current_core) == QUEUE_NOT_EMPTY)
        {
            return False;
        }
    }
    return True;
}
void send_data_round_robin(process_info_t *arr, counter_t processes_count)
{
    counter_t counter = 0;
    for (; counter < processes_count; ++counter)
    {
        enqueue_round_robin_process(&arr[counter]);
        finished_processes[arr[counter].pid - 'A'] = arr[counter];
    }
}
static void distribute_processes(process_status_e process_status)
{
    // iterate over every process, check if its original is available. if not add it to another empty ready
    if (cores_number == 1)
        return;
    process_core_t core_from, core_to;
    for (core_from = 0; core_from < cores_number; ++core_from)
    {
        process_info_t current_process;
        process_dequeue_status_e process_dequeue_status = dequeue_process(process_status, &current_process, core_from);
        if (process_dequeue_status == PROCESS_DEQUEUE_SUCCESSFUL)
        {
            process_info_t to_return = current_process;
            core_to = core_from;
            while (check_queue_empty(process_status, core_from) != QUEUE_EMPTY)
            {
                core_to = (core_to + 1) % cores_number;
                dequeue_process(process_status, &current_process, core_from);
                current_process.current_core = core_to;
                finished_processes[current_process.pid - 'A'] = current_process;
                enqueue_process(current_process, core_to);
            }
            enqueue_process(to_return, core_from);
        }
    }
    /*
    a,b,c ready in 0
    1 is empty, 0 not empty, a in 1
                1 not empty, a in 1
                2 empty
    2 is empty, 0 not empty, b in 2
                1 not empty, a in 2
                2 not empty, a,b in 2
    */
}
void static round_robin_handler(signal_id_t signal_id)
{
    // distribute_processes(BLOCKED);
    while (True)
    {
        if (signal_id == 0)
        {
            process_core_t current_core;
            // distribute_processes(BLOCKED);
            // distribute_processes(READY);
            if (!first_time)
            {
                for (current_core = 0; current_core < cores_number; ++current_core)
                {
                    process_info_t finished_process = update_blocked_processes_round_robin(current_core);
                    if (finished_process.pid != PROCESS_NOT_VALID)
                    {
                        finished_processes[finished_process.pid - 'A'] = finished_process;
                        fprintf(logFile, "process with pid %c finished \n", finished_process.pid);
                    }
                    else
                    {
                        fprintf(logFile, "No process finished from blocked\n");
                    }
                    finished_process = update_running_processes_round_robin(current_core);
                    if (finished_process.pid != PROCESS_NOT_VALID)
                    {
                        finished_processes[finished_process.pid - 'A'] = finished_process;
                        fprintf(logFile, "process with pid %c finished\n", finished_process.pid);
                    }
                    else
                    {
                        fprintf(logFile, "No process finished from running\n");
                    }
                }
            }
            if ((((total_time - last_total_time) == slice_time) && mlfq_is_on) || end_processes())
            {
                stop_round_robin();
                last_total_time = total_time;
                mlfq_is_on = False;
                first_time = True;
                return;
            }
            for (current_core = 0; current_core < cores_number; ++current_core)
            {
                process_info_t current_process;
                current_process.pid = 'i';
                fprintf(logFile, "Got process from Run with pid %c\n", current_process.pid);
                if (check_queue_empty(RUN, current_core) == QUEUE_EMPTY)
                {
                    fprintf(logFile, "The process completed its time slice!\n");
                    current_process = schedule_next_process_from_current_core(current_core);
                    if (current_process.pid == PROCESS_NOT_VALID)
                    {
                        fprintf(logFile, "No process to schedule in the current core\n");
                        current_process = schedule_next_process_from_other_cores(current_core);
                        if (current_process.pid == PROCESS_NOT_VALID)
                        {
                            fprintf(logFile, "No process to schedule from other cores\n");
                            current_process.pid = 'i';
                        }
                        else
                        {
                            run_and_update_scheduled_process(current_process);
                            fprintf(logFile, "Process with pid %c run succefully burst number %lu\n", current_process.pid, current_process.next_burst);
                        }
                    }
                    else
                    {
                        run_and_update_scheduled_process(current_process);
                        fprintf(logFile, "Process with pid %c run succefully burst number %lu\n", current_process.pid, current_process.next_burst);
                    }
                }
                else
                {
                    dequeue_process(RUN, &current_process, current_core);
                    continue_process_execution(current_process);
                    fprintf(logFile, "continue execution of pid %c\n", current_process.pid);
                }
                scheduleProcess(simulation_result_round_robin, current_core, current_process.pid, total_time, total_time + 1);
                fprintf(logFile, "core: %d process: %c\n", current_core, current_process.pid);
                fprintf(logFile, "%s", "*****************************************************************\n");
            }
            counter_t counter;
            for (counter = 0; counter < process_counter; ++ counter){
                setProcessStatus(simulation_result_round_robin,finished_processes[counter].process_index,1,finished_processes[counter].process_status);
            }
            first_time = False;
            total_time++;
            //printf("total_time : %ld\n", total_time);
            fprintf(logFile, "%s", "##################################################################\n\n\n");
            if (!simulator_works)
                break;
        }
    }
}
void run_round_robin_simulator(void)
{
    simulator_works = True;
}
void run_round_robin_scheduler(void)
{
    simulator_works = False;
}
process_enqueue_status_e enqueue_round_robin_process(process_info_t *process_info)
{
    process_enqueue_status_e process_enqueue_status;
    if (process_info->burst_info[process_info->next_burst].burst_type == CPU_BURST)
    {
        process_info->process_status = READY;
    }
    else
    {
        process_info->process_status = BLOCKED;
    }
    //printf("%c\n", (process_info->pid));
    finished_processes[(process_info->pid) - 'A'] = *process_info;
    process_enqueue_status = enqueue_process(*process_info, process_info->current_core);
    return process_enqueue_status;
}
void stop_round_robin(void)
{
    // timer_delete(timer_id);
    timer_on = False;
    // fprintf(log_file,"Round robin stopped!\n");
}
