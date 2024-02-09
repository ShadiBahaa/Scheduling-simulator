#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include "pqueue.h"
#include "utilities.h"
#include "round_robin.h"
#include "mlfq.h"

static comparison_status_e compare_to(process_info_t process_info_1, process_info_t process_info_2, comparison_condition_e comparison_condition)
{
    comparison_status_e comparison_status;
    switch (comparison_condition)
    {
    case ARRIVAL_TIME:
        if (process_info_1.arrival_time == process_info_2.arrival_time)
        {
            comparison_status = EQUAL;
        }
        else if (process_info_1.arrival_time > process_info_2.arrival_time)
        {
            comparison_status = GREATER_THAN;
        }
        else
        {
            comparison_status = LESS_THAN;
        }
        break;
    case PROCESS_ID:
        if (process_info_1.pid == process_info_2.pid)
        {
            comparison_status = EQUAL;
        }
        else if (process_info_1.pid > process_info_2.pid)
        {
            comparison_status = GREATER_THAN;
        }
        else
        {
            comparison_status = LESS_THAN;
        }
        break;
    case EXCECUTION_TIME:
        if (process_info_1.execution_time == process_info_2.execution_time)
        {
            comparison_status = EQUAL;
        }
        else if (process_info_1.execution_time > process_info_2.execution_time)
        {
            comparison_status = GREATER_THAN;
        }
        else
        {
            comparison_status = LESS_THAN;
        }
        break;
    case WAITING_TIME:
        if (process_info_1.waiting_time == process_info_2.waiting_time)
        {
            comparison_status = EQUAL;
        }
        else if (process_info_1.waiting_time > process_info_2.waiting_time)
        {
            comparison_status = GREATER_THAN;
        }
        else
        {
            comparison_status = LESS_THAN;
        }
        break;
    case RESPONSE_TIME:
        if (process_info_1.response_time == process_info_2.response_time)
        {
            comparison_status = EQUAL;
        }
        else if (process_info_1.response_time > process_info_2.response_time)
        {
            comparison_status = GREATER_THAN;
        }
        else
        {
            comparison_status = LESS_THAN;
        }
        break;
    case TURNAROUND_TIME:
        if (process_info_1.turnaround_time == process_info_2.turnaround_time)
        {
            comparison_status = EQUAL;
        }
        else if (process_info_1.turnaround_time > process_info_2.turnaround_time)
        {
            comparison_status = GREATER_THAN;
        }
        else
        {
            comparison_status = LESS_THAN;
        }
        break;
    case CURRENT_CORE:
        if (process_info_1.current_core == process_info_2.current_core)
        {
            comparison_status = EQUAL;
        }
        else if (process_info_1.current_core > process_info_2.current_core)
        {
            comparison_status = GREATER_THAN;
        }
        else
        {
            comparison_status = LESS_THAN;
        }
        break;
    case TIME_CONSUMED:
        if (process_info_1.time_consumed == process_info_2.time_consumed)
        {
            comparison_status = EQUAL;
        }
        else if (process_info_1.time_consumed > process_info_2.time_consumed)
        {
            comparison_status = GREATER_THAN;
        }
        else
        {
            comparison_status = LESS_THAN;
        }
        break;
    case PROCESS_PRIORITY:
        if (process_info_1.priority == process_info_2.priority)
        {
            comparison_status = EQUAL;
        }
        else if (process_info_1.priority > process_info_2.priority)
        {
            comparison_status = GREATER_THAN;
        }
        else
        {
            comparison_status = LESS_THAN;
        }
        break;
    default:
        break;
    }
    return comparison_status;
}

static void merge(process_info_t * arr, iterator_t left, iterator_t mid, iterator_t right, comparison_condition_e comparison_condition)
{
    iterator_t first_array_iterator, second_array_iterator, original_array_iterator;
    size_t first_size = mid - left + 1;
    size_t second_size = right - mid;

    process_info_t left_array[first_size], right_array[second_size];

    for (first_array_iterator = 0; first_array_iterator < first_size; first_array_iterator++)
        left_array[first_array_iterator] = arr[left + first_array_iterator];
    for (second_array_iterator = 0; second_array_iterator < second_size; second_array_iterator++)
        right_array[second_array_iterator] = arr[mid + 1 + second_array_iterator];

    first_array_iterator = 0;
    second_array_iterator = 0;
    original_array_iterator = left;
    while (first_array_iterator < first_size && second_array_iterator < second_size)
    {
        if (compare_to(left_array[first_array_iterator], right_array[second_array_iterator], comparison_condition) == EQUAL || compare_to(left_array[first_array_iterator], right_array[second_array_iterator], comparison_condition) == LESS_THAN)
        {
            arr[original_array_iterator] = left_array[first_array_iterator];
            first_array_iterator++;
        }
        else
        {
            arr[original_array_iterator] = right_array[second_array_iterator];
            second_array_iterator++;
        }
        original_array_iterator++;
    }

    while (first_array_iterator < first_size)
    {
        arr[original_array_iterator] = left_array[first_array_iterator];
        first_array_iterator++;
        original_array_iterator++;
    }

    while (second_array_iterator < second_size)
    {
        arr[original_array_iterator] = right_array[second_array_iterator];
        second_array_iterator++;
        original_array_iterator++;
    }
}
void merge_sort(process_info_t * arr, iterator_t left, iterator_t right, comparison_condition_e comparison_condition)
{
    if (left < right)
    {
        iterator_t mid = left + (right - left) / 2;

        merge_sort(arr, left, mid, comparison_condition);
        merge_sort(arr, mid + 1, right, comparison_condition);

        merge(arr, left, mid, right, comparison_condition);
    }
}