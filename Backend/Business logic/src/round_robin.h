#ifndef _ROUND_ROBIN_H_
#define _ROUND_ROBIN_H_

#define TIMER_CREATION_FAILED -1
#define TIMER_SPECS_SETTING_FAILED -1
typedef unsigned short int signal_id_t;



void run_round_robin_simulator(void);
void run_round_robin_scheduler(void);
void stop_round_robin(void);
process_enqueue_status_e enqueue_round_robin_process(process_info_t *process_info);
void send_data_round_robin(process_info_t *arr, counter_t processes_count);
void init_round_robin_processes(process_core_t cores_count, counter_t processes_num);
void init_round_robin_scheduler(time_t slice_time_in_sec);
#endif