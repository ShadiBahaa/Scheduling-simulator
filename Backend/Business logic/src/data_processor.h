#ifndef DATA_PROCESSOR_H
#define DATA_PROCESSOR_H

#include "cJSON.h"
#include "pqueue.h"

// Burst information structure
typedef struct
{
    const char *type;
    int duration;
} BurstInfo;

// Process information structure
typedef struct
{
    char id;
    int arrivalTime;
    int totalDuration;
    int priority;
    BurstInfo *bursts;
    int burstsCount;
} ProcessInfo;

// Processed values structure
typedef struct
{
    int numCores;
    const char *schedulingPolicy;
    ProcessInfo *processes;
    int processesCount;
} ProcessedValues;



// Process analysis structure
typedef struct 
{
    char id;
    int turnaroundTime;
    int responseTime;
    int totalDuration;
    int arrivalTime;
    int firstRunTime;
    process_status_e *status;
    int statusCounter;
    int statusCapacity;
} ProcessAnalysis;

// Scheduling information structure
typedef struct
{
    char ** schedulingOrder;
    int numCores;
    double avgTurnaroundTime;
    double avgResponseTime;
    ProcessAnalysis *processesAnalysis;
    int processesCount;
} SimulationResult;

// Function declarations
ProcessedValues *getProcessedValues(FILE *logFile);
void logProcessedValues(FILE* logFile, ProcessedValues* values);
void freeProcessedValues(ProcessedValues *values);

SimulationResult * initSimulationResult(ProcessedValues * values);
void logSimulationResult(FILE* logFile, const SimulationResult *result);
void sendSimulationResult(SimulationResult *result, FILE *logFile);
void freeSimulationResult(SimulationResult *result);

int getProcessIndex(const SimulationResult *result, char processId);
void setProcessStatus(SimulationResult *result, int processIndex, int timeUnits, process_status_e status);
void setProcessTurnaroundTime(SimulationResult *result, int processIndex, int completionTime);
void setProcessResponseTime(SimulationResult *result, int processIndex, int firstRunTime);
void setAvgTurnaroundTime(SimulationResult *result, double avgTurnaroundTime);
void setAvgResponseTime(SimulationResult *result, double avgResponseTime);
void scheduleProcess(SimulationResult *result, int coreIndex, char processId, int startTime, int endTime);
process_info_t *fill_process_info(ProcessedValues *values, int process_index);


#endif // DATA_PROCESSOR_H
