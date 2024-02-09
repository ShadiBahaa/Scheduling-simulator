#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include "data_processor.h"
#include "pqueue.h"
#include "utilities.h"
#include "round_robin.h"
#include "round_robin_sem.h"

extern process_info_t * finished_processes;
SimulationResult * simulation_result_round_robin;

void transfer_processes(ProcessedValues *values, process_info_t *arr, counter_t pro_count)
{
    counter_t counter;
    for (counter = 0; counter < pro_count; ++counter)
    {
        arr[counter].arrival_time = values->processes[counter].arrivalTime;
        arr[counter].pid = values->processes[counter].id;
        arr[counter].execution_time = values->processes[counter].totalDuration;
        bursts_counter_t burst_count = values->processes[counter].burstsCount;
        arr[counter].bursts_count = burst_count;
        arr[counter].burst_info = malloc(sizeof(burst_info_t) * burst_count);
        bursts_counter_t burst_counter;
        for (burst_counter = 0; burst_counter < burst_count ; burst_counter++)
        {
            arr[counter].burst_info[burst_counter].burst_type = (values->processes[counter].bursts[burst_counter].type[0] == 'C' || values->processes[counter].bursts[burst_counter].type[0] == 'c') ? CPU_BURST : IO_BURST;
            arr[counter].burst_info[burst_counter].execution_time = values->processes[counter].bursts[burst_counter].duration;
            if (arr[counter].burst_info[burst_counter].burst_type == IO_BURST)
            {
                arr[counter].execution_time -= arr[counter].burst_info[burst_counter].execution_time;
            }
        }
        arr[counter].next_burst = 0;
        arr[counter].time_consumed = 0;
        arr[counter].original_core = 0;
        arr[counter].current_core = 0;
        arr[counter].response_time = -1;
        arr[counter].turnaround_time = -1;
        arr[counter].time_slice_completion = TIME_SLICE_NOT_COMPLETE;
        arr[counter].priority = -1;
        arr[counter].process_status = READY;
        arr[counter].process_index = counter;
    }
}

void simulateRoundRobin(ProcessedValues *values, SimulationResult *result, FILE *logFile)
{
    process_core_t cores_count = values->numCores;
    simulation_result_round_robin = result;
    process_info_t *arr = calloc(values->processesCount, sizeof(process_info_t));
    transfer_processes(values, arr, values->processesCount);
    run_round_robin_simulator();
    init_processes_queues(cores_count);
    init_round_robin_processes(cores_count,26);
    send_data_round_robin(arr,values->processesCount);
    init_round_robin_scheduler(1);
    counter_t process_counter;
    double avg_turnaround = 0, avg_response = 0;

    for (process_counter = 0 ; process_counter < values->processesCount; ++process_counter){
        fprintf(logFile, "pid: %c , response time: %lu , turnaround time: %lu\n", finished_processes[process_counter].pid, finished_processes[process_counter].response_time, finished_processes[process_counter].turnaround_time);
        avg_turnaround += finished_processes[process_counter].turnaround_time;
        avg_response += finished_processes[process_counter].response_time;

    }
    avg_turnaround /= values->processesCount;
    avg_response /= values->processesCount;
    setAvgResponseTime(simulation_result_round_robin,avg_response);
    setAvgTurnaroundTime(simulation_result_round_robin,avg_turnaround);
    fprintf(logFile,"Average turnaround : %lf \n",avg_turnaround);
    fprintf(logFile,"Average response : %lf \n",avg_response);
    //printf("Start of results\n");
    result = simulation_result_round_robin;
    logSimulationResult(logFile,result);
}