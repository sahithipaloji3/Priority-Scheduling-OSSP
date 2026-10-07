#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "scheduler.h"
#include "metrics.h"

int main(void)
{
    int count;

    /*
     * The Java backend will provide the number
     * of processes through standard input.
     */
    if (scanf("%d", &count) != 1)
    {
        fprintf(stderr, "Invalid process count\n");
        return 1;
    }

    if (count <= 0 || count > MAX_PROCESSES)
    {
        fprintf(stderr, "Invalid number of processes\n");
        return 1;
    }

    Process processes[MAX_PROCESSES];

    for (int i = 0; i < count; i++)
    {
        char id[ID_LENGTH];

        int arrival;
        int burst;
        int priority;

        if (scanf(
                "%19s %d %d %d",
                id,
                &arrival,
                &burst,
                &priority
            ) != 4)
        {
            fprintf(stderr, "Invalid process data\n");
            return 1;
        }

        strcpy(processes[i].id, id);

        processes[i].arrival_time = arrival;
        processes[i].burst_time = burst;
        processes[i].priority = priority;
    }

    /*
     * The Java backend sends:
     *
     * 0 = Non-Preemptive
     * 1 = Preemptive
     */
    int algorithm;

    if (scanf("%d", &algorithm) != 1)
    {
        fprintf(stderr, "Invalid algorithm\n");
        return 1;
    }

    ScheduleResult result;

    result.segment_count = 0;

    reset_processes(processes, count);

    if (algorithm == 0)
    {
        non_preemptive_priority(
            processes,
            count,
            0,
            0,
            &result
        );
    }
    else if (algorithm == 1)
    {
        preemptive_priority(
            processes,
            count,
            0,
            0,
            &result
        );
    }
    else
    {
        fprintf(stderr, "Invalid algorithm\n");
        return 1;
    }

    Metrics metrics =
        calculate_metrics(
            processes,
            count
        );

    /*
     * Simple machine-readable output
     * for the Java backend.
     */

    printf("AVERAGE_WAITING %.2f\n",
           metrics.average_waiting);

    printf("AVERAGE_TURNAROUND %.2f\n",
           metrics.average_turnaround);

    printf("AVERAGE_RESPONSE %.2f\n",
           metrics.average_response);

    printf("PROCESS_RESULTS\n");

    for (int i = 0; i < count; i++)
    {
        printf(
            "%s %d %d %d %d %d %d %d\n",
            processes[i].id,
            processes[i].arrival_time,
            processes[i].burst_time,
            processes[i].priority,
            processes[i].completion_time,
            processes[i].turnaround_time,
            processes[i].waiting_time,
            processes[i].response_time
        );
    }

    printf("GANTT\n");

    for (int i = 0;
         i < result.segment_count;
         i++)
    {
        printf(
            "%s %d %d\n",
            result.segments[i].process_id,
            result.segments[i].start_time,
            result.segments[i].end_time
        );
    }

    return 0;
}
