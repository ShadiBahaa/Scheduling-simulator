#ifndef _ROUND_ROBIN_SEM_H_
#define _ROUND_ROBIN_SEM_H_

void simulateRoundRobin(ProcessedValues *values, SimulationResult *result, FILE *logFile);
void transfer_processes(ProcessedValues *values, process_info_t *arr, counter_t pro_count);

#endif