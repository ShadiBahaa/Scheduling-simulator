#ifndef _MLFQ_H_
#define _MLFQ_H_

void send_mlfq_data(process_info_t *processes, counter_t processes_count);
void init_mlfq(process_core_t cores_count, time_t *time_slices, counter_t time_slices_count, time_t boost_time);
void run_mlfq(void);
void run_mlfq_simulator(void);
void run_mlfq_scheduler(void);

#endif