#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include "pqueue.h"
#include "utilities.h"
#include "round_robin.h"
#include "data_processor.h"
#include "round_robin_sem.h"
#include "mlfq.h"
#include "mlfq_sem.h"

extern process_info_t *finished_processes;
extern SimulationResult *simulation_result_round_robin;


void simulateMLFQ(ProcessedValues *values, SimulationResult *result, FILE *logFile)
{
    process_core_t cores_count = values->numCores;
    simulation_result_round_robin = result;
    process_info_t *arr = calloc(values->processesCount, sizeof(process_info_t));
    run_mlfq_simulator();
    transfer_processes(values, arr, values->processesCount);
    send_mlfq_data(arr,values->processesCount);
    time_t * time_slices = malloc(sizeof(time_t) * 15);
    time_t iterator;
    for (iterator = 0; iterator < 15; ++iterator){
        time_slices[iterator] = iterator;
    }
    init_mlfq(cores_count,time_slices,15,3);
    run_mlfq();
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
