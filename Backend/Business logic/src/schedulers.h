
#ifndef SCHEDULERS_H
#define SCHEDULERS_H

#include <stdio.h>
#include <stdlib.h>
#include "data_processor.h"
#include "pqueue.h"


void simulateFIFO1(ProcessedValues *values, SimulationResult *result, FILE *logFile);
void simulateFIFO2(ProcessedValues *values, SimulationResult *result, FILE *logFile);
void simulateSJF1(ProcessedValues *values, SimulationResult *result, FILE* logFile);
void simulateSJF2(ProcessedValues *values, SimulationResult *result, FILE* logFile);
void simulateSJF3(ProcessedValues *values, SimulationResult *result, FILE* logFile);
void simulateSTCF1(ProcessedValues *values, SimulationResult *result, FILE *logFile);
void simulatePriority1(ProcessedValues *values, SimulationResult *result, FILE *logFile);
void simulatePriority2(ProcessedValues *values, SimulationResult *result, FILE *logFile);
void simulatePriority3(ProcessedValues *values, SimulationResult *result, FILE *logFile);
void simulatePriority4(ProcessedValues *values, SimulationResult *result, FILE *logFile);
void simulateStride(ProcessedValues *values, SimulationResult *result, FILE *logFile);
void simulateLottery(ProcessedValues *values, SimulationResult *result, FILE *logFile);

#endif // SCHEDULERS_H