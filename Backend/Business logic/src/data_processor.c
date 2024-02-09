#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cJSON.h"
#include "data_processor.h"
#include "pqueue.h"

ProcessedValues *processJson(cJSON *root)
{
    ProcessedValues *values = (ProcessedValues *)malloc(sizeof(ProcessedValues));
    if (!values)
    {
        fprintf(stderr, "Memory allocation error\n");
        return NULL;
    }

    // Initialize values
    values->numCores = 0; // Set default values
    values->schedulingPolicy = NULL;
    values->processesCount = 0;

    // Access the JSON data as needed
    //  Process the Number of Cores
    cJSON *numCoresJson = cJSON_GetObjectItemCaseSensitive(root, "numCores");
    if (cJSON_IsNumber(numCoresJson))
    {
        const int numCores = numCoresJson->valueint;
        values->numCores = numCores;
    }
    else if (cJSON_IsString(numCoresJson))
    {
        const int numCores = atoi(numCoresJson->valuestring);
        values->numCores = numCores;
    }
    else
    {
        fprintf(stderr, "Invalid 'numCores' value\n");
        freeProcessedValues(values);
        return NULL;
    }

    // Process the Scheudling Policy
    cJSON *schedulingPolicyJson = cJSON_GetObjectItemCaseSensitive(root, "schedulingPolicy");
    if (cJSON_IsString(schedulingPolicyJson))
    {
        const char *schedulingPolicy = schedulingPolicyJson->valuestring;
        values->schedulingPolicy = strdup(schedulingPolicy);
    }
    else
    {
        fprintf(stderr, "Invalid 'schedulingPolicy' value\n");
        freeProcessedValues(values);
        return NULL;
    }

    // Process the array of processes
    cJSON *processesJson = cJSON_GetObjectItemCaseSensitive(root, "processes");
    if (cJSON_IsArray(processesJson))
    {
        // Process the array of processes
        int processesCount = cJSON_GetArraySize(processesJson);
        values->processesCount = processesCount;

        // Allocate memory for the array of processes
        values->processes = (ProcessInfo *)malloc(sizeof(ProcessInfo) * values->processesCount);
        if (!values->processes)
        {
            fprintf(stderr, "Memory allocation error\n");
            freeProcessedValues(values);
            return NULL;
        }

        for (int i = 0; i < processesCount; ++i)
        {
            cJSON *process = cJSON_GetArrayItem(processesJson, i);
            ProcessInfo *currentProcess = &(values->processes[i]);

            // Access process properties and perform necessary processing
            //  Process ID
            const char *processId = cJSON_GetObjectItemCaseSensitive(process, "id")->valuestring;
            currentProcess->id = processId[0];

            //  Process Arrival Time
            const cJSON *arrivalTimeJson = cJSON_GetObjectItemCaseSensitive(process, "arrivalTime");
            if (cJSON_IsNumber(arrivalTimeJson))
            {
                const int arrivalTime = arrivalTimeJson->valueint;
                currentProcess->arrivalTime = arrivalTime;
            }
            else if (cJSON_IsString(arrivalTimeJson))
            {
                const int arrivalTime = atoi(arrivalTimeJson->valuestring);
                currentProcess->arrivalTime = arrivalTime;
            }
            else
            {
                fprintf(stderr, "Invalid 'arrivalTime' value\n");
                freeProcessedValues(values);
                return NULL;
            }

            //  Process Total Duration
            const cJSON *totalDurationJson = cJSON_GetObjectItemCaseSensitive(process, "totalDuration");
            if (cJSON_IsNumber(totalDurationJson))
            {
                const int totalDuration = totalDurationJson->valueint;
                currentProcess->totalDuration = totalDuration;
            }
            else if (cJSON_IsString(totalDurationJson))
            {
                const int totalDuration = atoi(totalDurationJson->valuestring);
                currentProcess->totalDuration = totalDuration;
            }
            else
            {
                fprintf(stderr, "Invalid 'totalDuration' value\n");
                freeProcessedValues(values);
                return NULL;
            }

            //  Process Priority
            const cJSON *priorityJson = cJSON_GetObjectItemCaseSensitive(process, "priority");
            if (cJSON_IsNumber(priorityJson))
            {
                const int priority = priorityJson->valueint;
                currentProcess->priority = priority;
            }
            else if (cJSON_IsString(priorityJson))
            {
                const int priority = atoi(priorityJson->valuestring);
                currentProcess->priority = priority;
            }
            else
            {
                fprintf(stderr, "Invalid 'priority' value\n");
                freeProcessedValues(values);
                return NULL;
            }

            //  Process Bursts
            cJSON *burstsJson = cJSON_GetObjectItemCaseSensitive(process, "bursts");
            if (cJSON_IsArray(burstsJson))
            {
                // Process the array of bursts
                int burstsCount = cJSON_GetArraySize(burstsJson);
                currentProcess->burstsCount = burstsCount;

                // Allocate memory for the array of bursts
                currentProcess->bursts = (BurstInfo *)malloc(sizeof(BurstInfo) * burstsCount);
                if (!currentProcess->bursts)
                {
                    fprintf(stderr, "Memory allocation error\n");
                    freeProcessedValues(values);
                    return NULL;
                }

                for (int j = 0; j < burstsCount; ++j)
                {
                    cJSON *burst = cJSON_GetArrayItem(burstsJson, j);
                    BurstInfo *currentBurst = &(currentProcess->bursts[j]);

                    // Access burst properties and perform necessary processing
                    // Burst Type
                    const char *burstType = cJSON_GetObjectItemCaseSensitive(burst, "type")->valuestring;
                    currentBurst->type = strdup(burstType);

                    // Burst Duration
                    const cJSON *burstDurationJson = cJSON_GetObjectItemCaseSensitive(burst, "duration");
                    if (cJSON_IsNumber(burstDurationJson))
                    {
                        const int burstDuration = burstDurationJson->valueint;
                        currentBurst->duration = burstDuration;
                    }
                    else if (cJSON_IsString(burstDurationJson))
                    {
                        const int burstDuration = atoi(burstDurationJson->valuestring);
                        currentBurst->duration = burstDuration;
                    }
                    else
                    {
                        fprintf(stderr, "Invalid 'duration' value\n");
                        freeProcessedValues(values);
                        return NULL;
                    }
                }
            }
            else
            {
                fprintf(stderr, "Invalid 'bursts' value\n");
                freeProcessedValues(values);
                return NULL;
            }
        }
    }
    else
    {
        fprintf(stderr, "Invalid 'processes' value\n");
        freeProcessedValues(values);
        return NULL;
    }

    return values;
}

// Helper function to free the memory allocated for bursts
void freeBursts(BurstInfo *bursts, int burstsCount)
{
    if (bursts == NULL)
        return;

    for (int i = 0; i < burstsCount; ++i)
    {
        free((void *)bursts[i].type);
    }

    free(bursts);
}

// Helper function to free the memory allocated for processes
void freeProcesses(ProcessInfo *processes, int processesCount)
{
    if (processes == NULL)
        return;

    for (int i = 0; i < processesCount; ++i)
    {

        freeBursts(processes[i].bursts, processes[i].burstsCount);
    }

    free(processes);
}

// Helper function to free the memory allocated for processed values
void freeProcessedValues(ProcessedValues *values)
{
    if (values == NULL)
        return;

    free((void *)values->schedulingPolicy);
    freeProcesses(values->processes, values->processesCount);
    free(values);
}

ProcessedValues *getProcessedValues(FILE *logFile)
{
    // Read the JSON input from stdin
    char *buffer = NULL;
    size_t size = 40960; // Initial buffer size
    size_t len = 0;

    buffer = (char *)malloc(size);
    if (!buffer)
    {
        fprintf(stderr, "Memory allocation error\n");
        return NULL;
    }

    while (fgets(buffer + len, size - len, stdin) != NULL)
    {
        len += strlen(buffer + len);

        // Check if the entire JSON has been read
        if (len >= size - 1 || buffer[len - 1] == '\n')
            break;

        // If not, expand the buffer
        size *= 2;
        buffer = (char *)realloc(buffer, size);
        if (!buffer)
        {
            fprintf(stderr, "Memory reallocation error\n");
            return NULL;
        }
    }

    // Parse the JSON data
    cJSON *root = cJSON_Parse(buffer);
    fprintf(logFile, "%s\n\n", buffer);
    if (!root)
    {
        fprintf(stderr, "Error parsing JSON: %s\n", cJSON_GetErrorPtr());
        free(buffer);
        return NULL;
    }

    // Process the JSON data
    ProcessedValues *values = processJson(root);

    // Free the cJSON object and buffer
    cJSON_Delete(root);
    free(buffer);

    return values;
}

void logProcessedValues(FILE* logFile, ProcessedValues* values){
    fprintf(logFile, "numCores: %d\n", values->numCores);
    fprintf(logFile, "processesCount: %d\n", values->processesCount);
    fprintf(logFile, "schedulingPolicy: %s\n", values->schedulingPolicy);
    fprintf(logFile, "\n");
    for (int i = 0; i < values->processesCount; i++)
    {
        fprintf(logFile, "process %d\n", i + 1);
        fprintf(logFile, "id: %c\n", values->processes[i].id);
        fprintf(logFile, "arrivalTime: %d\n", values->processes[i].arrivalTime);
        fprintf(logFile, "totalDuration: %d\n", values->processes[i].totalDuration);
        fprintf(logFile, "priority: %d\n", values->processes[i].priority);
        fprintf(logFile, "burstsCount: %d\n", values->processes[i].burstsCount);
        for (int j = 0; j < values->processes[i].burstsCount; j++)
        {
            fprintf(logFile, "burst %d: ", j);
            fprintf(logFile, "type: %s: ", values->processes[i].bursts[j].type);
            fprintf(logFile, "duration: %d\n", values->processes[i].bursts[j].duration);
        }
        fprintf(logFile, "\n");
    }
}

void sendSimulationResult(SimulationResult *result, FILE *logFile)
{
    // Create the root JSON object
    cJSON *root = cJSON_CreateObject();

    // Add schedulingOrder to the JSON object
    // for (int i = 0; i < result->numCores; ++i)
    // {
        
    // }

    cJSON *schedulingOrderJson = cJSON_CreateStringArray((const char **)result->schedulingOrder, result->numCores);
    cJSON_AddItemToObject(root, "schedulingOrder", schedulingOrderJson);

    // Add numCores, avgTurnaroundTime, avgResponseTime, and processesCount to the JSON object
    cJSON_AddNumberToObject(root, "numCores", result->numCores);
    cJSON_AddNumberToObject(root, "avgTurnaroundTime", result->avgTurnaroundTime);
    cJSON_AddNumberToObject(root, "avgResponseTime", result->avgResponseTime);
    cJSON_AddNumberToObject(root, "processesCount", result->processesCount);

    // Add processesAnalysis to the JSON object
    cJSON *processesAnalysisJson = cJSON_CreateArray();
    for (int i = 0; i < result->processesCount; ++i)
    {
        cJSON *processAnalysisJson = cJSON_CreateObject();
        char idStr[2] = {result->processesAnalysis[i].id, '\0'};
        cJSON_AddStringToObject(processAnalysisJson, "id", idStr);
        cJSON_AddNumberToObject(processAnalysisJson, "turnaroundTime", result->processesAnalysis[i].turnaroundTime);
        cJSON_AddNumberToObject(processAnalysisJson, "responseTime", result->processesAnalysis[i].responseTime);
        cJSON_AddNumberToObject(processAnalysisJson, "totalDuration", result->processesAnalysis[i].totalDuration);
        cJSON_AddNumberToObject(processAnalysisJson, "arrivalTime", result->processesAnalysis[i].arrivalTime);
        cJSON_AddNumberToObject(processAnalysisJson, "firstRunTime", result->processesAnalysis[i].firstRunTime);
        cJSON_AddNumberToObject(processAnalysisJson, "statusCapacity", result->processesAnalysis[i].statusCapacity);
        cJSON_AddNumberToObject(processAnalysisJson, "statusCounter", 0);
        cJSON *statusJson = cJSON_CreateIntArray((const int *)result->processesAnalysis[i].status, result->processesAnalysis[i].statusCounter);
        cJSON_AddItemToObject(processAnalysisJson, "status", statusJson);
        cJSON_AddItemToArray(processesAnalysisJson, processAnalysisJson);
    }
    cJSON_AddItemToObject(root, "processesAnalysis", processesAnalysisJson);

    // Print the JSON object
    char *jsonString = cJSON_Print(root);
    printf("%s\n", jsonString);
    fprintf(logFile, "\n%s\n", jsonString);

    // Free the cJSON object and buffer
    cJSON_Delete(root);
    free(jsonString);
}

SimulationResult *initSimulationResult(ProcessedValues *values)
{
    SimulationResult *result = (SimulationResult *)malloc(sizeof(SimulationResult));
    if (!result)
    {
        fprintf(stderr, "Memory allocation error\n");
        return NULL;
    }

    // Initialize values
    result->numCores = values->numCores;
    result->avgTurnaroundTime = 0;
    result->avgResponseTime = 0;
    result->processesCount = values->processesCount;

    // Calculate length of scheduling order
    int totalTime = 0;
    int arivalMax = 0;
    for (int i = 0; i < values->processesCount; ++i)
    {
        totalTime += values->processes[i].totalDuration;
        if (values->processes[i].arrivalTime > arivalMax)
        {
            arivalMax = values->processes[i].arrivalTime;
        }
    }
    totalTime += arivalMax;

    // Allocate memory for the scheduling order array
    result->schedulingOrder = (char **)malloc(sizeof(char *) * result->numCores);
    if (!result->schedulingOrder)
    {
        fprintf(stderr, "Memory allocation error\n");
        free(result);
        return NULL;
    }

    // Allocate memory for each scheduling order string
    for (int i = 0; i < result->numCores; ++i)
    {
        result->schedulingOrder[i] = (char *)malloc(sizeof(char) * (totalTime + 1));
        if (!result->schedulingOrder[i])
        {
            fprintf(stderr, "Memory allocation error\n");
            for (int j = 0; j < i; ++j) {
                free(result->schedulingOrder[j]);
            }
            free(result->schedulingOrder);
            free(result);
            return NULL;
        }
        result->schedulingOrder[i][0] = '\0';
    }

    // Allocate memory for the processes analysis array
    result->processesAnalysis = (ProcessAnalysis *)malloc(sizeof(ProcessAnalysis) * result->processesCount);
    if (!result->processesAnalysis)
    {
        fprintf(stderr, "Memory allocation error\n");
        free(result->schedulingOrder);
        free(result);
        return NULL;
    }

    // Initialize the processes analysis array
    for (int i = 0; i < result->processesCount; ++i)
    {
        ProcessAnalysis *currentProcessAnalysis = &(result->processesAnalysis[i]);
        ProcessInfo *currentProcessInfo = &(values->processes[i]);

        // Initialize values
        currentProcessAnalysis->id = currentProcessInfo->id;
        currentProcessAnalysis->turnaroundTime = 0;
        currentProcessAnalysis->responseTime = 0;
        currentProcessAnalysis->totalDuration = currentProcessInfo->totalDuration;
        currentProcessAnalysis->arrivalTime = currentProcessInfo->arrivalTime;
        currentProcessAnalysis->statusCounter = 0;
        currentProcessAnalysis->statusCapacity = currentProcessInfo->totalDuration;
        currentProcessAnalysis->firstRunTime = -1;

        // Allocate memory for the status array
        currentProcessAnalysis->status = (process_status_e *)malloc(sizeof(process_status_e) * currentProcessAnalysis->totalDuration);
        if (!currentProcessAnalysis->status)
        {
            fprintf(stderr, "Memory allocation error\n");
            free(result->schedulingOrder);
            free(result->processesAnalysis);
            free(result);
            return NULL;
        }

        // Initialize the status array
        for (int j = 0; j < currentProcessAnalysis->totalDuration; ++j)
        {
            currentProcessAnalysis->status[j] = NOT_VALID;
        }
    }

    // sort processes by arrival time for the values only
    for (int i = 0; i < values->processesCount; ++i)
    {
        for (int j = i + 1; j < values->processesCount; ++j)
        {
            if (values->processes[i].arrivalTime > values->processes[j].arrivalTime)
            {
                ProcessInfo temp = values->processes[i];
                values->processes[i] = values->processes[j];
                values->processes[j] = temp;
            }
        }
    }

    // sort processes by arrival time for the result only
    for (int i = 0; i < result->processesCount; ++i)
    {
        for (int j = i + 1; j < result->processesCount; ++j)
        {
            if (result->processesAnalysis[i].arrivalTime > result->processesAnalysis[j].arrivalTime)
            {
                ProcessAnalysis temp = result->processesAnalysis[i];
                result->processesAnalysis[i] = result->processesAnalysis[j];
                result->processesAnalysis[j] = temp;
            }
        }
    }

    return result;
}

void freeSimulationResult(SimulationResult *result)
{
    if (result == NULL)
        return;

    free((void *)result->schedulingOrder);

    for (int i = 0; i < result->processesCount; ++i)
    {
        free((void *)result->processesAnalysis[i].status);
    }

    free(result->processesAnalysis);
    free(result);
}

void setProcessStatus(SimulationResult *result, int processIndex, int timeUnits, process_status_e status)
{
    if (!result)
    {
        fprintf(stderr, "Invalid input\n");
        return;
    }

    if (processIndex < 0 || processIndex >= result->processesCount)
    {
        fprintf(stderr, "Invalid process index\n");
        return;
    }

    //if reached the end of the status array then realloc

    if (result->processesAnalysis[processIndex].statusCounter + timeUnits > result->processesAnalysis[processIndex].statusCapacity)
    {
        result->processesAnalysis[processIndex].status = (process_status_e *)realloc(result->processesAnalysis[processIndex].status, sizeof(process_status_e) * (result->processesAnalysis[processIndex].statusCounter + timeUnits));
        if (!result->processesAnalysis[processIndex].status)
        {
            fprintf(stderr, "Memory allocation error\n");
            return;
        }
        for (int i = result->processesAnalysis[processIndex].statusCapacity; i < result->processesAnalysis[processIndex].statusCounter + timeUnits; ++i)
        {
            result->processesAnalysis[processIndex].status[i] = NOT_VALID;
        }
        result->processesAnalysis[processIndex].statusCapacity = result->processesAnalysis[processIndex].statusCounter + timeUnits;
    }

    

    for (int i = 0; i < timeUnits; ++i)
    {
        result->processesAnalysis[processIndex].status[result->processesAnalysis[processIndex].statusCounter] = status;
        result->processesAnalysis[processIndex].statusCounter++;
    }
}

void setProcessTurnaroundTime(SimulationResult *result, int processIndex, int completionTime)
{
    if (!result)
    {
        fprintf(stderr, "Invalid input\n");
        return;
    }

    if (processIndex < 0 || processIndex >= result->processesCount)
    {
        fprintf(stderr, "Invalid process index\n");
        return;
    }

    result->processesAnalysis[processIndex].turnaroundTime = completionTime - result->processesAnalysis[processIndex].arrivalTime;
}

void setProcessResponseTime(SimulationResult *result, int processIndex, int firstRunTime)
{
    if (!result)
    {
        fprintf(stderr, "Invalid input\n");
        return;
    }

    if (processIndex < 0 || processIndex >= result->processesCount)
    {
        fprintf(stderr, "Invalid process index\n");
        return;
    }

    result->processesAnalysis[processIndex].responseTime = firstRunTime - result->processesAnalysis[processIndex].arrivalTime;
    result->processesAnalysis[processIndex].firstRunTime = firstRunTime;
}

int getProcessIndex(const SimulationResult *result, char processId)
{
    if (!result)
    {
        fprintf(stderr, "Invalid input\n");
        return -1;
    }

    for (int i = 0; i < result->processesCount; ++i)
    {
        if (result->processesAnalysis[i].id == processId)
        {
            return i;
        }
    }

    return -1;
}

void setAvgTurnaroundTime(SimulationResult *result, double avgTurnaroundTime)
{
    if (!result)
    {
        fprintf(stderr, "Invalid input\n");
        return;
    }

    result->avgTurnaroundTime = avgTurnaroundTime;
}

void setAvgResponseTime(SimulationResult *result, double avgResponseTime)
{
    if (!result)
    {
        fprintf(stderr, "Invalid input\n");
        return;
    }

    result->avgResponseTime = avgResponseTime;
}

void scheduleProcess(SimulationResult *result, int coreIndex, char processId, int startTime, int endTime)
{
    if (!result)
    {
        fprintf(stderr, "Invalid input\n");
        return;
    }

    if (coreIndex < 0 || coreIndex >= result->numCores)
    {
        fprintf(stderr, "Invalid core index\n");
        return;
    }

    char *schedulingOrder = result->schedulingOrder[coreIndex];
    for (int i = startTime; i < endTime; ++i)
    {
        schedulingOrder[i] = processId;
    }
    schedulingOrder[endTime] = '\0';
}

// a function to log the simulation result
void logSimulationResult(FILE* logFile, const SimulationResult *result)
{
    fprintf(logFile, "numCores: %d\n", result->numCores);
    fprintf(logFile, "avgTurnaroundTime: %f\n", result->avgTurnaroundTime);
    fprintf(logFile, "avgResponseTime: %f\n", result->avgResponseTime);
    fprintf(logFile, "processesCount: %d\n", result->processesCount);
    fprintf(logFile, "\n");
    for (int i = 0; i < result->processesCount; i++)
    {
        fprintf(logFile, "process %d\n", i + 1);
        fprintf(logFile, "id: %c\n", result->processesAnalysis[i].id);
        fprintf(logFile, "turnaroundTime: %d\n", result->processesAnalysis[i].turnaroundTime);
        fprintf(logFile, "responseTime: %d\n", result->processesAnalysis[i].responseTime);
        fprintf(logFile, "totalDuration: %d\n", result->processesAnalysis[i].totalDuration);
        fprintf(logFile, "arrivalTime: %d\n", result->processesAnalysis[i].arrivalTime);
        fprintf(logFile, "firstRunTime: %d\n", result->processesAnalysis[i].firstRunTime);
        fprintf(logFile, "statusCounter: %d\n", result->processesAnalysis[i].statusCounter);
        fprintf(logFile, "statusCapacity: %d\n", result->processesAnalysis[i].statusCapacity);
        for (int j = 0; j < result->processesAnalysis[i].statusCapacity; j++)
        {
            fprintf(logFile, "status %d: %d, ", j, result->processesAnalysis[i].status[j]);
        }
        fprintf(logFile, "\n");
    }
    fprintf(logFile, "\n");
    for (int i = 0; i < result->numCores; i++)
    {
        fprintf(logFile, "core %d: %s\n", i + 1, result->schedulingOrder[i]);
    }
    fprintf(logFile, "\n");
}

// a function that fill make a process_info_t from processesd values
process_info_t *fill_process_info(ProcessedValues *values, int process_index)
{
    process_info_t *process_info = (process_info_t *)malloc(sizeof(process_info_t));
    if (!process_info)
    {
        fprintf(stderr, "Memory allocation error\n");
        return NULL;
    }

    // Initialize values
    process_info->pid = values->processes[process_index].id;
    process_info->process_status = NOT_VALID;
    process_info->execution_time = 0;
    process_info->arrival_time = values->processes[process_index].arrivalTime;
    process_info->waiting_time = 0;
    process_info->priority = values->processes[process_index].priority;
    process_info->total_duration = values->processes[process_index].totalDuration;
    process_info->tickets = (process_info->priority + 1) * 50;
    process_info->stride = 1000 / process_info->tickets;
    process_info->pass = 0;
    process_info->burst_info = (burst_info_t *)malloc(sizeof(burst_info_t) * values->processes[process_index].burstsCount);
    if (!process_info->burst_info)
    {
        fprintf(stderr, "Memory allocation error\n");
        free(process_info);
        return NULL;
    }
    process_info->bursts_count = values->processes[process_index].burstsCount;
    process_info->next_burst = 0;
    process_info->process_handler = NULL;
    process_info->proccess_args = NULL;
    process_info->process_return = NULL;
    process_info->original_core = -1;
    process_info->current_core = -1;

    for (int i = 0; i < values->processes[process_index].burstsCount; ++i)
    {
        process_info->burst_info[i].burst_type = values->processes[process_index].bursts[i].type[0];
        process_info->burst_info[i].execution_time = values->processes[process_index].bursts[i].duration;
    }

    return process_info;
}

