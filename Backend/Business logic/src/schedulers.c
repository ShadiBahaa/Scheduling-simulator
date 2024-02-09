#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "data_processor.h"
#include "pqueue.h"
#include "schedulers.h"

void simulateSTCF1(ProcessedValues *values, SimulationResult *result, FILE *logFile)
{
    fprintf(logFile, "///////////////////////////// start of STCF1 Simulation //////////////////////////\n"); // debug
    int numCores = values->numCores;
    // ready queue of the extra set of queues is used to store the processes in the order they arrived
    // blocked queue of the extra set of queues is used to store the processes that are waiting for IO
    // run queue of the extra set of queues is not used
    // Only using the run and ready queue of the cores. The blocked queue is not used
    init_processes_queues(numCores + 1);
    int coreIndex = 0;
    int currentTime = 0;
    fprintf(logFile, "numCores %d\n", numCores); // debug

    // Iterate over each process in the order they arrived and enqueue them in the processes queue
    for (int i = 0; i < values->processesCount; i++)
    {
        char processId = values->processes[i].id;
        int processIndex = getProcessIndex(result, processId);
        process_info_t *pinfo = fill_process_info(values, processIndex);
        pinfo->process_status = READY;
        enqueue_process(*pinfo, numCores);
    }
    fprintf(logFile, "processes enqueed successfully \n"); // debug
    queue_capacity_status_e status = check_queue_empty(READY, numCores);
    fprintf(logFile, "status %d\n\n", status); // debug

    process_info_t *currProcess = (process_info_t *)malloc(sizeof(process_info_t));
    while (1)
    {
        fprintf(logFile, "start of iteration\n");           // debug
        fprintf(logFile, "current time %d\n", currentTime); // debug
        // get number of busy cores
        int busyCores = 0;
        for (int i = 0; i < numCores; i++)
        {
            if (check_queue_empty(RUN, i) == QUEUE_NOT_EMPTY)
            {
                fprintf(logFile, "core %d is busy\n", i); // debug
                busyCores++;
            }
            else
            {
                if (check_queue_empty(BLOCKED, i) == QUEUE_NOT_EMPTY)
                {
                    fprintf(logFile, "core %d is not busy but has blocked processes\n", i); // debug
                    busyCores++;
                }
                else if (check_queue_empty(READY, i) == QUEUE_NOT_EMPTY)
                {
                    fprintf(logFile, "core %d is not busy but has ready processes\n", i); // debug
                    busyCores++;
                }
                else
                {
                    fprintf(logFile, "core %d is not busy\n", i); // debug
                }
            }
        }

        // if all are not busy and processes queue is empty, break
        if (busyCores == 0 && (check_queue_empty(READY, numCores) == QUEUE_EMPTY) && (check_queue_empty(BLOCKED, numCores) == QUEUE_EMPTY) && (check_queue_empty(RUN, numCores) == QUEUE_EMPTY))
        {
            fprintf(logFile, "all cores are not busy and processes queue and blocked queue is empty\n"); // debug
            break;
        }

        fprintf(logFile, "Either cores hasnt finished, or processes or blocked queue is not empty\n"); // debug

        // iterate over all cores and run the process in each core for 1 time unit
        for (int i = 0; i < numCores; i++)
        {
            fprintf(logFile, "core %d start of checking\n", i); // debug
            // check if the run queue of the core is empty
            if (check_queue_empty(RUN, i) == QUEUE_EMPTY)
            {
                fprintf(logFile, "run queue of core %d is empty\n", i); // debug

                fprintf(logFile, "checking if a new process has arrived\n"); // debug
                // if a core is available, peek the first process in the processes queue and enqueue it in the ready queue of the core if its arrival time is less than or equal to the current time
                if (peek_process(READY, &currProcess, numCores) == PROCESS_DEQUEUE_FAILED)
                {
                    fprintf(logFile, "empty processes quee\n"); // debug
                }
                else
                {
                    fprintf(logFile, "currProcess to assign to a core: %c\n", currProcess->pid); // debug
                    if (currProcess->arrival_time > currentTime)
                    {
                        fprintf(logFile, "process %c has not arrived yet\n", currProcess->pid);   // debug
                        fprintf(logFile, "process arrival time %d\n", currProcess->arrival_time); // debug
                    }
                    else
                    {
                        // find the core with the least ready queue size
                        int leastCore = 9999, leastCoreIndex = -1;
                        for (int j = 0; j < numCores; j++)
                        {
                            int readyQueueSize = get_queue_size(READY, j);
                            if (readyQueueSize < leastCore)
                            {
                                leastCore = readyQueueSize;
                                leastCoreIndex = j;
                            }
                            fprintf(logFile, "core %d ready queue size %d\n", j, readyQueueSize); // debug
                        }
                        fprintf(logFile, "process %c moved to ready at time %d\n", currProcess->pid, currentTime); // debug
                        // dequeue process from processes queue and enqueue it in the ready queue of the first available core
                        coreIndex = leastCoreIndex;
                        
                        if (dequeue_process(READY, currProcess, numCores) == PROCESS_DEQUEUE_FAILED)
                        {
                            fprintf(logFile, "dequeue failed\n"); // debug
                            continue;
                        }
                        currProcess->process_status = READY;
                        enqueue_process(*currProcess, coreIndex);
                        fprintf(logFile, "process %c enqueued successfully to ready queue on core %d\n", currProcess->pid, coreIndex); // debug
                        i--;
                        continue;
                    }
                }

                // Return the process to its original core from the general run queue

                if (peek_process(RUN, &currProcess, numCores) == PROCESS_DEQUEUE_FAILED)
                {
                    fprintf(logFile, "empty general running quee\n"); // debug
                }
                else
                {
                    fprintf(logFile, "currProcess to return to the core: %c\n", currProcess->pid); // debug
                    if (currProcess->current_core == i)
                    {
                        fprintf(logFile, "process %c moved back to ready of the core at time %d\n", currProcess->pid, currentTime); // debug
                        // dequeue process from processes queue and enqueue it in the ready queue of the first available core
                        coreIndex = i;
                        
                        if (dequeue_process(RUN, currProcess, numCores) == PROCESS_DEQUEUE_FAILED)
                        {
                            fprintf(logFile, "dequeue failed\n"); // debug
                            continue;
                        }
                        currProcess->process_status = READY;
                        enqueue_process_front(*currProcess, coreIndex);
                        fprintf(logFile, "process %c enqueued successfully to ready queue on core %d\n", currProcess->pid, coreIndex); // debug
                        i--;
                        continue;
                    }
                    else
                    {
                        fprintf(logFile, "process %c is not on the core %d\n", currProcess->pid, i); // debug
                    }
                }

                if (check_queue_empty(READY, i) == QUEUE_NOT_EMPTY)
                {
                    fprintf(logFile, "ready queue of core %d is not empty\n", i); // debug
                    // dequeue the process with the least current burst duration from the ready queue of the core
                    dequeue_process(READY, currProcess, i);
                    if (currProcess->burst_info[currProcess->next_burst].burst_type != 'I')
                    {
                        enqueue_process_front(*currProcess, i);
                        int leastBurstTime = get_least_current_burst_time_process(READY, i);
                        if (get_process_with_property(READY, currProcess, i, current_burst_time_match, &leastBurstTime) == PROCESS_DEQUEUE_FAILED)
                        {
                            fprintf(logFile, "dequeue failed from ready of core %d\n", i); // debug
                            continue;
                        }
                    }

                    // check if the process is comming from blocked queue and has finished execution
                    if (currProcess->waiting_time == 9999)
                    {
                        fprintf(logFile, "process %c came from IO and finished\n", currProcess->pid); // debug
                        setProcessTurnaroundTime(result, getProcessIndex(result, currProcess->pid), currProcess->execution_time);
                        i--;
                        continue;
                    }
                    // check if the process is comming from blocked queue and didnt finish yet
                    if (currProcess->waiting_time > 0)
                    {
                        fprintf(logFile, "process %c was ready since %d\n", currProcess->pid, currProcess->waiting_time); // debug
                        setProcessStatus(result, getProcessIndex(result, currProcess->pid), currentTime - currProcess->waiting_time, READY);
                        currProcess->waiting_time = 0;
                    }
                    currProcess->process_status = RUN;
                    enqueue_process(*currProcess, i);

                    // check if the proecces runs for the first time and set its first run time
                    int processIndex = getProcessIndex(result, currProcess->pid);
                    if (result->processesAnalysis[processIndex].firstRunTime == -1)
                    {
                        result->processesAnalysis[processIndex].firstRunTime = currentTime;
                        setProcessResponseTime(result, processIndex, currentTime);
                        setProcessStatus(result, processIndex, result->processesAnalysis[processIndex].firstRunTime - result->processesAnalysis[processIndex].arrivalTime, READY);
                        fprintf(logFile, "process %c is running for the first time on core %d\n", currProcess->pid, i); // debug
                    }
                    fprintf(logFile, "process %c enqueued successfully to run on core %d\n", currProcess->pid, coreIndex); // debug
                    i--;
                    continue;
                }
                // if the core is not busy and the processes queue is empty or first process has not arrived yet, search for a process in the ready queue of other cores
                fprintf(logFile, "searching for a process in the ready queue of other cores\n"); // debug
                int found = 0;
                // find the least current burst time process in the ready queue of other cores
                int leastCore = 9999, leastBurstTime = 9999, leastCoreIndex = -1;
                for (int j = 0; j < numCores; j++)
                {
                    if (j == i)
                    {
                        continue;
                    }
                    if (check_queue_empty(READY, j) == QUEUE_NOT_EMPTY)
                    {
                        fprintf(logFile, "ready queue of core %d is not empty\n", j); // debug
                        leastBurstTime = get_least_current_burst_time_process(READY, j);
                        if (leastBurstTime < leastCore)
                        {
                            leastCore = leastBurstTime;
                            leastCoreIndex = j;
                            found = 1;
                        }
                    }
                }
                
                if (found == 1)
                {
                    fprintf(logFile, "found a process in the ready queue of core %d\n", leastCoreIndex); // debug
                    if (get_process_with_property(READY, currProcess, leastCoreIndex, current_burst_time_match, &leastBurstTime) == PROCESS_DEQUEUE_FAILED)
                    {
                        fprintf(logFile, "dequeue failed from ready of core %d\n", leastCoreIndex); // debug
                        continue;
                    }
                    currProcess->process_status = READY;
                    enqueue_process(*currProcess, i);
                    fprintf(logFile, "process %c enqueued successfully to run on core %d\n", currProcess->pid, i); // debug
                    i--;
                    continue;
                }
                else
                {
                    // if no process is found, schedule an idle process
                    fprintf(logFile, "no processes to run on core %d\n", i); // debug
                    scheduleProcess(result, i, 'i', currentTime, currentTime + 1);
                }
            }
            else
            {
                fprintf(logFile, "core %d start of running\n", i); // debug

                peek_process(RUN, &currProcess, i);
                char processId = currProcess->pid;
                int processIndex = getProcessIndex(result, processId);
                ProcessInfo *process = &(values->processes[processIndex]);

                BurstInfo *burst = &(process->bursts[currProcess->next_burst]);

                if (strcmp(burst->type, "CPU") == 0)
                {
                    setProcessStatus(result, processIndex, 1, RUN);
                    scheduleProcess(result, i, processId, currentTime, currentTime + 1);
                    fprintf(logFile, "process %c is running on core %d cpu\n", processId, i); // debug
                    burst->duration--;
                    currProcess->burst_info[currProcess->next_burst].execution_time--;
                }
                else
                {
                    fprintf(logFile, "process %c made IO on core %d\n", processId, i); // debug
                    setProcessStatus(result, processIndex, burst->duration, BLOCKED);
                    currProcess->next_burst++;
                    process->burstsCount--;
                    dequeue_process(RUN, currProcess, i);
                    currProcess->original_core = i;
                    currProcess->current_core = -1;
                    currProcess->process_status = BLOCKED;
                    currProcess->waiting_time = burst->duration;
                    enqueue_process(*currProcess, numCores);
                    i--;
                    continue;
                }

                if (burst->duration == 0)
                {
                    fprintf(logFile, "process %c burst completed\n", processId); // debug
                    currProcess->next_burst++;
                    process->burstsCount--;
                    if (process->burstsCount == 0)
                    {
                        fprintf(logFile, "process %c completed\n", processId); // debug
                        setProcessTurnaroundTime(result, processIndex, currentTime + 1);
                        dequeue_process(RUN, currProcess, i);
                    }
                    else
                    {
                        dequeue_process(RUN, currProcess, i);
                        currProcess->process_status = RUN;
                        currProcess->current_core = i;
                        currProcess->waiting_time = currentTime + 1;
                        enqueue_process(*currProcess, numCores);
                    }
                }
                else
                {
                    dequeue_process(RUN, currProcess, i);
                    currProcess->process_status = RUN;
                    currProcess->current_core = i;
                    currProcess->waiting_time = currentTime + 1;
                    enqueue_process(*currProcess, numCores);
                }
                fprintf(logFile, "core %d end of running\n", i); // debug
            }
            fprintf(logFile, "core %d end of checking\n", i); // debug
        }
        currentTime++;
        update_waiting_time(currentTime, numCores);
        fprintf(logFile, "end of iteration\n\n"); // debug
    }
    free(currProcess);
    currProcess = NULL;

    // Calculate and set average turnaround time and average response time
    double totalTurnaroundTime = 0, totalResponseTime = 0;
    for (int i = 0; i < result->processesCount; i++)
    {
        totalTurnaroundTime += result->processesAnalysis[i].turnaroundTime;
        totalResponseTime += result->processesAnalysis[i].responseTime;
    }
    setAvgTurnaroundTime(result, totalTurnaroundTime / result->processesCount);
    setAvgResponseTime(result, totalResponseTime / result->processesCount);
    fprintf(logFile, "///////////////////////////// end of STCF1 Simulation //////////////////////////\n"); // debug
}

void simulateSJF1(ProcessedValues *values, SimulationResult *result, FILE *logFile)
{
    fprintf(logFile, "///////////////////////////// start of SJF1 Simulation //////////////////////////\n"); // debug
    int numCores = values->numCores;
    // ready queue of the extra set of queues is used to store the processes in the order they arrived
    // blocked queue of the extra set of queues is used to store the processes that are waiting for IO
    // run queue of the extra set of queues is not used
    // Only using the run and ready queue of the cores. The blocked queue is not used
    init_processes_queues(numCores + 1);
    int coreIndex = 0;
    int currentTime = 0;
    fprintf(logFile, "numCores %d\n", numCores); // debug

    // Iterate over each process in the order they arrived and enqueue them in the processes queue
    for (int i = 0; i < values->processesCount; i++)
    {
        char processId = values->processes[i].id;
        int processIndex = getProcessIndex(result, processId);
        process_info_t *pinfo = fill_process_info(values, processIndex);
        pinfo->process_status = READY;
        enqueue_process(*pinfo, numCores);
    }
    fprintf(logFile, "processes enqueed successfully \n"); // debug
    queue_capacity_status_e status = check_queue_empty(READY, numCores);
    fprintf(logFile, "status %d\n\n", status); // debug

    process_info_t *currProcess = (process_info_t *)malloc(sizeof(process_info_t));
    while (1)
    {
        fprintf(logFile, "start of iteration\n");           // debug
        fprintf(logFile, "current time %d\n", currentTime); // debug
        // get number of busy cores
        int busyCores = 0;
        for (int i = 0; i < numCores; i++)
        {
            if (check_queue_empty(RUN, i) == QUEUE_NOT_EMPTY)
            {
                fprintf(logFile, "core %d is busy\n", i); // debug
                busyCores++;
            }
            else
            {
                if (check_queue_empty(BLOCKED, i) == QUEUE_NOT_EMPTY)
                {
                    fprintf(logFile, "core %d is not busy but has blocked processes\n", i); // debug
                    busyCores++;
                }
                else if (check_queue_empty(READY, i) == QUEUE_NOT_EMPTY)
                {
                    fprintf(logFile, "core %d is not busy but has ready processes\n", i); // debug
                    busyCores++;
                }
                else
                {
                    fprintf(logFile, "core %d is not busy\n", i); // debug
                }
            }
        }

        // if all are not busy and processes queue is empty, break
        if (busyCores == 0 && (check_queue_empty(READY, numCores) == QUEUE_EMPTY) && (check_queue_empty(BLOCKED, numCores) == QUEUE_EMPTY))
        {
            fprintf(logFile, "all cores are not busy and processes queue and blocked queue is empty\n"); // debug
            break;
        }

        fprintf(logFile, "Either cores hasnt finished, or processes or blocked queue is not empty\n"); // debug

        // iterate over all cores and run the process in each core for 1 time unit
        for (int i = 0; i < numCores; i++)
        {
            fprintf(logFile, "core %d start of checking\n", i); // debug
            // check if the run queue of the core is empty
            if (check_queue_empty(RUN, i) == QUEUE_EMPTY)
            {
                fprintf(logFile, "run queue of core %d is empty\n", i); // debug

                fprintf(logFile, "checking if a new process has arrived\n"); // debug
                // if a core is available, peek the first process in the processes queue and enqueue it in the ready queue of the core if its arrival time is less than or equal to the current time
                if (peek_process(READY, &currProcess, numCores) == PROCESS_DEQUEUE_FAILED)
                {
                    fprintf(logFile, "empty processes quee\n"); // debug
                }
                else
                {
                    fprintf(logFile, "currProcess to assign to a core: %c\n", currProcess->pid); // debug
                    if (currProcess->arrival_time > currentTime)
                    {
                        fprintf(logFile, "process %c has not arrived yet\n", currProcess->pid);   // debug
                        fprintf(logFile, "process arrival time %d\n", currProcess->arrival_time); // debug
                    }
                    else
                    {
                        // find the core with the least ready queue size
                        int leastCore = 9999, leastCoreIndex = -1;
                        for (int j = 0; j < numCores; j++)
                        {
                            int readyQueueSize = get_queue_size(READY, j);
                            if (readyQueueSize < leastCore)
                            {
                                leastCore = readyQueueSize;
                                leastCoreIndex = j;
                            }
                            fprintf(logFile, "core %d ready queue size %d\n", j, readyQueueSize); // debug
                        }
                        fprintf(logFile, "process %c moved to ready at time %d\n", currProcess->pid, currentTime); // debug
                        // dequeue process from processes queue and enqueue it in the ready queue of the first available core
                        coreIndex = leastCoreIndex;

                        if (dequeue_process(READY, currProcess, numCores) == PROCESS_DEQUEUE_FAILED)
                        {
                            fprintf(logFile, "dequeue failed\n"); // debug
                            continue;
                        }
                        currProcess->process_status = READY;
                        enqueue_process(*currProcess, coreIndex);
                        fprintf(logFile, "process %c enqueued successfully to ready queue on core %d\n", currProcess->pid, coreIndex); // debug
                        i--;
                        continue;
                    }
                }

                if (check_queue_empty(READY, i) == QUEUE_NOT_EMPTY)
                {
                    fprintf(logFile, "ready queue of core %d is not empty\n", i); // debug
                    // dequeue the process with the least current burst duration from the ready queue of the core
                    int leastBurstTime = get_least_current_burst_time_process(READY, i);
                    if (get_process_with_property(READY, currProcess, i, current_burst_time_match, &leastBurstTime) == PROCESS_DEQUEUE_FAILED)
                    {
                        fprintf(logFile, "dequeue failed from ready of core %d\n", i); // debug
                        continue;
                    }

                    // check if the process is comming from blocked queue and has finished execution
                    if (currProcess->waiting_time == 9999)
                    {
                        fprintf(logFile, "process %c came from IO and finished\n", currProcess->pid); // debug
                        setProcessTurnaroundTime(result, getProcessIndex(result, currProcess->pid), currProcess->execution_time);
                        i--;
                        continue;
                    }
                    // check if the process is comming from blocked queue and didnt finish yet
                    if (currProcess->waiting_time > 0)
                    {
                        fprintf(logFile, "process %c came for IO and it was ready since %d\n", currProcess->pid, currProcess->waiting_time); // debug
                        setProcessStatus(result, getProcessIndex(result, currProcess->pid), currentTime - currProcess->waiting_time, READY);
                        currProcess->waiting_time = 0;
                    }
                    currProcess->process_status = RUN;
                    enqueue_process(*currProcess, i);

                    // check if the proecces runs for the first time and set its first run time
                    int processIndex = getProcessIndex(result, currProcess->pid);
                    if (result->processesAnalysis[processIndex].firstRunTime == -1)
                    {
                        result->processesAnalysis[processIndex].firstRunTime = currentTime;
                        setProcessResponseTime(result, processIndex, currentTime);
                        setProcessStatus(result, processIndex, result->processesAnalysis[processIndex].firstRunTime - result->processesAnalysis[processIndex].arrivalTime, READY);
                        fprintf(logFile, "process %c is running for the first time on core %d\n", currProcess->pid, i); // debug
                    }
                    fprintf(logFile, "process %c enqueued successfully to run on core %d\n", currProcess->pid, coreIndex); // debug
                    i--;
                    continue;
                }
                // if the core is not busy and the processes queue is empty or first process has not arrived yet, search for a process in the ready queue of other cores
                fprintf(logFile, "searching for a process in the ready queue of other cores\n"); // debug
                int found = 0;
                // find the least current burst time process in the ready queue of other cores
                int leastCore = 9999, leastBurstTime = 9999, leastCoreIndex = -1;
                for (int j = 0; j < numCores; j++)
                {
                    if (j == i)
                    {
                        continue;
                    }
                    if (check_queue_empty(READY, j) == QUEUE_NOT_EMPTY)
                    {
                        fprintf(logFile, "ready queue of core %d is not empty\n", j); // debug
                        leastBurstTime = get_least_current_burst_time_process(READY, j);
                        if (leastBurstTime < leastCore)
                        {
                            leastCore = leastBurstTime;
                            leastCoreIndex = j;
                            found = 1;
                        }
                    }
                }
                if (found == 1)
                {
                    fprintf(logFile, "found a process in the ready queue of core %d\n", leastCoreIndex); // debug
                    if (get_process_with_property(READY, currProcess, leastCoreIndex, current_burst_time_match, &leastBurstTime) == PROCESS_DEQUEUE_FAILED)
                    {
                        fprintf(logFile, "dequeue failed from ready of core %d\n", leastCoreIndex); // debug
                        continue;
                    }
                    currProcess->process_status = READY;
                    enqueue_process(*currProcess, i);
                    fprintf(logFile, "process %c enqueued successfully to run on core %d\n", currProcess->pid, i); // debug
                    i--;
                    continue;
                }
                else
                {
                    // if no process is found, schedule an idle process
                    fprintf(logFile, "no processes to run on core %d\n", i); // debug
                    scheduleProcess(result, i, 'i', currentTime, currentTime + 1);
                }
            }
            else
            {
                fprintf(logFile, "core %d start of running\n", i); // debug
                peek_process(RUN, &currProcess, i);
                char processId = currProcess->pid;
                int processIndex = getProcessIndex(result, processId);
                ProcessInfo *process = &(values->processes[processIndex]);

                BurstInfo *burst = &(process->bursts[currProcess->next_burst]);

                if (strcmp(burst->type, "CPU") == 0)
                {
                    setProcessStatus(result, processIndex, 1, RUN);
                    scheduleProcess(result, i, processId, currentTime, currentTime + 1);
                    fprintf(logFile, "process %c is running on core %d cpu\n", processId, i); // debug
                    burst->duration--;
                }
                else
                {
                    fprintf(logFile, "process %c made IO on core %d\n", processId, i); // debug
                    setProcessStatus(result, processIndex, burst->duration, BLOCKED);
                    currProcess->next_burst++;
                    process->burstsCount--;
                    dequeue_process(RUN, currProcess, i);
                    currProcess->original_core = i;
                    currProcess->process_status = BLOCKED;
                    currProcess->waiting_time = burst->duration;
                    enqueue_process(*currProcess, numCores);
                    i--;
                    continue;
                }

                if (burst->duration == 0)
                {
                    fprintf(logFile, "process %c burst completed\n", processId); // debug
                    currProcess->next_burst++;
                    process->burstsCount--;
                    if (process->burstsCount == 0)
                    {
                        fprintf(logFile, "process %c completed\n", processId); // debug
                        setProcessTurnaroundTime(result, processIndex, currentTime + 1);
                        dequeue_process(RUN, currProcess, i);
                    }
                }
                fprintf(logFile, "core %d end of running\n", i); // debug
            }
            fprintf(logFile, "core %d end of checking\n", i); // debug
        }
        currentTime++;
        update_waiting_time(currentTime, numCores);
        fprintf(logFile, "end of iteration\n\n"); // debug
    }
    free(currProcess);
    currProcess = NULL;

    // Calculate and set average turnaround time and average response time
    double totalTurnaroundTime = 0, totalResponseTime = 0;
    for (int i = 0; i < result->processesCount; i++)
    {
        totalTurnaroundTime += result->processesAnalysis[i].turnaroundTime;
        totalResponseTime += result->processesAnalysis[i].responseTime;
    }
    setAvgTurnaroundTime(result, totalTurnaroundTime / result->processesCount);
    setAvgResponseTime(result, totalResponseTime / result->processesCount);
    fprintf(logFile, "///////////////////////////// end of SJF1 Simulation //////////////////////////\n"); // debug
}

void simulateSJF2(ProcessedValues *values, SimulationResult *result, FILE *logFile)
{
    fprintf(logFile, "///////////////////////////// start of SJF2 Simulation //////////////////////////\n"); // debug
    int numCores = values->numCores;
    // ready queue of the extra set of queues is used to store the processes in the order of their priority
    // Only using the run queue of the cores. The blocked and ready queue is not used
    init_processes_queues(numCores + 1);
    int coreIndex = 0;
    int currentTime = 0;
    int arrived = 0;
    fprintf(logFile, "numCores %d\n", numCores); // debug

    fprintf(logFile, "processes enqueed successfully \n"); // debug
    queue_capacity_status_e status = check_queue_empty(READY, numCores);
    fprintf(logFile, "status %d\n\n", status); // debug

    while (1)
    {
        fprintf(logFile, "start of iteration\n");           // debug
        fprintf(logFile, "current time %d\n", currentTime); // debug
        // get number of busy cores
        int busyCores = 0;
        for (int i = 0; i < numCores; i++)
        {
            if (check_queue_empty(RUN, i) == QUEUE_NOT_EMPTY)
            {
                fprintf(logFile, "core %d is busy\n", i); // debug
                busyCores++;
            }
            else
            {
                fprintf(logFile, "core %d is not busy\n", i); // debug
            }
        }
        // enqueue the arrived processes in ready queue based on priority
        for (int i = 0; i < values->processesCount; i++)
        {
            if (values->processes[i].arrivalTime == currentTime)
            {
                char processId = values->processes[i].id;
                int processIndex = getProcessIndex(result, processId);
                process_info_t *pinfo = fill_process_info(values, processIndex);
                if (result->processesAnalysis[processIndex].firstRunTime == -1)
                {
                    fprintf(logFile, "process %c status %d\n\n", pinfo->pid, pinfo->process_status); // debug
                    pinfo->process_status = READY;
                    enqueue_process_duration(*pinfo, numCores);
                    fprintf(logFile, "process %c status %d\n\n", pinfo->pid, pinfo->process_status); // debug
                    arrived++;
                }
            }
        }

        // if all are not busy and processes queue is empty, break
        if (arrived == values->processesCount && busyCores == 0 && check_queue_empty(READY, numCores) == QUEUE_EMPTY)
        {
            fprintf(logFile, "all cores are not busy and processes queue is empty\n"); // debug
            break;
        }

        fprintf(logFile, "busy cores %d\n", busyCores); // debug

        // check if any core is available to run a process
        for (int i = 0; i < numCores; i++)
        {
            if (check_queue_empty(RUN, i) == QUEUE_EMPTY)
            {
                // if a core is available, peek the first process in the processes queue and enqueue it in the run queue of the core if its arrival time is less than or equal to the current time
                process_info_t *currProcess = (process_info_t *)malloc(sizeof(process_info_t));
                if (peek_process(READY, &currProcess, numCores) == PROCESS_DEQUEUE_FAILED)
                {
                    fprintf(logFile, "empty process quee\n"); // debug
                    continue;
                }
                fprintf(logFile, "currProcess to assign to a core: %c\n", currProcess->pid); // debug
                if (currProcess->arrival_time > currentTime)
                {
                    fprintf(logFile, "process %c not arrived yet\n", currProcess->pid);       // debug
                    fprintf(logFile, "process arrival time %d\n", currProcess->arrival_time); // debug
                    fprintf(logFile, "so we breaking\n");                                     // debug
                    break;
                }
                else
                {
                    fprintf(logFile, "process %c arrived at time %d\n", currProcess->pid, currentTime); // debug
                    // dequeue process from processes queue and enqueue it in the ready queue of the first available core
                    coreIndex = i;
                    process_info_t *currProcess = (process_info_t *)malloc(sizeof(process_info_t));
                    if (dequeue_process(READY, currProcess, numCores) == PROCESS_DEQUEUE_FAILED)
                    {
                        fprintf(logFile, "dequeue failed\n"); // debug
                        continue;
                    }
                    currProcess->process_status = RUN;
                    enqueue_process(*currProcess, coreIndex);
                    int processIndex = getProcessIndex(result, currProcess->pid);
                    result->processesAnalysis[processIndex].firstRunTime = currentTime;
                    setProcessResponseTime(result, processIndex, currentTime);
                    setProcessStatus(result, processIndex, result->processesAnalysis[processIndex].firstRunTime - result->processesAnalysis[processIndex].arrivalTime, READY);
                    fprintf(logFile, "process %c enqueued successfully to run on core %d\n", currProcess->pid, coreIndex); // debug
                }
            }
        }

        // iterate over all cores and run the process in each core for 1 time unit
        for (int i = 0; i < numCores; i++)
        {
            // check if the run queue of the core is empty
            if (check_queue_empty(RUN, i) == QUEUE_EMPTY)
            {
                fprintf(logFile, "run queue of core %d is empty\n", i); // debug
                scheduleProcess(result, i, 'i', currentTime, currentTime + 1);
                continue;
            }
            else
            {
                fprintf(logFile, "core %d start of running\n", i); // debug
                process_info_t *currProcess = (process_info_t *)malloc(sizeof(process_info_t));
                peek_process(RUN, &currProcess, i);
                char processId = currProcess->pid;
                int processIndex = getProcessIndex(result, processId);
                ProcessInfo *process = &(values->processes[processIndex]);

                BurstInfo *burst = &(process->bursts[currProcess->next_burst]);

                if (strcmp(burst->type, "CPU") == 0)
                {
                    setProcessStatus(result, processIndex, 1, RUN);
                    scheduleProcess(result, i, processId, currentTime, currentTime + 1);
                    fprintf(logFile, "process %c is running on core %d cpu\n", processId, i); // debug
                    burst->duration--;
                }
                else
                {
                    setProcessStatus(result, processIndex, 1, BLOCKED);
                    scheduleProcess(result, i, 'i', currentTime, currentTime + 1);
                    fprintf(logFile, "process %c is running on core %d io\n", processId, i); // debug
                    burst->duration--;
                }

                if (burst->duration == 0)
                {
                    fprintf(logFile, "process %c burst completed\n", processId); // debug
                    currProcess->next_burst++;
                    process->burstsCount--;
                    if (process->burstsCount == 0)
                    {
                        fprintf(logFile, "process %c completed\n", processId); // debug
                        setProcessTurnaroundTime(result, processIndex, currentTime + 1);
                        dequeue_process(RUN, currProcess, i);
                    }
                }
                fprintf(logFile, "core %d end of running\n", i); // debug
            }
        }
        currentTime++;
        fprintf(logFile, "end of iteration\n\n"); // debug
    }

    // Calculate and set average turnaround time and average response time
    double totalTurnaroundTime = 0, totalResponseTime = 0;
    for (int i = 0; i < result->processesCount; i++)
    {
        totalTurnaroundTime += result->processesAnalysis[i].turnaroundTime;
        totalResponseTime += result->processesAnalysis[i].responseTime;
    }
    setAvgTurnaroundTime(result, totalTurnaroundTime / result->processesCount);
    setAvgResponseTime(result, totalResponseTime / result->processesCount);
    fprintf(logFile, "///////////////////////////// end of SJF2 Simulation //////////////////////////\n"); // debug
}
void simulateSJF3(ProcessedValues *values, SimulationResult *result, FILE *logFile)
{
    fprintf(logFile, "///////////////////////////// start of SJF3 Simulation //////////////////////////\n"); // debug
    int numCores = values->numCores;
    // ready queue of the extra set of queues is used to store the processes in the order of their priority
    // Only using the run queue of the cores. The blocked and ready queue is not used
    init_processes_queues(numCores + 1);
    int coreIndex = 0;
    int currentTime = 0;
    int arrived = 0;
    int ff = 0;
    fprintf(logFile, "numCores %d\n", numCores); // debug

    process_info_t *currProcess = (process_info_t *)malloc(sizeof(process_info_t));
    process_info_t *check = (process_info_t *)malloc(sizeof(process_info_t));
    fprintf(logFile, "processes enqueed successfully \n"); // debug
    queue_capacity_status_e status = check_queue_empty(READY, numCores);
    fprintf(logFile, "status %d\n\n", status); // debug

    while (1)
    {
        fprintf(logFile, "start of iteration\n");           // debug
        fprintf(logFile, "current time %d\n", currentTime); // debug
        // get number of busy cores
        int busyCores = 0;
        for (int i = 0; i < numCores; i++)
        {
            if (check_queue_empty(RUN, i) == QUEUE_NOT_EMPTY)
            {
                fprintf(logFile, "core %d is busy\n", i); // debug
                busyCores++;
            }
            else
            {
                fprintf(logFile, "core %d is not busy\n", i); // debug
            }
        }
        // enqueue the arrived processes in ready queue based on priority
        if (ff == 0)
        {
            for (int i = 0; i < values->processesCount; i++)
            {
                if (values->processes[i].arrivalTime == currentTime)
                {
                    char processId = values->processes[i].id;
                    int processIndex = getProcessIndex(result, processId);
                    process_info_t *pinfo = fill_process_info(values, processIndex);
                    if (result->processesAnalysis[processIndex].firstRunTime == -1)
                    {
                        fprintf(logFile, "process %c status %d\n\n", pinfo->pid, pinfo->process_status); // debug
                        pinfo->process_status = READY;
                        enqueue_process_duration(*pinfo, numCores);
                        fprintf(logFile, "process %c status %d\n\n", pinfo->pid, pinfo->process_status); // debug
                        arrived++;
                    }
                }
            }
        }
        ff = 0;
        // if all are not busy and processes queue is empty, break
        if (arrived == values->processesCount && busyCores == 0 && check_queue_empty(READY, numCores) == QUEUE_EMPTY)
        {
            fprintf(logFile, "all cores are not busy and processes queue is empty\n"); // debug
            break;
        }

        fprintf(logFile, "busy cores %d\n", busyCores); // debug

        // check if any core is available to run a process
        if (peek_process(READY, &currProcess, numCores) == PROCESS_DEQUEUE_FAILED)
        {
            fprintf(logFile, "dequeue failed\n"); // debug
        }
        else
        {
            if ((currProcess->original_core) != -1)
            {
                if (check_queue_empty(RUN, currProcess->original_core) == QUEUE_EMPTY)
                {
                    fprintf(logFile, "%c original core empty\n", currProcess->pid);
                    fprintf(logFile, "process %c arrived at time %d\n", currProcess->pid, currentTime); // debug
                    // dequeue process from processes queue and enqueue it in the ready queue of the first available core
                    process_info_t *currProcess = (process_info_t *)malloc(sizeof(process_info_t));
                    if (dequeue_process(READY, currProcess, numCores) == PROCESS_DEQUEUE_FAILED)
                    {
                        fprintf(logFile, "dequeue failed\n"); // debug
                        continue;
                    }
                    currProcess->process_status = RUN;
                    enqueue_process(*currProcess, currProcess->original_core);
                    int processIndex = getProcessIndex(result, currProcess->pid);
                    setProcessStatus(result, processIndex, currentTime - currProcess->waiting_time, READY);
                    currProcess->waiting_time = 0;
                    currProcess->original_core = -1;
                    fprintf(logFile, "process %c enqueued successfully to run on core line 883 %d\n", currProcess->pid, currProcess->original_core); // debug
                    peek_process(READY, &currProcess, numCores);
                    fprintf(logFile, "the process in ready queue is %c\n", currProcess->pid);
                    ff = 1;
                }
            }
            if (ff == 0)
            {
                for (int i = 0; i < numCores; i++)
                {
                    if (check_queue_empty(RUN, i) == QUEUE_EMPTY)
                    {
                        // if a core is available, peek the first process in the processes queue and enqueue it in the run queue of the core if its arrival time is less than or equal to the current time
                        if (dequeue_process(READY, currProcess, numCores) == PROCESS_DEQUEUE_FAILED)
                        {
                            fprintf(logFile, "empty process quee\n"); // debug
                            continue;
                        }
                        fprintf(logFile, "currProcess to assign to a core: %c\n", currProcess->pid); // debug
                        if (currProcess->arrival_time > currentTime)
                        {
                            fprintf(logFile, "process %c not arrived yet\n", currProcess->pid);       // debug
                            fprintf(logFile, "process arrival time %d\n", currProcess->arrival_time); // debug
                            fprintf(logFile, "so we breaking\n");                                     // debug
                            break;
                        }
                        else
                        {
                            fprintf(logFile, "%c other core empty\n", currProcess->pid);
                            fprintf(logFile, "process %c arrived at time %d\n", currProcess->pid, currentTime); // debug
                            // dequeue process from processes queue and enqueue it in the ready queue of the first available core
                            coreIndex = i;
                            currProcess->process_status = RUN;
                            enqueue_process(*currProcess, coreIndex);
                            if (currProcess->original_core != -1)
                            {
                                setProcessStatus(result, getProcessIndex(result, currProcess->pid), currentTime - currProcess->waiting_time, READY);
                                currProcess->waiting_time = 0;
                                currProcess->original_core = -1;
                            }
                            int processIndex = getProcessIndex(result, currProcess->pid);
                            fprintf(logFile, "process %c enqueued to run on core line 916%d\n", currProcess->pid, coreIndex); // debug
                            if (result->processesAnalysis[processIndex].firstRunTime == -1)
                            {
                                result->processesAnalysis[processIndex].firstRunTime = currentTime;
                                setProcessResponseTime(result, processIndex, currentTime);
                                setProcessStatus(result, processIndex, result->processesAnalysis[processIndex].firstRunTime - result->processesAnalysis[processIndex].arrivalTime, READY);
                            }
                            fprintf(logFile, "process %c enqueued successfully to run on core line 922 %d\n", currProcess->pid, coreIndex); // debug
                            peek_process(READY, &currProcess, numCores);
                            fprintf(logFile, "the process in ready queue is %c\n", currProcess->pid);
                            ff = 1;
                            break;
                        }
                    }
                }
            }
            else
            {
                continue;
            }
            if (ff == 0)
            {
                if (currProcess->original_core != -1)
                {
                    if (peek_process(READY, &currProcess, numCores) == PROCESS_DEQUEUE_FAILED)
                    {
                        fprintf(logFile, "dequeue failed\n"); // debug
                    }
                    peek_process(RUN, &check, currProcess->original_core);
                    if (currProcess->total_duration < check->total_duration)
                    {
                        fprintf(logFile, "%c original core less priority\n", currProcess->pid);
                        if (dequeue_process(RUN, check, currProcess->original_core) == PROCESS_DEQUEUE_FAILED)
                        {
                            fprintf(logFile, "dequeue failed from ready of core %d\n", numCores); // debug
                        }
                        check->process_status = READY;
                        check->original_core = currProcess->original_core;
                        check->waiting_time = currentTime;
                        enqueue_process_duration(*check, numCores);

                        if (dequeue_process(READY, currProcess, numCores) == PROCESS_DEQUEUE_FAILED)
                        {
                            fprintf(logFile, "dequeue failed from ready of core %d\n", numCores); // debug
                        }
                        currProcess->process_status = RUN;
                        enqueue_process(*currProcess, currProcess->original_core);
                        setProcessStatus(result, getProcessIndex(result, currProcess->pid), currentTime - currProcess->waiting_time, READY);
                        currProcess->waiting_time = 0;
                        currProcess->original_core = -1;
                        ff = 1;
                    }
                }
            }
            else
            {
                continue;
            }
            if (ff == 0)
            {
                for (int i = 0; i < numCores; i++)
                {
                    process_info_t *currProcess = (process_info_t *)malloc(sizeof(process_info_t));
                    process_info_t *check = (process_info_t *)malloc(sizeof(process_info_t));
                    if (peek_process(READY, &currProcess, numCores) == PROCESS_DEQUEUE_FAILED)
                    {
                        fprintf(logFile, "dequeue failed\n"); // debug
                    }
                    peek_process(RUN, &check, i);
                    if (currProcess->total_duration < check->total_duration)
                    {
                        fprintf(logFile, "%c other core less priority\n", currProcess->pid);
                        if (dequeue_process(RUN, check, i) == PROCESS_DEQUEUE_FAILED)
                        {
                            fprintf(logFile, "dequeue failed from ready of core %d\n", numCores); // debug
                        }
                        fprintf(logFile, "%c in ready queue, %c in running for %d\n", check->pid, currProcess->pid, i);
                        check->process_status = READY;
                        check->original_core = i;
                        check->waiting_time = currentTime;
                        enqueue_process_duration(*check, numCores);
                        if (dequeue_process(READY, currProcess, numCores) == PROCESS_DEQUEUE_FAILED)
                        {
                            fprintf(logFile, "dequeue failed from ready of core %d\n", numCores); // debug
                        }
                        fprintf(logFile, "%d\n", check_queue_empty(RUN, i));
                        currProcess->process_status = RUN;
                        enqueue_process(*currProcess, i);
                        fprintf(logFile, "%c in ready queue, %c in running for %d\n", check->pid, currProcess->pid, i);
                        if (currProcess->original_core != -1)
                        {
                            setProcessStatus(result, getProcessIndex(result, currProcess->pid), currentTime - currProcess->waiting_time, READY);
                            currProcess->waiting_time = 0;
                            currProcess->original_core = -1;
                        }
                        if (result->processesAnalysis[getProcessIndex(result, currProcess->pid)].firstRunTime == -1)
                        {
                            result->processesAnalysis[getProcessIndex(result, currProcess->pid)].firstRunTime = currentTime;
                            setProcessResponseTime(result, getProcessIndex(result, currProcess->pid), currentTime);
                            setProcessStatus(result, getProcessIndex(result, currProcess->pid), result->processesAnalysis[getProcessIndex(result, currProcess->pid)].firstRunTime - result->processesAnalysis[getProcessIndex(result, currProcess->pid)].arrivalTime, READY);
                        }
                        ff = 1;
                        break;
                    }
                }
            }
            else
            {
                continue;
            }
            if (ff == 1)
            {
                continue;
            }
        }
        // iterate over all cores and run the process in each core for 1 time unit
        for (int i = 0; i < numCores; i++)
        {
            // check if the run queue of the core is empty
            if (check_queue_empty(RUN, i) == QUEUE_EMPTY)
            {
                fprintf(logFile, "run queue of core %d is empty\n", i); // debug
                scheduleProcess(result, i, 'i', currentTime, currentTime + 1);
                continue;
            }
            else
            {
                fprintf(logFile, "core %d start of running\n", i); // debug
                peek_process(RUN, &currProcess, i);
                char processId = currProcess->pid;
                int processIndex = getProcessIndex(result, processId);
                ProcessInfo *process = &(values->processes[processIndex]);

                BurstInfo *burst = &(process->bursts[currProcess->next_burst]);

                if (strcmp(burst->type, "CPU") == 0)
                {
                    setProcessStatus(result, processIndex, 1, RUN);
                    scheduleProcess(result, i, processId, currentTime, currentTime + 1);
                    fprintf(logFile, "process %c is running on core %d cpu\n", processId, i); // debug
                    burst->duration--;
                }
                else
                {
                    setProcessStatus(result, processIndex, 1, BLOCKED);
                    scheduleProcess(result, i, 'i', currentTime, currentTime + 1);
                    fprintf(logFile, "process %c is running on core %d io\n", processId, i); // debug
                    burst->duration--;
                }

                if (burst->duration == 0)
                {
                    fprintf(logFile, "process %c burst completed\n", processId); // debug
                    currProcess->next_burst++;
                    process->burstsCount--;
                    if (process->burstsCount == 0)
                    {
                        fprintf(logFile, "process %c completed\n", processId); // debug
                        setProcessTurnaroundTime(result, processIndex, currentTime + 1);
                        dequeue_process(RUN, currProcess, i);
                    }
                }
                fprintf(logFile, "core %d end of running\n", i); // debug
            }
        }
        currentTime++;
        fprintf(logFile, "end of iteration\n\n"); // debug
    }

    // Calculate and set average turnaround time and average response time
    double totalTurnaroundTime = 0, totalResponseTime = 0;
    for (int i = 0; i < result->processesCount; i++)
    {
        totalTurnaroundTime += result->processesAnalysis[i].turnaroundTime;
        totalResponseTime += result->processesAnalysis[i].responseTime;
    }
    setAvgTurnaroundTime(result, totalTurnaroundTime / result->processesCount);
    setAvgResponseTime(result, totalResponseTime / result->processesCount);
    fprintf(logFile, "///////////////////////////// end of SJF3 Simulation //////////////////////////\n"); // debug
}

void simulatePriority1(ProcessedValues *values, SimulationResult *result, FILE *logFile)
{
    fprintf(logFile, "///////////////////////////// start of Priority 1 Simulation //////////////////////////\n"); // debug
    int numCores = values->numCores;
    // ready queue of the extra set of queues is used to store the processes in the order they arrived
    // blocked queue of the extra set of queues is used to store the processes that are waiting for IO
    // run queue of the extra set of queues is not used
    // Only using the run and ready queue of the cores. The blocked queue is not used
    init_processes_queues(numCores + 1);
    int coreIndex = 0;
    int currentTime = 0;
    fprintf(logFile, "numCores %d\n", numCores); // debug

    // Iterate over each process in the order they arrived and enqueue them in the processes queue
    for (int i = 0; i < values->processesCount; i++)
    {
        char processId = values->processes[i].id;
        int processIndex = getProcessIndex(result, processId);
        process_info_t *pinfo = fill_process_info(values, processIndex);
        pinfo->process_status = READY;
        enqueue_process(*pinfo, numCores);
    }
    fprintf(logFile, "processes enqueed successfully \n"); // debug
    queue_capacity_status_e status = check_queue_empty(READY, numCores);
    fprintf(logFile, "status %d\n\n", status); // debug

    process_info_t *currProcess = (process_info_t *)malloc(sizeof(process_info_t));
    while (1)
    {
        fprintf(logFile, "start of iteration\n");           // debug
        fprintf(logFile, "current time %d\n", currentTime); // debug
        // get number of busy cores
        int busyCores = 0;
        for (int i = 0; i < numCores; i++)
        {
            if (check_queue_empty(RUN, i) == QUEUE_NOT_EMPTY)
            {
                fprintf(logFile, "core %d is busy\n", i); // debug
                busyCores++;
            }
            else
            {
                if (check_queue_empty(BLOCKED, i) == QUEUE_NOT_EMPTY)
                {
                    fprintf(logFile, "core %d is not busy but has blocked processes\n", i); // debug
                    busyCores++;
                }
                else if (check_queue_empty(READY, i) == QUEUE_NOT_EMPTY)
                {
                    fprintf(logFile, "core %d is not busy but has ready processes\n", i); // debug
                    busyCores++;
                }
                else
                {
                    fprintf(logFile, "core %d is not busy\n", i); // debug
                }
            }
        }

        // if all are not busy and processes queue is empty, break
        if (busyCores == 0 && (check_queue_empty(READY, numCores) == QUEUE_EMPTY) && (check_queue_empty(BLOCKED, numCores) == QUEUE_EMPTY))
        {
            fprintf(logFile, "all cores are not busy and processes queue and blocked queue is empty\n"); // debug
            break;
        }

        fprintf(logFile, "Either cores hasnt finished, or processes or blocked queue is not empty\n"); // debug

        // iterate over all cores and run the process in each core for 1 time unit
        for (int i = 0; i < numCores; i++)
        {
            fprintf(logFile, "core %d start of checking\n", i); // debug
            // check if the run queue of the core is empty
            if (check_queue_empty(RUN, i) == QUEUE_EMPTY)
            {
                fprintf(logFile, "run queue of core %d is empty\n", i); // debug

                fprintf(logFile, "checking if a new process has arrived\n"); // debug
                // if a core is available, peek the first process in the processes queue and enqueue it in the ready queue of the core if its arrival time is less than or equal to the current time

                if (peek_process(READY, &currProcess, numCores) == PROCESS_DEQUEUE_FAILED)
                {
                    fprintf(logFile, "empty processes quee\n"); // debug
                }
                else
                {
                    fprintf(logFile, "currProcess to assign to a core: %c\n", currProcess->pid); // debug
                    if (currProcess->arrival_time > currentTime)
                    {
                        fprintf(logFile, "process %c has not arrived yet\n", currProcess->pid);   // debug
                        fprintf(logFile, "process arrival time %d\n", currProcess->arrival_time); // debug
                    }
                    else
                    {
                        // find the core with the least ready queue size
                        int leastCore = 9999, leastCoreIndex = -1;
                        for (int j = 0; j < numCores; j++)
                        {
                            int readyQueueSize = get_queue_size(READY, j);
                            if (readyQueueSize < leastCore)
                            {
                                leastCore = readyQueueSize;
                                leastCoreIndex = j;
                            }
                            fprintf(logFile, "core %d ready queue size %d\n", j, readyQueueSize); // debug
                        }
                        fprintf(logFile, "process %c moved to ready at time %d\n", currProcess->pid, currentTime); // debug
                        // dequeue process from processes queue and enqueue it in the ready queue of the first available core
                        coreIndex = leastCoreIndex;

                        if (dequeue_process(READY, currProcess, numCores) == PROCESS_DEQUEUE_FAILED)
                        {
                            fprintf(logFile, "dequeue failed\n"); // debug
                            continue;
                        }
                        currProcess->process_status = READY;
                        enqueue_process(*currProcess, coreIndex);
                        fprintf(logFile, "process %c enqueued successfully to ready queue on core %d\n", currProcess->pid, coreIndex); // debug
                        i--;
                        continue;
                    }
                }

                if (check_queue_empty(READY, i) == QUEUE_NOT_EMPTY)
                {
                    fprintf(logFile, "ready queue of core %d is not empty\n", i); // debug

                    // dequeue the process with the least priority from the ready queue of the core
                    int leastPriority = get_least_priority_process(READY, i);
                    if (get_process_with_property(READY, currProcess, i, priority_match, &leastPriority) == PROCESS_DEQUEUE_FAILED)
                    {
                        fprintf(logFile, "dequeue failed from ready of core %d\n", i); // debug
                        continue;
                    }

                    // check if the process is comming from blocked queue and has finished execution
                    if (currProcess->waiting_time == 9999)
                    {
                        fprintf(logFile, "process %c came from IO and finished\n", currProcess->pid); // debug
                        setProcessTurnaroundTime(result, getProcessIndex(result, currProcess->pid), currProcess->execution_time);
                        i--;
                        continue;
                    }
                    // check if the process is comming from blocked queue and didnt finish yet
                    if (currProcess->waiting_time > 0)
                    {
                        fprintf(logFile, "process %c came for IO and it was ready since %d\n", currProcess->pid, currProcess->waiting_time); // debug
                        setProcessStatus(result, getProcessIndex(result, currProcess->pid), currentTime - currProcess->waiting_time, READY);
                        currProcess->waiting_time = 0;
                    }
                    currProcess->process_status = RUN;
                    enqueue_process(*currProcess, i);

                    // check if the proecces runs for the first time and set its first run time
                    int processIndex = getProcessIndex(result, currProcess->pid);
                    if (result->processesAnalysis[processIndex].firstRunTime == -1)
                    {
                        result->processesAnalysis[processIndex].firstRunTime = currentTime;
                        setProcessResponseTime(result, processIndex, currentTime);
                        setProcessStatus(result, processIndex, result->processesAnalysis[processIndex].firstRunTime - result->processesAnalysis[processIndex].arrivalTime, READY);
                        fprintf(logFile, "process %c is running for the first time on core %d\n", currProcess->pid, i); // debug
                    }
                    fprintf(logFile, "process %c enqueued successfully to run on core %d\n", currProcess->pid, coreIndex); // debug
                    i--;
                    continue;
                }
                // if the core is not busy and the processes queue is empty or first process has not arrived yet, search for a process in the ready queue of other cores
                fprintf(logFile, "searching for a process in the ready queue of other cores\n"); // debug
                int found = 0;
                // find the least current burst time process in the ready queue of other cores
                int leastCore = 9999, leastPriority = 9999, leastCoreIndex = -1;
                for (int j = 0; j < numCores; j++)
                {
                    if (j == i)
                    {
                        continue;
                    }
                    if (check_queue_empty(READY, j) == QUEUE_NOT_EMPTY)
                    {
                        fprintf(logFile, "ready queue of core %d is not empty\n", j); // debug
                        leastPriority = get_least_priority_process(READY, j);
                        if (leastPriority < leastCore)
                        {
                            leastCore = leastPriority;
                            leastCoreIndex = j;
                            found = 1;
                        }
                    }
                }

                if (found == 1)
                {
                    fprintf(logFile, "found a process in the ready queue of core %d\n", leastCoreIndex); // debug
                    if (get_process_with_property(READY, currProcess, leastCoreIndex, priority_match, &leastPriority) == PROCESS_DEQUEUE_FAILED)
                    {
                        fprintf(logFile, "dequeue failed from ready of core %d\n", leastCoreIndex); // debug
                        continue;
                    }
                    currProcess->process_status = READY;
                    enqueue_process(*currProcess, i);
                    fprintf(logFile, "process %c enqueued successfully to run on core %d\n", currProcess->pid, i); // debug
                    i--;
                    continue;
                }
                else
                {
                    // if no process is found, schedule an idle process
                    fprintf(logFile, "no processes to run on core %d\n", i); // debug
                    scheduleProcess(result, i, 'i', currentTime, currentTime + 1);
                }
            }
            else
            {
                fprintf(logFile, "core %d start of running\n", i); // debug

                peek_process(RUN, &currProcess, i);
                char processId = currProcess->pid;
                int processIndex = getProcessIndex(result, processId);
                ProcessInfo *process = &(values->processes[processIndex]);

                BurstInfo *burst = &(process->bursts[currProcess->next_burst]);

                if (strcmp(burst->type, "CPU") == 0)
                {
                    setProcessStatus(result, processIndex, 1, RUN);
                    scheduleProcess(result, i, processId, currentTime, currentTime + 1);
                    fprintf(logFile, "process %c is running on core %d cpu\n", processId, i); // debug
                    burst->duration--;
                }
                else
                {
                    fprintf(logFile, "process %c made IO on core %d\n", processId, i); // debug
                    setProcessStatus(result, processIndex, burst->duration, BLOCKED);
                    currProcess->next_burst++;
                    process->burstsCount--;
                    dequeue_process(RUN, currProcess, i);
                    currProcess->original_core = i;
                    currProcess->process_status = BLOCKED;
                    currProcess->waiting_time = burst->duration;
                    enqueue_process(*currProcess, numCores);
                    i--;
                    continue;
                }

                if (burst->duration == 0)
                {
                    fprintf(logFile, "process %c burst completed\n", processId); // debug
                    currProcess->next_burst++;
                    process->burstsCount--;
                    if (process->burstsCount == 0)
                    {
                        fprintf(logFile, "process %c completed\n", processId); // debug
                        setProcessTurnaroundTime(result, processIndex, currentTime + 1);
                        dequeue_process(RUN, currProcess, i);
                    }
                }
                fprintf(logFile, "core %d end of running\n", i); // debug
            }
            fprintf(logFile, "core %d end of checking\n", i); // debug
        }
        currentTime++;
        update_waiting_time(currentTime, numCores);
        fprintf(logFile, "end of iteration\n\n"); // debug
    }
    free(currProcess);
    currProcess = NULL;

    // Calculate and set average turnaround time and average response time
    double totalTurnaroundTime = 0, totalResponseTime = 0;
    for (int i = 0; i < result->processesCount; i++)
    {
        totalTurnaroundTime += result->processesAnalysis[i].turnaroundTime;
        totalResponseTime += result->processesAnalysis[i].responseTime;
    }
    setAvgTurnaroundTime(result, totalTurnaroundTime / result->processesCount);
    setAvgResponseTime(result, totalResponseTime / result->processesCount);
    fprintf(logFile, "///////////////////////////// end of Priority 1 Simulation //////////////////////////\n"); // debug
}

void simulatePriority2(ProcessedValues *values, SimulationResult *result, FILE *logFile)
{
    fprintf(logFile, "///////////////////////////// start of Priority 2 Simulation //////////////////////////\n"); // debug
    int numCores = values->numCores;
    // ready queue of the extra set of queues is used to store the processes in the order they arrived
    // blocked queue of the extra set of queues is used to store the processes that are waiting for IO
    // Only using the run and ready queue of the cores. The blocked queue is not used
    init_processes_queues(numCores + 1);
    int coreIndex = 0;
    int currentTime = 0;
    fprintf(logFile, "numCores %d\n", numCores); // debug

    // Iterate over each process in the order they arrived and enqueue them in the processes queue
    for (int i = 0; i < values->processesCount; i++)
    {
        char processId = values->processes[i].id;
        int processIndex = getProcessIndex(result, processId);
        process_info_t *pinfo = fill_process_info(values, processIndex);
        pinfo->process_status = READY;
        enqueue_process(*pinfo, numCores);
    }
    fprintf(logFile, "processes enqueed successfully \n"); // debug
    queue_capacity_status_e status = check_queue_empty(READY, numCores);
    fprintf(logFile, "status %d\n\n", status); // debug

    process_info_t *currProcess = (process_info_t *)malloc(sizeof(process_info_t));
    while (1)
    {
        fprintf(logFile, "start of iteration\n");           // debug
        fprintf(logFile, "current time %d\n", currentTime); // debug
        // get number of busy cores
        int busyCores = 0;
        for (int i = 0; i < numCores; i++)
        {
            if (check_queue_empty(RUN, i) == QUEUE_NOT_EMPTY)
            {
                fprintf(logFile, "core %d is busy\n", i); // debug
                busyCores++;
            }
            else
            {
                if (check_queue_empty(BLOCKED, i) == QUEUE_NOT_EMPTY)
                {
                    fprintf(logFile, "core %d is not busy but has blocked processes\n", i); // debug
                    busyCores++;
                }
                else if (check_queue_empty(READY, i) == QUEUE_NOT_EMPTY)
                {
                    fprintf(logFile, "core %d is not busy but has ready processes\n", i); // debug
                    busyCores++;
                }
                else
                {
                    fprintf(logFile, "core %d is not busy\n", i); // debug
                }
            }
        }

        // if all are not busy and processes queue is empty, break
        if (busyCores == 0 && (check_queue_empty(READY, numCores) == QUEUE_EMPTY) && (check_queue_empty(BLOCKED, numCores) == QUEUE_EMPTY) && (check_queue_empty(RUN, numCores) == QUEUE_EMPTY))
        {
            fprintf(logFile, "all cores are not busy and processes queue and blocked queue is empty\n"); // debug
            break;
        }

        fprintf(logFile, "Either cores hasnt finished, or processes or blocked queue is not empty\n"); // debug

        // iterate over all cores and run the process in each core for 1 time unit
        for (int i = 0; i < numCores; i++)
        {
            fprintf(logFile, "core %d start of checking\n", i); // debug
            // check if the run queue of the core is empty
            if (check_queue_empty(RUN, i) == QUEUE_EMPTY)
            {
                fprintf(logFile, "run queue of core %d is empty\n", i); // debug

                fprintf(logFile, "checking if a new process has arrived\n"); // debug
                // if a core is available, peek the first process in the processes queue and enqueue it in the ready queue of the core if its arrival time is less than or equal to the current time

                if (peek_process(READY, &currProcess, numCores) == PROCESS_DEQUEUE_FAILED)
                {
                    fprintf(logFile, "empty processes quee\n"); // debug
                }
                else
                {
                    fprintf(logFile, "currProcess to assign to a core: %c\n", currProcess->pid); // debug
                    if (currProcess->arrival_time > currentTime)
                    {
                        fprintf(logFile, "process %c has not arrived yet\n", currProcess->pid);   // debug
                        fprintf(logFile, "process arrival time %d\n", currProcess->arrival_time); // debug
                    }
                    else
                    {
                        // find the core with the least ready queue size
                        int leastCore = 9999, leastCoreIndex = -1;
                        for (int j = 0; j < numCores; j++)
                        {
                            int readyQueueSize = get_queue_size(READY, j);
                            if (readyQueueSize < leastCore)
                            {
                                leastCore = readyQueueSize;
                                leastCoreIndex = j;
                            }
                            fprintf(logFile, "core %d ready queue size %d\n", j, readyQueueSize); // debug
                        }
                        fprintf(logFile, "process %c moved to ready at time %d\n", currProcess->pid, currentTime); // debug
                        // dequeue process from processes queue and enqueue it in the ready queue of the first available core
                        coreIndex = leastCoreIndex;

                        if (dequeue_process(READY, currProcess, numCores) == PROCESS_DEQUEUE_FAILED)
                        {
                            fprintf(logFile, "dequeue failed\n"); // debug
                            continue;
                        }
                        currProcess->process_status = READY;
                        enqueue_process(*currProcess, coreIndex);
                        fprintf(logFile, "process %c enqueued successfully to ready queue on core %d\n", currProcess->pid, coreIndex); // debug
                        i--;
                        continue;
                    }
                }

                // Return the process to its original core from the general run queue

                if (peek_process(RUN, &currProcess, numCores) == PROCESS_DEQUEUE_FAILED)
                {
                    fprintf(logFile, "empty general running quee\n"); // debug
                }
                else
                {
                    fprintf(logFile, "currProcess to return to the core: %c\n", currProcess->pid); // debug
                    if (currProcess->current_core == i)
                    {
                        fprintf(logFile, "process %c moved back to ready of the core at time %d\n", currProcess->pid, currentTime); // debug
                        // dequeue process from processes queue and enqueue it in the ready queue of the first available core
                        coreIndex = i;

                        if (dequeue_process(RUN, currProcess, numCores) == PROCESS_DEQUEUE_FAILED)
                        {
                            fprintf(logFile, "dequeue failed\n"); // debug
                            continue;
                        }
                        currProcess->process_status = READY;
                        enqueue_process_front(*currProcess, coreIndex);
                        fprintf(logFile, "process %c enqueued successfully to ready queue on core %d\n", currProcess->pid, coreIndex); // debug
                        i--;
                        continue;
                    }
                    else
                    {
                        fprintf(logFile, "process %c is not on the core %d\n", currProcess->pid, i); // debug
                    }
                }

                if (check_queue_empty(READY, i) == QUEUE_NOT_EMPTY)
                {
                    fprintf(logFile, "ready queue of core %d is not empty\n", i); // debug

                    // dequeue the process with the least priority from the ready queue of the core
                    dequeue_process(READY, currProcess, i);
                    if (currProcess->burst_info[currProcess->next_burst].burst_type != 'I')
                    {
                        enqueue_process_front(*currProcess, i);
                        int leastPriority = get_least_priority_process(READY, i);
                        if (get_process_with_property(READY, currProcess, i, priority_match, &leastPriority) == PROCESS_DEQUEUE_FAILED)
                        {
                            fprintf(logFile, "dequeue failed from ready of core %d\n", i); // debug
                            continue;
                        }
                    }

                    // check if the process is comming from blocked queue and has finished execution
                    if (currProcess->waiting_time == 9999)
                    {
                        fprintf(logFile, "process %c came from IO and finished\n", currProcess->pid); // debug
                        setProcessTurnaroundTime(result, getProcessIndex(result, currProcess->pid), currProcess->execution_time);
                        i--;
                        continue;
                    }
                    // check if the process is comming from blocked queue and didnt finish yet
                    if (currProcess->waiting_time > 0)
                    {
                        fprintf(logFile, "process %c was ready since %d\n", currProcess->pid, currProcess->waiting_time); // debug
                        setProcessStatus(result, getProcessIndex(result, currProcess->pid), currentTime - currProcess->waiting_time, READY);
                        currProcess->waiting_time = 0;
                    }
                    currProcess->process_status = RUN;
                    enqueue_process(*currProcess, i);

                    // check if the proecces runs for the first time and set its first run time
                    int processIndex = getProcessIndex(result, currProcess->pid);
                    if (result->processesAnalysis[processIndex].firstRunTime == -1)
                    {
                        result->processesAnalysis[processIndex].firstRunTime = currentTime;
                        setProcessResponseTime(result, processIndex, currentTime);
                        setProcessStatus(result, processIndex, result->processesAnalysis[processIndex].firstRunTime - result->processesAnalysis[processIndex].arrivalTime, READY);
                        fprintf(logFile, "process %c is running for the first time on core %d\n", currProcess->pid, i); // debug
                    }
                    fprintf(logFile, "process %c enqueued successfully to run on core %d\n", currProcess->pid, coreIndex); // debug
                    i--;
                    continue;
                }
                // if the core is not busy and the processes queue is empty or first process has not arrived yet, search for a process in the ready queue of other cores
                fprintf(logFile, "searching for a process in the ready queue of other cores\n"); // debug
                int found = 0;
                // find the least current burst time process in the ready queue of other cores
                int leastCore = 9999, leastPriority = 9999, leastCoreIndex = -1;
                for (int j = 0; j < numCores; j++)
                {
                    if (j == i)
                    {
                        continue;
                    }
                    if (check_queue_empty(READY, j) == QUEUE_NOT_EMPTY)
                    {
                        fprintf(logFile, "ready queue of core %d is not empty\n", j); // debug
                        leastPriority = get_least_priority_process(READY, j);
                        if (leastPriority < leastCore)
                        {
                            leastCore = leastPriority;
                            leastCoreIndex = j;
                            found = 1;
                        }
                    }
                }
                
                if (found == 1)
                {
                    fprintf(logFile, "found a process in the ready queue of core %d\n", leastCoreIndex); // debug
                    if (get_process_with_property(READY, currProcess, leastCoreIndex, priority_match, &leastPriority) == PROCESS_DEQUEUE_FAILED)
                    {
                        fprintf(logFile, "dequeue failed from ready of core %d\n", leastCoreIndex); // debug
                        continue;
                    }
                    currProcess->process_status = READY;
                    enqueue_process(*currProcess, i);
                    fprintf(logFile, "process %c enqueued successfully to run on core %d\n", currProcess->pid, i); // debug
                    i--;
                    continue;
                }
                else
                {
                    // if no process is found, schedule an idle process
                    fprintf(logFile, "no processes to run on core %d\n", i); // debug
                    scheduleProcess(result, i, 'i', currentTime, currentTime + 1);
                }
            }
            else
            {
                fprintf(logFile, "core %d start of running\n", i); // debug

                peek_process(RUN, &currProcess, i);
                char processId = currProcess->pid;
                int processIndex = getProcessIndex(result, processId);
                ProcessInfo *process = &(values->processes[processIndex]);

                BurstInfo *burst = &(process->bursts[currProcess->next_burst]);

                if (strcmp(burst->type, "CPU") == 0)
                {
                    setProcessStatus(result, processIndex, 1, RUN);
                    scheduleProcess(result, i, processId, currentTime, currentTime + 1);
                    fprintf(logFile, "process %c is running on core %d cpu\n", processId, i); // debug
                    burst->duration--;
                    currProcess->burst_info[currProcess->next_burst].execution_time--;
                }
                else
                {
                    fprintf(logFile, "process %c made IO on core %d\n", processId, i); // debug
                    setProcessStatus(result, processIndex, burst->duration, BLOCKED);
                    currProcess->next_burst++;
                    process->burstsCount--;
                    dequeue_process(RUN, currProcess, i);
                    currProcess->original_core = i;
                    currProcess->current_core = -1;
                    currProcess->process_status = BLOCKED;
                    currProcess->waiting_time = burst->duration;
                    enqueue_process(*currProcess, numCores);
                    i--;
                    continue;
                }

                if (burst->duration == 0)
                {
                    fprintf(logFile, "process %c burst completed\n", processId); // debug
                    currProcess->next_burst++;
                    process->burstsCount--;
                    if (process->burstsCount == 0)
                    {
                        fprintf(logFile, "process %c completed\n", processId); // debug
                        setProcessTurnaroundTime(result, processIndex, currentTime + 1);
                        dequeue_process(RUN, currProcess, i);
                    }
                    else
                    {
                        dequeue_process(RUN, currProcess, i);
                        currProcess->process_status = RUN;
                        currProcess->current_core = i;
                        currProcess->waiting_time = currentTime + 1;
                        enqueue_process(*currProcess, numCores);
                    }
                }
                else
                {
                    dequeue_process(RUN, currProcess, i);
                    currProcess->process_status = RUN;
                    currProcess->current_core = i;
                    currProcess->waiting_time = currentTime + 1;
                    enqueue_process(*currProcess, numCores);
                }
                fprintf(logFile, "core %d end of running\n", i); // debug
            }
            fprintf(logFile, "core %d end of checking\n", i); // debug
        }
        currentTime++;
        update_waiting_time(currentTime, numCores);
        fprintf(logFile, "end of iteration\n\n"); // debug
    }
    free(currProcess);
    currProcess = NULL;

    // Calculate and set average turnaround time and average response time
    double totalTurnaroundTime = 0, totalResponseTime = 0;
    for (int i = 0; i < result->processesCount; i++)
    {
        totalTurnaroundTime += result->processesAnalysis[i].turnaroundTime;
        totalResponseTime += result->processesAnalysis[i].responseTime;
    }
    setAvgTurnaroundTime(result, totalTurnaroundTime / result->processesCount);
    setAvgResponseTime(result, totalResponseTime / result->processesCount);
    fprintf(logFile, "///////////////////////////// end of Priority 2 Simulation //////////////////////////\n"); // debug
}
void simulatePriority3(ProcessedValues *values, SimulationResult *result, FILE *logFile)
{
    fprintf(logFile, "///////////////////////////// start of Priority3 Simulation //////////////////////////\n"); // debug
    int numCores = values->numCores;
    // ready queue of the extra set of queues is used to store the processes in the order of their priority
    // Only using the run queue of the cores. The blocked and ready queue is not used
    init_processes_queues(numCores + 1);
    int coreIndex = 0;
    int currentTime = 0;
    int arrived = 0;
    fprintf(logFile, "numCores %d\n", numCores); // debug

    fprintf(logFile, "processes enqueed successfully \n"); // debug
    queue_capacity_status_e status = check_queue_empty(READY, numCores);
    fprintf(logFile, "status %d\n\n", status); // debug

    while (1)
    {
        fprintf(logFile, "start of iteration\n");           // debug
        fprintf(logFile, "current time %d\n", currentTime); // debug
        // get number of busy cores
        int busyCores = 0;
        for (int i = 0; i < numCores; i++)
        {
            if (check_queue_empty(RUN, i) == QUEUE_NOT_EMPTY)
            {
                fprintf(logFile, "core %d is busy\n", i); // debug
                busyCores++;
            }
            else
            {
                fprintf(logFile, "core %d is not busy\n", i); // debug
            }
        }
        // enqueue the arrived processes in ready queue based on priority
        for (int i = 0; i < values->processesCount; i++)
        {
            if (values->processes[i].arrivalTime == currentTime)
            {
                char processId = values->processes[i].id;
                int processIndex = getProcessIndex(result, processId);
                process_info_t *pinfo = fill_process_info(values, processIndex);
                if (result->processesAnalysis[processIndex].firstRunTime == -1)
                {
                    fprintf(logFile, "process %c status %d\n\n", pinfo->pid, pinfo->process_status); // debug
                    pinfo->process_status = READY;
                    enqueue_process_priority(*pinfo, numCores);
                    fprintf(logFile, "process %c status %d\n\n", pinfo->pid, pinfo->process_status); // debug
                    arrived++;
                }
            }
        }

        // if all are not busy and processes queue is empty, break
        if (arrived == values->processesCount && busyCores == 0 && check_queue_empty(READY, numCores) == QUEUE_EMPTY)
        {
            fprintf(logFile, "all cores are not busy and processes queue is empty\n"); // debug
            break;
        }

        fprintf(logFile, "busy cores %d\n", busyCores); // debug

        // check if any core is available to run a process
        for (int i = 0; i < numCores; i++)
        {
            if (check_queue_empty(RUN, i) == QUEUE_EMPTY)
            {
                // if a core is available, peek the first process in the processes queue and enqueue it in the run queue of the core if its arrival time is less than or equal to the current time
                process_info_t *currProcess = (process_info_t *)malloc(sizeof(process_info_t));
                if (peek_process(READY, &currProcess, numCores) == PROCESS_DEQUEUE_FAILED)
                {
                    fprintf(logFile, "empty process quee\n"); // debug
                    continue;
                }
                fprintf(logFile, "currProcess to assign to a core: %c\n", currProcess->pid); // debug
                if (currProcess->arrival_time > currentTime)
                {
                    fprintf(logFile, "process %c not arrived yet\n", currProcess->pid);       // debug
                    fprintf(logFile, "process arrival time %d\n", currProcess->arrival_time); // debug
                    fprintf(logFile, "so we breaking\n");                                     // debug
                    break;
                }
                else
                {
                    fprintf(logFile, "process %c arrived at time %d\n", currProcess->pid, currentTime); // debug
                    // dequeue process from processes queue and enqueue it in the ready queue of the first available core
                    coreIndex = i;
                    process_info_t *currProcess = (process_info_t *)malloc(sizeof(process_info_t));
                    if (dequeue_process(READY, currProcess, numCores) == PROCESS_DEQUEUE_FAILED)
                    {
                        fprintf(logFile, "dequeue failed\n"); // debug
                        continue;
                    }
                    currProcess->process_status = RUN;
                    enqueue_process(*currProcess, coreIndex);
                    int processIndex = getProcessIndex(result, currProcess->pid);
                    result->processesAnalysis[processIndex].firstRunTime = currentTime;
                    setProcessResponseTime(result, processIndex, currentTime);
                    setProcessStatus(result, processIndex, result->processesAnalysis[processIndex].firstRunTime - result->processesAnalysis[processIndex].arrivalTime, READY);
                    fprintf(logFile, "process %c enqueued successfully to run on core %d\n", currProcess->pid, coreIndex); // debug
                }
            }
        }

        // iterate over all cores and run the process in each core for 1 time unit
        for (int i = 0; i < numCores; i++)
        {
            // check if the run queue of the core is empty
            if (check_queue_empty(RUN, i) == QUEUE_EMPTY)
            {
                fprintf(logFile, "run queue of core %d is empty\n", i); // debug
                scheduleProcess(result, i, 'i', currentTime, currentTime + 1);
                continue;
            }
            else
            {
                fprintf(logFile, "core %d start of running\n", i); // debug
                process_info_t *currProcess = (process_info_t *)malloc(sizeof(process_info_t));
                peek_process(RUN, &currProcess, i);
                char processId = currProcess->pid;
                int processIndex = getProcessIndex(result, processId);
                ProcessInfo *process = &(values->processes[processIndex]);

                BurstInfo *burst = &(process->bursts[currProcess->next_burst]);

                if (strcmp(burst->type, "CPU") == 0)
                {
                    setProcessStatus(result, processIndex, 1, RUN);
                    scheduleProcess(result, i, processId, currentTime, currentTime + 1);
                    fprintf(logFile, "process %c is running on core %d cpu\n", processId, i); // debug
                    burst->duration--;
                }
                else
                {
                    setProcessStatus(result, processIndex, 1, BLOCKED);
                    scheduleProcess(result, i, 'i', currentTime, currentTime + 1);
                    fprintf(logFile, "process %c is running on core %d io\n", processId, i); // debug
                    burst->duration--;
                }

                if (burst->duration == 0)
                {
                    fprintf(logFile, "process %c burst completed\n", processId); // debug
                    currProcess->next_burst++;
                    process->burstsCount--;
                    if (process->burstsCount == 0)
                    {
                        fprintf(logFile, "process %c completed\n", processId); // debug
                        setProcessTurnaroundTime(result, processIndex, currentTime + 1);
                        dequeue_process(RUN, currProcess, i);
                    }
                }
                fprintf(logFile, "core %d end of running\n", i); // debug
            }
        }
        currentTime++;
        fprintf(logFile, "end of iteration\n\n"); // debug
    }

    // Calculate and set average turnaround time and average response time
    double totalTurnaroundTime = 0, totalResponseTime = 0;
    for (int i = 0; i < result->processesCount; i++)
    {
        totalTurnaroundTime += result->processesAnalysis[i].turnaroundTime;
        totalResponseTime += result->processesAnalysis[i].responseTime;
    }
    setAvgTurnaroundTime(result, totalTurnaroundTime / result->processesCount);
    setAvgResponseTime(result, totalResponseTime / result->processesCount);
    fprintf(logFile, "///////////////////////////// end of Priority3 Simulation //////////////////////////\n"); // debug
}
void simulatePriority4(ProcessedValues *values, SimulationResult *result, FILE *logFile)
{
    fprintf(logFile, "///////////////////////////// start of Priority4 Simulation //////////////////////////\n"); // debug
    int numCores = values->numCores;
    // ready queue of the extra set of queues is used to store the processes in the order of their priority
    // Only using the run queue of the cores. The blocked and ready queue is not used
    init_processes_queues(numCores + 1);
    int coreIndex = 0;
    int currentTime = 0;
    int arrived = 0;
    int ff = 0;
    fprintf(logFile, "numCores %d\n", numCores); // debug

    process_info_t *currProcess = (process_info_t *)malloc(sizeof(process_info_t));
    process_info_t *check = (process_info_t *)malloc(sizeof(process_info_t));
    fprintf(logFile, "processes enqueed successfully \n"); // debug
    queue_capacity_status_e status = check_queue_empty(READY, numCores);
    fprintf(logFile, "status %d\n\n", status); // debug

    while (1)
    {
        fprintf(logFile, "start of iteration\n");           // debug
        fprintf(logFile, "current time %d\n", currentTime); // debug
        // get number of busy cores
        int busyCores = 0;
        for (int i = 0; i < numCores; i++)
        {
            if (check_queue_empty(RUN, i) == QUEUE_NOT_EMPTY)
            {
                fprintf(logFile, "core %d is busy\n", i); // debug
                busyCores++;
            }
            else
            {
                fprintf(logFile, "core %d is not busy\n", i); // debug
            }
        }
        // enqueue the arrived processes in ready queue based on priority
        if (ff == 0)
        {
            for (int i = 0; i < values->processesCount; i++)
            {
                if (values->processes[i].arrivalTime == currentTime)
                {
                    char processId = values->processes[i].id;
                    int processIndex = getProcessIndex(result, processId);
                    process_info_t *pinfo = fill_process_info(values, processIndex);
                    if (result->processesAnalysis[processIndex].firstRunTime == -1)
                    {
                        fprintf(logFile, "process %c status %d\n\n", pinfo->pid, pinfo->process_status); // debug
                        pinfo->process_status = READY;
                        enqueue_process_priority(*pinfo, numCores);
                        fprintf(logFile, "process %c status %d\n\n", pinfo->pid, pinfo->process_status); // debug
                        arrived++;
                    }
                }
            }
        }
        ff = 0;
        // if all are not busy and processes queue is empty, break
        if (arrived == values->processesCount && busyCores == 0 && check_queue_empty(READY, numCores) == QUEUE_EMPTY)
        {
            fprintf(logFile, "all cores are not busy and processes queue is empty\n"); // debug
            break;
        }

        fprintf(logFile, "busy cores %d\n", busyCores); // debug

        // check if any core is available to run a process
        if (peek_process(READY, &currProcess, numCores) == PROCESS_DEQUEUE_FAILED)
        {
            fprintf(logFile, "dequeue failed\n"); // debug
        }
        else
        {
            if ((currProcess->original_core) != -1)
            {
                if (check_queue_empty(RUN, currProcess->original_core) == QUEUE_EMPTY)
                {
                    fprintf(logFile, "%c original core empty\n", currProcess->pid);
                    fprintf(logFile, "process %c arrived at time %d\n", currProcess->pid, currentTime); // debug
                    // dequeue process from processes queue and enqueue it in the ready queue of the first available core
                    process_info_t *currProcess = (process_info_t *)malloc(sizeof(process_info_t));
                    if (dequeue_process(READY, currProcess, numCores) == PROCESS_DEQUEUE_FAILED)
                    {
                        fprintf(logFile, "dequeue failed\n"); // debug
                        continue;
                    }
                    currProcess->process_status = RUN;
                    enqueue_process(*currProcess, currProcess->original_core);
                    int processIndex = getProcessIndex(result, currProcess->pid);
                    setProcessStatus(result, processIndex, currentTime - currProcess->waiting_time, READY);
                    currProcess->waiting_time = 0;
                    currProcess->original_core = -1;
                    fprintf(logFile, "process %c enqueued successfully to run on core line 883 %d\n", currProcess->pid, currProcess->original_core); // debug
                    peek_process(READY, &currProcess, numCores);
                    fprintf(logFile, "the process in ready queue is %c\n", currProcess->pid);
                    ff = 1;
                }
            }
            if (ff == 0)
            {
                for (int i = 0; i < numCores; i++)
                {
                    if (check_queue_empty(RUN, i) == QUEUE_EMPTY)
                    {
                        // if a core is available, peek the first process in the processes queue and enqueue it in the run queue of the core if its arrival time is less than or equal to the current time
                        if (dequeue_process(READY, currProcess, numCores) == PROCESS_DEQUEUE_FAILED)
                        {
                            fprintf(logFile, "empty process quee\n"); // debug
                            continue;
                        }
                        fprintf(logFile, "currProcess to assign to a core: %c\n", currProcess->pid); // debug
                        if (currProcess->arrival_time > currentTime)
                        {
                            fprintf(logFile, "process %c not arrived yet\n", currProcess->pid);       // debug
                            fprintf(logFile, "process arrival time %d\n", currProcess->arrival_time); // debug
                            fprintf(logFile, "so we breaking\n");                                     // debug
                            break;
                        }
                        else
                        {
                            fprintf(logFile, "%c other core empty\n", currProcess->pid);
                            fprintf(logFile, "process %c arrived at time %d\n", currProcess->pid, currentTime); // debug
                            // dequeue process from processes queue and enqueue it in the ready queue of the first available core
                            coreIndex = i;
                            currProcess->process_status = RUN;
                            enqueue_process(*currProcess, coreIndex);
                            if (currProcess->original_core != -1)
                            {
                                setProcessStatus(result, getProcessIndex(result, currProcess->pid), currentTime - currProcess->waiting_time, READY);
                                currProcess->waiting_time = 0;
                                currProcess->original_core = -1;
                            }
                            int processIndex = getProcessIndex(result, currProcess->pid);
                            fprintf(logFile, "process %c enqueued to run on core line 916%d\n", currProcess->pid, coreIndex); // debug
                            if (result->processesAnalysis[processIndex].firstRunTime == -1)
                            {
                                result->processesAnalysis[processIndex].firstRunTime = currentTime;
                                setProcessResponseTime(result, processIndex, currentTime);
                                setProcessStatus(result, processIndex, result->processesAnalysis[processIndex].firstRunTime - result->processesAnalysis[processIndex].arrivalTime, READY);
                            }
                            fprintf(logFile, "process %c enqueued successfully to run on core line 922 %d\n", currProcess->pid, coreIndex); // debug
                            peek_process(READY, &currProcess, numCores);
                            fprintf(logFile, "the process in ready queue is %c\n", currProcess->pid);
                            ff = 1;
                            break;
                        }
                    }
                }
            }
            else
            {
                continue;
            }
            if (ff == 0)
            {
                if (currProcess->original_core != -1)
                {
                    if (peek_process(READY, &currProcess, numCores) == PROCESS_DEQUEUE_FAILED)
                    {
                        fprintf(logFile, "dequeue failed\n"); // debug
                    }
                    peek_process(RUN, &check, currProcess->original_core);
                    if (currProcess->priority < check->priority)
                    {
                        fprintf(logFile, "%c original core less priority\n", currProcess->pid);
                        if (dequeue_process(RUN, check, currProcess->original_core) == PROCESS_DEQUEUE_FAILED)
                        {
                            fprintf(logFile, "dequeue failed from ready of core %d\n", numCores); // debug
                        }
                        check->process_status = READY;
                        check->original_core = currProcess->original_core;
                        check->waiting_time = currentTime;
                        enqueue_process_priority(*check, numCores);

                        if (dequeue_process(READY, currProcess, numCores) == PROCESS_DEQUEUE_FAILED)
                        {
                            fprintf(logFile, "dequeue failed from ready of core %d\n", numCores); // debug
                        }
                        currProcess->process_status = RUN;
                        enqueue_process(*currProcess, currProcess->original_core);
                        setProcessStatus(result, getProcessIndex(result, currProcess->pid), currentTime - currProcess->waiting_time, READY);
                        currProcess->waiting_time = 0;
                        currProcess->original_core = -1;
                        ff = 1;
                    }
                }
            }
            else
            {
                continue;
            }
            if (ff == 0)
            {
                for (int i = 0; i < numCores; i++)
                {
                    process_info_t *currProcess = (process_info_t *)malloc(sizeof(process_info_t));
                    process_info_t *check = (process_info_t *)malloc(sizeof(process_info_t));
                    if (peek_process(READY, &currProcess, numCores) == PROCESS_DEQUEUE_FAILED)
                    {
                        fprintf(logFile, "dequeue failed\n"); // debug
                    }
                    peek_process(RUN, &check, i);
                    if (currProcess->priority < check->priority)
                    {
                        fprintf(logFile, "%c other core less priority\n", currProcess->pid);
                        if (dequeue_process(RUN, check, i) == PROCESS_DEQUEUE_FAILED)
                        {
                            fprintf(logFile, "dequeue failed from ready of core %d\n", numCores); // debug
                        }
                        fprintf(logFile, "%c in ready queue, %c in running for %d\n", check->pid, currProcess->pid, i);
                        check->process_status = READY;
                        check->original_core = i;
                        check->waiting_time = currentTime;
                        enqueue_process_priority(*check, numCores);
                        if (dequeue_process(READY, currProcess, numCores) == PROCESS_DEQUEUE_FAILED)
                        {
                            fprintf(logFile, "dequeue failed from ready of core %d\n", numCores); // debug
                        }
                        fprintf(logFile, "%d\n", check_queue_empty(RUN, i));
                        currProcess->process_status = RUN;
                        enqueue_process(*currProcess, i);
                        fprintf(logFile, "%c in ready queue, %c in running for %d\n", check->pid, currProcess->pid, i);
                        if (currProcess->original_core != -1)
                        {
                            setProcessStatus(result, getProcessIndex(result, currProcess->pid), currentTime - currProcess->waiting_time, READY);
                            currProcess->waiting_time = 0;
                            currProcess->original_core = -1;
                        }
                        if (result->processesAnalysis[getProcessIndex(result, currProcess->pid)].firstRunTime == -1)
                        {
                            result->processesAnalysis[getProcessIndex(result, currProcess->pid)].firstRunTime = currentTime;
                            setProcessResponseTime(result, getProcessIndex(result, currProcess->pid), currentTime);
                            setProcessStatus(result, getProcessIndex(result, currProcess->pid), result->processesAnalysis[getProcessIndex(result, currProcess->pid)].firstRunTime - result->processesAnalysis[getProcessIndex(result, currProcess->pid)].arrivalTime, READY);
                        }
                        ff = 1;
                        break;
                    }
                }
            }
            else
            {
                continue;
            }
            if (ff == 1)
            {
                continue;
            }
        }
        // iterate over all cores and run the process in each core for 1 time unit
        for (int i = 0; i < numCores; i++)
        {
            // check if the run queue of the core is empty
            if (check_queue_empty(RUN, i) == QUEUE_EMPTY)
            {
                fprintf(logFile, "run queue of core %d is empty\n", i); // debug
                scheduleProcess(result, i, 'i', currentTime, currentTime + 1);
                continue;
            }
            else
            {
                fprintf(logFile, "core %d start of running\n", i); // debug
                peek_process(RUN, &currProcess, i);
                char processId = currProcess->pid;
                int processIndex = getProcessIndex(result, processId);
                ProcessInfo *process = &(values->processes[processIndex]);

                BurstInfo *burst = &(process->bursts[currProcess->next_burst]);

                if (strcmp(burst->type, "CPU") == 0)
                {
                    setProcessStatus(result, processIndex, 1, RUN);
                    scheduleProcess(result, i, processId, currentTime, currentTime + 1);
                    fprintf(logFile, "process %c is running on core %d cpu\n", processId, i); // debug
                    burst->duration--;
                }
                else
                {
                    setProcessStatus(result, processIndex, 1, BLOCKED);
                    scheduleProcess(result, i, 'i', currentTime, currentTime + 1);
                    fprintf(logFile, "process %c is running on core %d io\n", processId, i); // debug
                    burst->duration--;
                }

                if (burst->duration == 0)
                {
                    fprintf(logFile, "process %c burst completed\n", processId); // debug
                    currProcess->next_burst++;
                    process->burstsCount--;
                    if (process->burstsCount == 0)
                    {
                        fprintf(logFile, "process %c completed\n", processId); // debug
                        setProcessTurnaroundTime(result, processIndex, currentTime + 1);
                        dequeue_process(RUN, currProcess, i);
                    }
                }
                fprintf(logFile, "core %d end of running\n", i); // debug
            }
        }
        currentTime++;
        fprintf(logFile, "end of iteration\n\n"); // debug
    }

    // Calculate and set average turnaround time and average response time
    double totalTurnaroundTime = 0, totalResponseTime = 0;
    for (int i = 0; i < result->processesCount; i++)
    {
        totalTurnaroundTime += result->processesAnalysis[i].turnaroundTime;
        totalResponseTime += result->processesAnalysis[i].responseTime;
    }
    setAvgTurnaroundTime(result, totalTurnaroundTime / result->processesCount);
    setAvgResponseTime(result, totalResponseTime / result->processesCount);
    fprintf(logFile, "///////////////////////////// end of Priority4 Simulation //////////////////////////\n"); // debug
}
void simulateStride(ProcessedValues *values, SimulationResult *result, FILE *logFile)
{
    fprintf(logFile, "///////////////////////////// start of Stride Simulation //////////////////////////\n"); // debug
    int numCores = values->numCores;
    // ready queue of the extra set of queues is used to store the processes in the order they arrived
    // blocked queue of the extra set of queues is used to store the processes that are waiting for IO
    // Only using the run and ready queue of the cores. The blocked queue is not used
    init_processes_queues(numCores + 1);
    int coreIndex = 0;
    int currentTime = 0;
    fprintf(logFile, "numCores %d\n", numCores); // debug

    // Iterate over each process in the order they arrived and enqueue them in the processes queue
    for (int i = 0; i < values->processesCount; i++)
    {
        char processId = values->processes[i].id;
        int processIndex = getProcessIndex(result, processId);
        process_info_t *pinfo = fill_process_info(values, processIndex);
        pinfo->process_status = READY;
        enqueue_process(*pinfo, numCores);
    }
    fprintf(logFile, "processes enqueed successfully \n"); // debug
    queue_capacity_status_e status = check_queue_empty(READY, numCores);
    fprintf(logFile, "status %d\n\n", status); // debug

    process_info_t *currProcess = (process_info_t *)malloc(sizeof(process_info_t));
    while (1)
    {
        fprintf(logFile, "start of iteration\n");           // debug
        fprintf(logFile, "current time %d\n", currentTime); // debug
        // get number of busy cores
        int busyCores = 0;
        for (int i = 0; i < numCores; i++)
        {
            if (check_queue_empty(RUN, i) == QUEUE_NOT_EMPTY)
            {
                fprintf(logFile, "core %d is busy\n", i); // debug
                busyCores++;
            }
            else
            {
                if (check_queue_empty(BLOCKED, i) == QUEUE_NOT_EMPTY)
                {
                    fprintf(logFile, "core %d is not busy but has blocked processes\n", i); // debug
                    busyCores++;
                }
                else if (check_queue_empty(READY, i) == QUEUE_NOT_EMPTY)
                {
                    fprintf(logFile, "core %d is not busy but has ready processes\n", i); // debug
                    busyCores++;
                }
                else
                {
                    fprintf(logFile, "core %d is not busy\n", i); // debug
                }
            }
        }

        // if all are not busy and processes queue is empty, break
        if (busyCores == 0 && (check_queue_empty(READY, numCores) == QUEUE_EMPTY) && (check_queue_empty(BLOCKED, numCores) == QUEUE_EMPTY) && (check_queue_empty(RUN, numCores) == QUEUE_EMPTY))
        {
            fprintf(logFile, "all cores are not busy and processes queue and blocked queue is empty\n"); // debug
            break;
        }

        fprintf(logFile, "Either cores hasnt finished, or processes or blocked queue is not empty\n"); // debug

        // iterate over all cores and run the process in each core for 1 time unit
        for (int i = 0; i < numCores; i++)
        {
            fprintf(logFile, "core %d start of checking\n", i); // debug
            // check if the run queue of the core is empty
            if (check_queue_empty(RUN, i) == QUEUE_EMPTY)
            {
                fprintf(logFile, "run queue of core %d is empty\n", i); // debug

                fprintf(logFile, "checking if a new process has arrived\n"); // debug
                // if a core is available, peek the first process in the processes queue and enqueue it in the ready queue of the core if its arrival time is less than or equal to the current time
                if (peek_process(READY, &currProcess, numCores) == PROCESS_DEQUEUE_FAILED)
                {
                    fprintf(logFile, "empty processes quee\n"); // debug
                }
                else
                {
                    fprintf(logFile, "currProcess to assign to a core: %c\n", currProcess->pid); // debug
                    if (currProcess->arrival_time > currentTime)
                    {
                        fprintf(logFile, "process %c has not arrived yet\n", currProcess->pid);   // debug
                        fprintf(logFile, "process arrival time %d\n", currProcess->arrival_time); // debug
                    }
                    else
                    {
                        // find the core with the least ready queue size
                        int leastCore = 9999, leastCoreIndex = -1;
                        for (int j = 0; j < numCores; j++)
                        {
                            int readyQueueSize = get_queue_size(READY, j);
                            if (readyQueueSize < leastCore)
                            {
                                leastCore = readyQueueSize;
                                leastCoreIndex = j;
                            }
                            fprintf(logFile, "core %d ready queue size %d\n", j, readyQueueSize); // debug
                        }
                        fprintf(logFile, "process %c moved to ready at time %d\n", currProcess->pid, currentTime); // debug
                        // dequeue process from processes queue and enqueue it in the ready queue of the first available core
                        coreIndex = leastCoreIndex;
                        
                        if (dequeue_process(READY, currProcess, numCores) == PROCESS_DEQUEUE_FAILED)
                        {
                            fprintf(logFile, "dequeue failed\n"); // debug
                            continue;
                        }
                        currProcess->process_status = READY;
                        enqueue_process(*currProcess, coreIndex);
                        fprintf(logFile, "process %c enqueued successfully to ready queue on core %d\n", currProcess->pid, coreIndex); // debug
                        i--;
                        continue;
                    }
                }

                // Return the process to its original core from the general run queue

                if (peek_process(RUN, &currProcess, numCores) == PROCESS_DEQUEUE_FAILED)
                {
                    fprintf(logFile, "empty general running quee\n"); // debug
                }
                else
                {
                    fprintf(logFile, "currProcess to return to the core: %c\n", currProcess->pid); // debug
                    if (currProcess->current_core == i)
                    {
                        fprintf(logFile, "process %c moved back to ready of the core at time %d\n", currProcess->pid, currentTime); // debug
                        // dequeue process from processes queue and enqueue it in the ready queue of the first available core
                        coreIndex = i;

                        if (dequeue_process(RUN, currProcess, numCores) == PROCESS_DEQUEUE_FAILED)
                        {
                            fprintf(logFile, "dequeue failed\n"); // debug
                            continue;
                        }
                        currProcess->process_status = READY;
                        enqueue_process_front(*currProcess, coreIndex);
                        fprintf(logFile, "process %c enqueued successfully to ready queue on core %d\n", currProcess->pid, coreIndex); // debug
                        i--;
                        continue;
                    }
                    else
                    {
                        fprintf(logFile, "process %c is not on the core %d\n", currProcess->pid, i); // debug
                    }
                }

                if (check_queue_empty(READY, i) == QUEUE_NOT_EMPTY)
                {
                    fprintf(logFile, "ready queue of core %d is not empty\n", i); // debug
                    // dequeue the process with the least pass value from the ready queue of the core
                    int leastPassValue = get_least_pass_process(READY, i);
                    if (get_process_with_property(READY, currProcess, i, pass_match, &leastPassValue) == PROCESS_DEQUEUE_FAILED)
                    {
                        fprintf(logFile, "dequeue failed from ready of core %d\n", i); // debug
                        continue;
                    }

                    // check if the process is comming from blocked queue and has finished execution
                    if (currProcess->waiting_time == 9999)
                    {
                        fprintf(logFile, "process %c came from IO and finished\n", currProcess->pid); // debug
                        setProcessTurnaroundTime(result, getProcessIndex(result, currProcess->pid), currProcess->execution_time);
                        i--;
                        continue;
                    }
                    // check if the process is comming from blocked queue and didnt finish yet
                    if (currProcess->waiting_time > 0)
                    {
                        fprintf(logFile, "process %c was ready since %d\n", currProcess->pid, currProcess->waiting_time); // debug
                        setProcessStatus(result, getProcessIndex(result, currProcess->pid), currentTime - currProcess->waiting_time, READY);
                        currProcess->waiting_time = 0;
                    }
                    currProcess->process_status = RUN;
                    currProcess->pass += currProcess->stride;
                    enqueue_process(*currProcess, i);

                    // check if the proecces runs for the first time and set its first run time
                    int processIndex = getProcessIndex(result, currProcess->pid);
                    if (result->processesAnalysis[processIndex].firstRunTime == -1)
                    {
                        result->processesAnalysis[processIndex].firstRunTime = currentTime;
                        setProcessResponseTime(result, processIndex, currentTime);
                        setProcessStatus(result, processIndex, result->processesAnalysis[processIndex].firstRunTime - result->processesAnalysis[processIndex].arrivalTime, READY);
                        fprintf(logFile, "process %c is running for the first time on core %d\n", currProcess->pid, i); // debug
                    }
                    fprintf(logFile, "process %c enqueued successfully to run on core %d\n", currProcess->pid, coreIndex); // debug
                    i--;
                    continue;
                }
                // if the core is not busy and the processes queue is empty or first process has not arrived yet, search for a process in the ready queue of other cores
                fprintf(logFile, "searching for a process in the ready queue of other cores\n"); // debug
                int found = 0;
                // find the least current burst time process in the ready queue of other cores
                int leastCore = 9999, leastPassValue = 9999, leastCoreIndex = -1;
                for (int j = 0; j < numCores; j++)
                {
                    if (j == i)
                    {
                        continue;
                    }
                    if (check_queue_empty(READY, j) == QUEUE_NOT_EMPTY)
                    {
                        fprintf(logFile, "ready queue of core %d is not empty\n", j); // debug
                        leastPassValue = get_least_pass_process(READY, j);
                        if (leastPassValue < leastCore)
                        {
                            leastCore = leastPassValue;
                            leastCoreIndex = j;
                            found = 1;
                        }
                    }
                }
                
                if (found == 1)
                {
                    fprintf(logFile, "found a process in the ready queue of core %d\n", leastCoreIndex); // debug
                    if (get_process_with_property(READY, currProcess, leastCoreIndex, pass_match, &leastPassValue) == PROCESS_DEQUEUE_FAILED)
                    {
                        fprintf(logFile, "dequeue failed from ready of core %d\n", leastCoreIndex); // debug
                        continue;
                    }
                    currProcess->process_status = READY;
                    enqueue_process(*currProcess, i);
                    fprintf(logFile, "process %c enqueued successfully to run on core %d\n", currProcess->pid, i); // debug
                    i--;
                    continue;
                }
                else
                {
                    // if no process is found, schedule an idle process
                    fprintf(logFile, "no processes to run on core %d\n", i); // debug
                    scheduleProcess(result, i, 'i', currentTime, currentTime + 1);
                }
            }
            else
            {
                fprintf(logFile, "core %d start of running\n", i); // debug
                peek_process(RUN, &currProcess, i);
                char processId = currProcess->pid;
                int processIndex = getProcessIndex(result, processId);
                ProcessInfo *process = &(values->processes[processIndex]);

                BurstInfo *burst = &(process->bursts[currProcess->next_burst]);

                if (strcmp(burst->type, "CPU") == 0)
                {
                    setProcessStatus(result, processIndex, 1, RUN);
                    scheduleProcess(result, i, processId, currentTime, currentTime + 1);
                    fprintf(logFile, "process %c is running on core %d cpu\n", processId, i); // debug
                    burst->duration--;
                    currProcess->burst_info[currProcess->next_burst].execution_time--;
                }
                else
                {
                    fprintf(logFile, "process %c made IO on core %d\n", processId, i); // debug
                    setProcessStatus(result, processIndex, burst->duration, BLOCKED);
                    currProcess->next_burst++;
                    process->burstsCount--;
                    dequeue_process(RUN, currProcess, i);
                    currProcess->original_core = i;
                    currProcess->current_core = -1;
                    currProcess->process_status = BLOCKED;
                    currProcess->waiting_time = burst->duration;
                    enqueue_process(*currProcess, numCores);
                    i--;
                    continue;
                }

                if (burst->duration == 0)
                {
                    fprintf(logFile, "process %c burst completed\n", processId); // debug
                    currProcess->next_burst++;
                    process->burstsCount--;
                    if (process->burstsCount == 0)
                    {
                        fprintf(logFile, "process %c completed\n", processId); // debug
                        setProcessTurnaroundTime(result, processIndex, currentTime + 1);
                        dequeue_process(RUN, currProcess, i);
                    }
                    else
                    {
                        dequeue_process(RUN, currProcess, i);
                        currProcess->process_status = RUN;
                        currProcess->current_core = i;
                        currProcess->waiting_time = currentTime + 1;
                        enqueue_process(*currProcess, numCores);
                    }
                }
                else
                {
                    dequeue_process(RUN, currProcess, i);
                    currProcess->process_status = RUN;
                    currProcess->current_core = i;
                    currProcess->waiting_time = currentTime + 1;
                    enqueue_process(*currProcess, numCores);
                }
                fprintf(logFile, "core %d end of running\n", i); // debug
            }
            fprintf(logFile, "core %d end of checking\n", i); // debug
        }
        currentTime++;
        update_waiting_time(currentTime, numCores);
        fprintf(logFile, "end of iteration\n\n"); // debug
    }
    free(currProcess);
    currProcess = NULL;

    // Calculate and set average turnaround time and average response time
    double totalTurnaroundTime = 0, totalResponseTime = 0;
    for (int i = 0; i < result->processesCount; i++)
    {
        totalTurnaroundTime += result->processesAnalysis[i].turnaroundTime;
        totalResponseTime += result->processesAnalysis[i].responseTime;
    }
    setAvgTurnaroundTime(result, totalTurnaroundTime / result->processesCount);
    setAvgResponseTime(result, totalResponseTime / result->processesCount);
    fprintf(logFile, "///////////////////////////// end of Stride Simulation //////////////////////////\n"); // debug
}

void simulateLottery(ProcessedValues *values, SimulationResult *result, FILE *logFile)
{
    fprintf(logFile, "///////////////////////////// start of Lottery Simulation //////////////////////////\n"); // debug
    int numCores = values->numCores;
    // ready queue of the extra set of queues is used to store the processes in the order they arrived
    // blocked queue of the extra set of queues is used to store the processes that are waiting for IO
    // Only using the run and ready queue of the cores. The blocked queue is not used
    init_processes_queues(numCores + 1);
    int coreIndex = 0;
    int currentTime = 0;
    fprintf(logFile, "numCores %d\n", numCores); // debug

    // Iterate over each process in the order they arrived and enqueue them in the processes queue
    for (int i = 0; i < values->processesCount; i++)
    {
        char processId = values->processes[i].id;
        int processIndex = getProcessIndex(result, processId);
        process_info_t *pinfo = fill_process_info(values, processIndex);
        pinfo->process_status = READY;
        enqueue_process(*pinfo, numCores);
    }
    fprintf(logFile, "processes enqueed successfully \n"); // debug
    queue_capacity_status_e status = check_queue_empty(READY, numCores);
    fprintf(logFile, "status %d\n\n", status); // debug

    process_info_t *currProcess = (process_info_t *)malloc(sizeof(process_info_t));
    while (1)
    {
        fprintf(logFile, "start of iteration\n");           // debug
        fprintf(logFile, "current time %d\n", currentTime); // debug
        // get number of busy cores
        int busyCores = 0;
        for (int i = 0; i < numCores; i++)
        {
            if (check_queue_empty(RUN, i) == QUEUE_NOT_EMPTY)
            {
                fprintf(logFile, "core %d is busy\n", i); // debug
                busyCores++;
            }
            else
            {
                if (check_queue_empty(BLOCKED, i) == QUEUE_NOT_EMPTY)
                {
                    fprintf(logFile, "core %d is not busy but has blocked processes\n", i); // debug
                    busyCores++;
                }
                else if (check_queue_empty(READY, i) == QUEUE_NOT_EMPTY)
                {
                    fprintf(logFile, "core %d is not busy but has ready processes\n", i); // debug
                    busyCores++;
                }
                else
                {
                    fprintf(logFile, "core %d is not busy\n", i); // debug
                }
            }
        }

        // if all are not busy and processes queue is empty, break
        if (busyCores == 0 && (check_queue_empty(READY, numCores) == QUEUE_EMPTY) && (check_queue_empty(BLOCKED, numCores) == QUEUE_EMPTY) && (check_queue_empty(RUN, numCores) == QUEUE_EMPTY))
        {
            fprintf(logFile, "all cores are not busy and processes queue and blocked queue is empty\n"); // debug
            break;
        }

        fprintf(logFile, "Either cores hasnt finished, or processes or blocked queue is not empty\n"); // debug

        // iterate over all cores and run the process in each core for 1 time unit
        for (int i = 0; i < numCores; i++)
        {
            fprintf(logFile, "core %d start of checking\n", i); // debug
            // check if the run queue of the core is empty
            if (check_queue_empty(RUN, i) == QUEUE_EMPTY)
            {
                fprintf(logFile, "run queue of core %d is empty\n", i); // debug

                fprintf(logFile, "checking if a new process has arrived\n"); // debug
                // if a core is available, peek the first process in the processes queue and enqueue it in the ready queue of the core if its arrival time is less than or equal to the current time

                if (peek_process(READY, &currProcess, numCores) == PROCESS_DEQUEUE_FAILED)
                {
                    fprintf(logFile, "empty processes quee\n"); // debug
                }
                else
                {
                    fprintf(logFile, "currProcess to assign to a core: %c\n", currProcess->pid); // debug
                    if (currProcess->arrival_time > currentTime)
                    {
                        fprintf(logFile, "process %c has not arrived yet\n", currProcess->pid);   // debug
                        fprintf(logFile, "process arrival time %d\n", currProcess->arrival_time); // debug
                    }
                    else
                    {
                        fprintf(logFile, "process %c moved to ready at time %d\n", currProcess->pid, currentTime); // debug
                        // dequeue process from processes queue and enqueue it in the ready queue of the first available core
                        coreIndex = i;
                        
                        if (dequeue_process(READY, currProcess, numCores) == PROCESS_DEQUEUE_FAILED)
                        {
                            fprintf(logFile, "dequeue failed\n"); // debug
                            continue;
                        }
                        currProcess->process_status = READY;
                        enqueue_process(*currProcess, coreIndex);
                        fprintf(logFile, "process %c enqueued successfully to ready queue on core %d\n", currProcess->pid, coreIndex); // debug
                    }
                }

                if (check_queue_empty(READY, i) == QUEUE_NOT_EMPTY)
                {
                    fprintf(logFile, "ready queue of core %d is not empty\n", i); // debug

                    // dequeue the process with random id from the ready queue of the core
                    int randomProcessId = get_random_process_id(READY, i);
                    fprintf(logFile, "randomProcessId %d\n", randomProcessId); // debug
                    if (get_process_with_property(READY, currProcess, i, pid_match, &randomProcessId) == PROCESS_DEQUEUE_FAILED)
                    {
                        fprintf(logFile, "dequeue failed from ready of core %d\n", i); // debug
                        continue;
                    }

                    // check if the process is comming from blocked queue and has finished execution
                    if (currProcess->waiting_time == 9999)
                    {
                        fprintf(logFile, "process %c came from IO and finished\n", currProcess->pid); // debug
                        setProcessTurnaroundTime(result, getProcessIndex(result, currProcess->pid), currProcess->execution_time);
                        i--;
                        continue;
                    }
                    // check if the process is comming from blocked queue and didnt finish yet
                    if (currProcess->waiting_time > 0)
                    {
                        fprintf(logFile, "process %c was ready since %d\n", currProcess->pid, currProcess->waiting_time); // debug
                        setProcessStatus(result, getProcessIndex(result, currProcess->pid), currentTime - currProcess->waiting_time, READY);
                        currProcess->waiting_time = 0;
                    }
                    currProcess->process_status = RUN;
                    currProcess->pass += currProcess->stride;
                    enqueue_process(*currProcess, i);

                    // check if the proecces runs for the first time and set its first run time
                    int processIndex = getProcessIndex(result, currProcess->pid);
                    if (result->processesAnalysis[processIndex].firstRunTime == -1)
                    {
                        result->processesAnalysis[processIndex].firstRunTime = currentTime;
                        setProcessResponseTime(result, processIndex, currentTime);
                        setProcessStatus(result, processIndex, result->processesAnalysis[processIndex].firstRunTime - result->processesAnalysis[processIndex].arrivalTime, READY);
                        fprintf(logFile, "process %c is running for the first time on core %d\n", currProcess->pid, i); // debug
                    }
                    fprintf(logFile, "process %c enqueued successfully to run on core %d\n", currProcess->pid, coreIndex); // debug
                    i--;
                    continue;
                }
                // if the core is not busy and the processes queue is empty or first process has not arrived yet, search for a process in the ready queue of other cores
                fprintf(logFile, "searching for a process in the ready queue of other cores\n"); // debug
                int found = 0;
                for (int j = 0; j < numCores; j++)
                {
                    if (j == i)
                    {
                        continue;
                    }
                    if (check_queue_empty(READY, j) == QUEUE_NOT_EMPTY)
                    {
                        fprintf(logFile, "ready queue of core %d is not empty\n", j); // debug

                        // dequeue the process with random id from the ready queue of the core
                        int randomProcessId = get_random_process_id(READY, j);
                        if (get_process_with_property(READY, currProcess, j, pid_match, &randomProcessId) == PROCESS_DEQUEUE_FAILED)
                        {
                            fprintf(logFile, "dequeue failed from ready of core %d\n", j); // debug
                            continue;
                        }

                        currProcess->process_status = READY;
                        enqueue_process(*currProcess, i);
                        fprintf(logFile, "process %c enqueued successfully to run on core %d\n", currProcess->pid, i); // debug
                        found = 1;
                        break;
                    }
                }
                if (found == 0)
                {
                    // if no process is found, schedule an idle process
                    fprintf(logFile, "no processes to run on core %d\n", i); // debug
                    scheduleProcess(result, i, 'i', currentTime, currentTime + 1);
                }
                else
                {
                    i--;
                    continue;
                }
            }
            else
            {
                fprintf(logFile, "core %d start of running\n", i); // debug

                peek_process(RUN, &currProcess, i);
                char processId = currProcess->pid;
                int processIndex = getProcessIndex(result, processId);
                ProcessInfo *process = &(values->processes[processIndex]);

                BurstInfo *burst = &(process->bursts[currProcess->next_burst]);

                if (strcmp(burst->type, "CPU") == 0)
                {
                    setProcessStatus(result, processIndex, 1, RUN);
                    scheduleProcess(result, i, processId, currentTime, currentTime + 1);
                    fprintf(logFile, "process %c is running on core %d cpu\n", processId, i); // debug
                    burst->duration--;
                    currProcess->burst_info[currProcess->next_burst].execution_time--;
                }
                else
                {
                    fprintf(logFile, "process %c made IO on core %d\n", processId, i); // debug
                    setProcessStatus(result, processIndex, burst->duration, BLOCKED);
                    currProcess->next_burst++;
                    process->burstsCount--;
                    dequeue_process(RUN, currProcess, i);
                    currProcess->original_core = i;
                    currProcess->current_core = -1;
                    currProcess->process_status = BLOCKED;
                    currProcess->waiting_time = burst->duration;
                    enqueue_process(*currProcess, numCores);
                    i--;
                    continue;
                }

                if (burst->duration == 0)
                {
                    fprintf(logFile, "process %c burst completed\n", processId); // debug
                    currProcess->next_burst++;
                    process->burstsCount--;
                    if (process->burstsCount == 0)
                    {
                        fprintf(logFile, "process %c completed\n", processId); // debug
                        setProcessTurnaroundTime(result, processIndex, currentTime + 1);
                        dequeue_process(RUN, currProcess, i);
                    }
                    else
                    {
                        dequeue_process(RUN, currProcess, i);
                        currProcess->process_status = RUN;
                        currProcess->current_core = i;
                        currProcess->waiting_time = currentTime + 1;
                        enqueue_process(*currProcess, numCores);
                    }
                }
                else
                {
                    dequeue_process(RUN, currProcess, i);
                    currProcess->process_status = RUN;
                    currProcess->current_core = i;
                    currProcess->waiting_time = currentTime + 1;
                    enqueue_process(*currProcess, numCores);
                }
                fprintf(logFile, "core %d end of running\n", i); // debug
            }
            fprintf(logFile, "core %d end of checking\n", i); // debug
        }
        // loop through all process in the run of the extra set and enqueue them in the ready queue of the core they were running on
        while (check_queue_empty(RUN, numCores) == QUEUE_NOT_EMPTY)
        {
            if (dequeue_process(RUN, currProcess, numCores) == PROCESS_DEQUEUE_FAILED)
            {
                fprintf(logFile, "dequeue failed\n"); // debug
                continue;
            }
            currProcess->process_status = READY;
            enqueue_process(*currProcess, currProcess->current_core);
        }
        currentTime++;
        update_waiting_time(currentTime, numCores);
        fprintf(logFile, "end of iteration\n\n"); // debug
    }
    free(currProcess);
    currProcess = NULL;

    // Calculate and set average turnaround time and average response time
    double totalTurnaroundTime = 0, totalResponseTime = 0;
    for (int i = 0; i < result->processesCount; i++)
    {
        totalTurnaroundTime += result->processesAnalysis[i].turnaroundTime;
        totalResponseTime += result->processesAnalysis[i].responseTime;
    }
    setAvgTurnaroundTime(result, totalTurnaroundTime / result->processesCount);
    setAvgResponseTime(result, totalResponseTime / result->processesCount);
    fprintf(logFile, "///////////////////////////// end of Lottery Simulation //////////////////////////\n"); // debug
}

void simulateFIFO1(ProcessedValues *values, SimulationResult *result, FILE *logFile)
{
    fprintf(logFile, "///////////////////////////// start of FIFO1 Simulation //////////////////////////\n"); // debug
    int numCores = values->numCores;
    // ready queue of the extra set of queues is used to store the processes in the order they arrived
    // Only using the run queue of the cores. The blocked and ready queue is not used
    init_processes_queues(numCores + 1);
    int coreIndex = 0;
    int currentTime = 0;
    fprintf(logFile, "numCores %d\n", numCores); // debug

    // Iterate over each process in the order they arrived and enqueue them in the processes queue
    for (int i = 0; i < values->processesCount; i++)
    {
        char processId = values->processes[i].id;
        int processIndex = getProcessIndex(result, processId);
        process_info_t *pinfo = fill_process_info(values, processIndex);
        pinfo->process_status = READY;
        enqueue_process(*pinfo, numCores);
    }
    fprintf(logFile, "processes enqueed successfully \n"); // debug
    queue_capacity_status_e status = check_queue_empty(READY, numCores);
    fprintf(logFile, "status %d\n\n", status); // debug

    process_info_t *currProcess = (process_info_t *)malloc(sizeof(process_info_t));
    while (1)
    {
        fprintf(logFile, "start of iteration\n");           // debug
        fprintf(logFile, "current time %d\n", currentTime); // debug
        // get number of busy cores
        int busyCores = 0;
        for (int i = 0; i < numCores; i++)
        {
            if (check_queue_empty(RUN, i) == QUEUE_NOT_EMPTY)
            {
                fprintf(logFile, "core %d is busy\n", i); // debug
                busyCores++;
            }
            else
            {
                fprintf(logFile, "core %d is not busy\n", i); // debug
            }
        }

        // if all are not busy and processes queue is empty, break
        if (busyCores == 0 && check_queue_empty(READY, numCores) == QUEUE_EMPTY)
        {
            fprintf(logFile, "all cores are not busy and processes queue is empty\n"); // debug
            break;
        }

        fprintf(logFile, "busy cores %d\n", busyCores); // debug

        // check if any core is available to run a process
        for (int i = 0; i < numCores; i++)
        {
            if (check_queue_empty(RUN, i) == QUEUE_EMPTY)
            {
                // if a core is available, peek the first process in the processes queue and enqueue it in the run queue of the core if its arrival time is less than or equal to the current time
                if (peek_process(READY, &currProcess, numCores) == PROCESS_DEQUEUE_FAILED)
                {
                    fprintf(logFile, "empty process quee\n"); // debug
                    continue;
                }
                fprintf(logFile, "currProcess to assign to a core: %c\n", currProcess->pid); // debug
                if (currProcess->arrival_time > currentTime)
                {
                    fprintf(logFile, "process %c not arrived yet\n", currProcess->pid);       // debug
                    fprintf(logFile, "process arrival time %d\n", currProcess->arrival_time); // debug
                    fprintf(logFile, "so we breaking\n");                                     // debug
                    break;
                }
                else
                {
                    fprintf(logFile, "process %c arrived at time %d\n", currProcess->pid, currentTime); // debug
                    // dequeue process from processes queue and enqueue it in the ready queue of the first available core
                    coreIndex = i;
                    if (dequeue_process(READY, currProcess, numCores) == PROCESS_DEQUEUE_FAILED)
                    {
                        fprintf(logFile, "dequeue failed\n"); // debug
                        continue;
                    }
                    currProcess->process_status = RUN;
                    enqueue_process(*currProcess, coreIndex);
                    int processIndex = getProcessIndex(result, currProcess->pid);
                    result->processesAnalysis[processIndex].firstRunTime = currentTime;
                    setProcessResponseTime(result, processIndex, currentTime);
                    setProcessStatus(result, processIndex, result->processesAnalysis[processIndex].firstRunTime - result->processesAnalysis[processIndex].arrivalTime, READY);
                    fprintf(logFile, "process %c enqueued successfully to run on core %d\n", currProcess->pid, coreIndex); // debug
                }
            }
        }

        // iterate over all cores and run the process in each core for 1 time unit
        for (int i = 0; i < numCores; i++)
        {
            // check if the run queue of the core is empty
            if (check_queue_empty(RUN, i) == QUEUE_EMPTY)
            {
                fprintf(logFile, "run queue of core %d is empty\n", i); // debug
                scheduleProcess(result, i, 'i', currentTime, currentTime + 1);
                continue;
            }
            else
            {
                fprintf(logFile, "core %d start of running\n", i); // debug
                peek_process(RUN, &currProcess, i);
                char processId = currProcess->pid;
                int processIndex = getProcessIndex(result, processId);
                ProcessInfo *process = &(values->processes[processIndex]);

                BurstInfo *burst = &(process->bursts[currProcess->next_burst]);

                if (strcmp(burst->type, "CPU") == 0)
                {
                    setProcessStatus(result, processIndex, 1, RUN);
                    scheduleProcess(result, i, processId, currentTime, currentTime + 1);
                    fprintf(logFile, "process %c is running on core %d cpu\n", processId, i); // debug
                    burst->duration--;
                }
                else
                {
                    setProcessStatus(result, processIndex, 1, BLOCKED);
                    scheduleProcess(result, i, 'i', currentTime, currentTime + 1);
                    fprintf(logFile, "process %c is running on core %d io\n", processId, i); // debug
                    burst->duration--;
                }

                if (burst->duration == 0)
                {
                    fprintf(logFile, "process %c burst completed\n", processId); // debug
                    currProcess->next_burst++;
                    process->burstsCount--;
                    if (process->burstsCount == 0)
                    {
                        fprintf(logFile, "process %c completed\n", processId); // debug
                        setProcessTurnaroundTime(result, processIndex, currentTime + 1);
                        dequeue_process(RUN, currProcess, i);
                    }
                }
                fprintf(logFile, "core %d end of running\n", i); // debug
            }
        }
        currentTime++;
        fprintf(logFile, "end of iteration\n\n"); // debug
    }
    free(currProcess);
    currProcess = NULL;

    // Calculate and set average turnaround time and average response time
    double totalTurnaroundTime = 0, totalResponseTime = 0;
    for (int i = 0; i < result->processesCount; i++)
    {
        totalTurnaroundTime += result->processesAnalysis[i].turnaroundTime;
        totalResponseTime += result->processesAnalysis[i].responseTime;
    }
    setAvgTurnaroundTime(result, totalTurnaroundTime / result->processesCount);
    setAvgResponseTime(result, totalResponseTime / result->processesCount);
    fprintf(logFile, "///////////////////////////// end of FIFO1 Simulation //////////////////////////\n"); // debug
}

void simulateFIFO2(ProcessedValues *values, SimulationResult *result, FILE *logFile)
{
    fprintf(logFile, "///////////////////////////// start of FIFO2 Simulation //////////////////////////\n"); // debug
    int numCores = values->numCores;
    // ready queue of the extra set of queues is used to store the processes in the order they arrived
    // blocked queue of the extra set of queues is used to store the processes that are waiting for IO
    // run queue of the extra set of queues is not used
    // Only using the run and ready queue of the cores. The blocked queue is not used
    init_processes_queues(numCores + 1);
    int coreIndex = 0;
    int currentTime = 0;
    fprintf(logFile, "numCores %d\n", numCores); // debug

    // Iterate over each process in the order they arrived and enqueue them in the processes queue
    for (int i = 0; i < values->processesCount; i++)
    {
        char processId = values->processes[i].id;
        int processIndex = getProcessIndex(result, processId);
        process_info_t *pinfo = fill_process_info(values, processIndex);
        pinfo->process_status = READY;
        enqueue_process(*pinfo, numCores);
    }
    fprintf(logFile, "processes enqueed successfully \n"); // debug
    queue_capacity_status_e status = check_queue_empty(READY, numCores);
    fprintf(logFile, "status %d\n\n", status); // debug

    process_info_t *currProcess = (process_info_t *)malloc(sizeof(process_info_t));
    while (1)
    {
        fprintf(logFile, "start of iteration\n");           // debug
        fprintf(logFile, "current time %d\n", currentTime); // debug
        // get number of busy cores
        int busyCores = 0;
        for (int i = 0; i < numCores; i++)
        {
            if (check_queue_empty(RUN, i) == QUEUE_NOT_EMPTY)
            {
                fprintf(logFile, "core %d is busy\n", i); // debug
                busyCores++;
            }
            else
            {
                if (check_queue_empty(BLOCKED, i) == QUEUE_NOT_EMPTY)
                {
                    fprintf(logFile, "core %d is not busy but has blocked processes\n", i); // debug
                    busyCores++;
                }
                else if (check_queue_empty(READY, i) == QUEUE_NOT_EMPTY)
                {
                    fprintf(logFile, "core %d is not busy but has ready processes\n", i); // debug
                    busyCores++;
                }
                else
                {
                    fprintf(logFile, "core %d is not busy\n", i); // debug
                }
            }
        }

        // if all are not busy and processes queue is empty, break
        if (busyCores == 0 && (check_queue_empty(READY, numCores) == QUEUE_EMPTY) && (check_queue_empty(BLOCKED, numCores) == QUEUE_EMPTY))
        {
            fprintf(logFile, "all cores are not busy and processes queue and blocked queue is empty\n"); // debug
            break;
        }

        fprintf(logFile, "Either cores hasnt finished, or processes or blocked queue is not empty\n"); // debug

        // iterate over all cores and run the process in each core for 1 time unit
        for (int i = 0; i < numCores; i++)
        {
            fprintf(logFile, "core %d start of checking\n", i); // debug
            // check if the run queue of the core is empty
            if (check_queue_empty(RUN, i) == QUEUE_EMPTY)
            {
                fprintf(logFile, "run queue of core %d is empty\n", i); // debug

                if (check_queue_empty(READY, i) == QUEUE_NOT_EMPTY)
                {
                    fprintf(logFile, "ready queue of core %d is not empty\n", i); // debug
                    if (dequeue_process(READY, currProcess, i) == PROCESS_DEQUEUE_FAILED)
                    {
                        fprintf(logFile, "dequeue failed from ready of core %d\n", i); // debug
                    }

                    // check if the process is comming from blocked queue and has finished execution
                    if (currProcess->waiting_time == 9999)
                    {
                        fprintf(logFile, "process %c came from IO and finished\n", currProcess->pid); // debug
                        setProcessTurnaroundTime(result, getProcessIndex(result, currProcess->pid), currProcess->execution_time);
                        i--;
                        continue;
                    }

                    // check if the process is comming from blocked queue and didnt finish yet
                    if (currProcess->waiting_time > 0)
                    {
                        fprintf(logFile, "process %c came for IO and it was ready since %d\n", currProcess->pid, currProcess->waiting_time); // debug
                        setProcessStatus(result, getProcessIndex(result, currProcess->pid), currentTime - currProcess->waiting_time, READY);
                        currProcess->waiting_time = 0;
                    }
                    currProcess->process_status = RUN;
                    enqueue_process(*currProcess, i);

                    // check if the proecces runs for the first time and set its first run time
                    int processIndex = getProcessIndex(result, currProcess->pid);
                    if (result->processesAnalysis[processIndex].firstRunTime == -1)
                    {
                        result->processesAnalysis[processIndex].firstRunTime = currentTime;
                        setProcessResponseTime(result, processIndex, currentTime);
                        setProcessStatus(result, processIndex, result->processesAnalysis[processIndex].firstRunTime - result->processesAnalysis[processIndex].arrivalTime, READY);
                        fprintf(logFile, "process %c is running for the first time on core %d\n", currProcess->pid, i); // debug
                    }
                    fprintf(logFile, "process %c enqueued successfully to run on core %d\n", currProcess->pid, coreIndex); // debug
                    i--;
                    continue;
                }
                else
                {
                    fprintf(logFile, "ready queue of core %d is empty\n", i); // debug
                    // if a core is available, peek the first process in the processes queue and enqueue it in the run queue of the core if its arrival time is less than or equal to the current time
                    if (peek_process(READY, &currProcess, numCores) == PROCESS_DEQUEUE_FAILED)
                    {
                        fprintf(logFile, "empty processes quee\n"); // debug
                    }
                    else
                    {
                        fprintf(logFile, "currProcess to assign to a core: %c\n", currProcess->pid); // debug
                        if (currProcess->arrival_time > currentTime)
                        {
                            fprintf(logFile, "process %c has not arrived yet\n", currProcess->pid);   // debug
                            fprintf(logFile, "process arrival time %d\n", currProcess->arrival_time); // debug
                        }
                        else
                        {
                            // find the core with the least ready queue size
                            int leastCore = 9999, leastCoreIndex = -1;
                            for (int j = 0; j < numCores; j++)
                            {
                                int readyQueueSize = get_queue_size(READY, j);
                                if (readyQueueSize < leastCore)
                                {
                                    leastCore = readyQueueSize;
                                    leastCoreIndex = j;
                                }
                                fprintf(logFile, "core %d ready queue size %d\n", j, readyQueueSize); // debug
                            }
                            fprintf(logFile, "process %c moved to ready at time %d\n", currProcess->pid, currentTime); // debug
                            // dequeue process from processes queue and enqueue it in the ready queue of the first available core
                            coreIndex = leastCoreIndex;
                            
                            if (dequeue_process(READY, currProcess, numCores) == PROCESS_DEQUEUE_FAILED)
                            {
                                fprintf(logFile, "dequeue failed\n"); // debug
                                continue;
                            }
                            currProcess->process_status = READY;
                            enqueue_process(*currProcess, coreIndex);
                            fprintf(logFile, "process %c enqueued successfully to ready queue on core %d\n", currProcess->pid, coreIndex); // debug
                            i--;
                            continue;
                        }
                    }
                }
                // if the core is not busy and the processes queue is empty or first process has not arrived yet, search for a process in the ready queue of other cores
                fprintf(logFile, "searching for a process in the ready queue of other cores\n"); // debug
                int found = 0;
                for (int j = 0; j < numCores; j++)
                {
                    if (j == i)
                    {
                        continue;
                    }
                    if (check_queue_empty(READY, j) == QUEUE_NOT_EMPTY)
                    {
                        fprintf(logFile, "ready queue of core %d is not empty\n", j); // debug
                        
                        if (dequeue_process(READY, currProcess, j) == PROCESS_DEQUEUE_FAILED)
                        {
                            fprintf(logFile, "dequeue failed from ready of core %d\n", j); // debug
                        }
                        currProcess->process_status = READY;
                        enqueue_process(*currProcess, i);
                        fprintf(logFile, "process %c enqueued successfully to run on core %d\n", currProcess->pid, i); // debug
                        found = 1;
                        break;
                    }
                }
                if (found == 0)
                {
                    // if no process is found, schedule an idle process
                    fprintf(logFile, "no processes to run on core %d\n", i); // debug
                    scheduleProcess(result, i, 'i', currentTime, currentTime + 1);
                }
                else
                {
                    i--;
                    continue;
                }
            }
            else
            {
                fprintf(logFile, "core %d start of running\n", i); // debug
                peek_process(RUN, &currProcess, i);
                char processId = currProcess->pid;
                int processIndex = getProcessIndex(result, processId);
                ProcessInfo *process = &(values->processes[processIndex]);

                BurstInfo *burst = &(process->bursts[currProcess->next_burst]);

                if (strcmp(burst->type, "CPU") == 0)
                {
                    setProcessStatus(result, processIndex, 1, RUN);
                    scheduleProcess(result, i, processId, currentTime, currentTime + 1);
                    fprintf(logFile, "process %c is running on core %d cpu\n", processId, i); // debug
                    burst->duration--;
                }
                else
                {
                    fprintf(logFile, "process %c made IO on core %d\n", processId, i); // debug
                    setProcessStatus(result, processIndex, burst->duration, BLOCKED);
                    currProcess->next_burst++;
                    process->burstsCount--;
                    dequeue_process(RUN, currProcess, i);
                    currProcess->original_core = i;
                    currProcess->process_status = BLOCKED;
                    currProcess->waiting_time = burst->duration;
                    enqueue_process(*currProcess, numCores);
                    i--;
                    continue;
                }

                if (burst->duration == 0)
                {
                    fprintf(logFile, "process %c burst completed\n", processId); // debug
                    currProcess->next_burst++;
                    process->burstsCount--;
                    if (process->burstsCount == 0)
                    {
                        fprintf(logFile, "process %c completed\n", processId); // debug
                        setProcessTurnaroundTime(result, processIndex, currentTime + 1);
                        dequeue_process(RUN, currProcess, i);
                    }
                }
                fprintf(logFile, "core %d end of running\n", i); // debug
            }
            fprintf(logFile, "core %d end of checking\n", i); // debug
        }
        currentTime++;
        update_waiting_time(currentTime, numCores);
        fprintf(logFile, "end of iteration\n\n"); // debug
    }
    free(currProcess);
    currProcess = NULL;

    // Calculate and set average turnaround time and average response time
    double totalTurnaroundTime = 0, totalResponseTime = 0;
    for (int i = 0; i < result->processesCount; i++)
    {
        totalTurnaroundTime += result->processesAnalysis[i].turnaroundTime;
        totalResponseTime += result->processesAnalysis[i].responseTime;
    }
    setAvgTurnaroundTime(result, totalTurnaroundTime / result->processesCount);
    setAvgResponseTime(result, totalResponseTime / result->processesCount);
    fprintf(logFile, "///////////////////////////// end of FIFO2 Simulation //////////////////////////\n"); // debug
}