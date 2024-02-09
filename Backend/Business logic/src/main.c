#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include "data_processor.h"
#include "pqueue.h"
#include "schedulers.h"
#include "utilities.h"
#include "schedulers.h"
#include "round_robin_sem.h"

FILE *logFile;

int main()
{
    srand(time(NULL));
    // Open the log file and time stamp it
    logFile = fopen("logfile.txt", "w");
    if (logFile == NULL)
    {
        printf("Error opening log file!\n");
        return 1;
    }

    // time stamp of the log file
    // Get the current time
    time_t currentTime;
    time(&currentTime);
    // Convert the time to a string
    char timeString[26];
    strftime(timeString, sizeof(timeString), "%Y-%m-%d %H:%M:%S", localtime(&currentTime));
    // Write the time stamp to the log file
    fprintf(logFile, "Log file created at %s\n", timeString);

    // Get the processed values
    ProcessedValues *values = getProcessedValues(logFile);
    assert(values != NULL);
    fprintf(logFile, "Processed values retrieved successfully\n"); // debug

    // log the processed values to the log file // debug
    logProcessedValues(logFile, values);

    // Initialize the simulation result
    SimulationResult *result = initSimulationResult(values);
    assert(result != NULL);
    fprintf(logFile, "Simulation result initialized successfully\n"); // debug

    // determine the scheduling policy and call the appropriate function
    if (strcmp(values->schedulingPolicy, "FCFS1") == 0)
    {
        fprintf(logFile, "FCFS1 policy detected\n\n"); // debug
        simulateFIFO1(values, result, logFile);
    }
    else if (strcmp(values->schedulingPolicy, "FCFS2") == 0)
    {
        fprintf(logFile, "FCFS2 policy detected\n\n"); // debug
        simulateFIFO2(values, result, logFile);
    }
    else if (strcmp(values->schedulingPolicy, "SJF1") == 0)
    {
        fprintf(logFile, "SJF1 policy detected\n\n"); // debug
        simulateSJF1(values, result, logFile);
    }
    else if (strcmp(values->schedulingPolicy, "SJF2") == 0)
    {
        fprintf(logFile, "SJF1 policy detected\n\n"); // debug
        simulateSJF2(values, result, logFile);
    }
    else if (strcmp(values->schedulingPolicy, "SJF3") == 0)
    {
        fprintf(logFile, "SJF1 policy detected\n\n"); // debug
        simulateSJF3(values, result, logFile);
    }
    else if (strcmp(values->schedulingPolicy, "STCF1") == 0)
    {
        fprintf(logFile, "STCF1 policy detected\n\n"); // debug
        simulateSTCF1(values, result, logFile);
    }
    else if (strcmp(values->schedulingPolicy, "Priority1") == 0)
    {
        fprintf(logFile, "Priority1 policy detected\n\n"); // debug
        simulatePriority1(values, result, logFile);
    }
    else if (strcmp(values->schedulingPolicy, "Priority2") == 0)
    {
        fprintf(logFile, "Priority2 policy detected\n\n"); // debug
        simulatePriority2(values, result, logFile);
    }
    else if (strcmp(values->schedulingPolicy, "Priority3") == 0)
    {
        fprintf(logFile, "Priority2 policy detected\n\n"); // debug
        simulatePriority3(values, result, logFile);
    }
    else if (strcmp(values->schedulingPolicy, "Priority4") == 0)
    {
        fprintf(logFile, "Priority2 policy detected\n\n"); // debug
        simulatePriority4(values, result, logFile);
    }
    else if (strcmp(values->schedulingPolicy, "Stride") == 0)
    {
        fprintf(logFile, "Stride policy detected\n\n"); // debug
        simulateStride(values, result, logFile);
    }
    else if (strcmp(values->schedulingPolicy, "Lottery") == 0)
    {
        fprintf(logFile, "Lottery policy detected\n\n"); // debug
        simulateLottery(values, result, logFile);
    }
    else if (strcmp(values->schedulingPolicy, "RR") == 0)
    {
        fprintf(logFile, "Round Robin policy detected\n\n"); // debug
        simulateRoundRobin(values, result, logFile);
    }
    else if (strcmp(values->schedulingPolicy, "MLFQ") == 0)
    {
        fprintf(logFile, "MLFQ policy detected\n\n"); // debug
        simulateMLFQ(values, result, logFile);
    }
    else
    {
        fprintf(logFile, "Unknown policy detected\n"); // debug
        freeSimulationResult(result);
        freeProcessedValues(values);
        fclose(logFile);
    }

    // Send the simulation result
    sendSimulationResult(result, logFile);
    fprintf(logFile, "Simulation result sent successfully\n"); // debug

    // Free the result
    freeSimulationResult(result);
    // Free the processed values
    freeProcessedValues(values);
    // Close the log file
    fclose(logFile);

    return 0;
}
