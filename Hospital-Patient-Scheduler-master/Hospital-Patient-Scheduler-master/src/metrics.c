#include <stdio.h>
#include "metrics.h"

/*
 * Calculate average scheduling metrics.
 */
Metrics calculate_metrics(
    Process processes[],
    int count
) {
    Metrics metrics = {
        0.0,
        0.0,
        0.0
    };

    if (count <= 0) {
        return metrics;
    }

    double total_waiting = 0.0;
    double total_turnaround = 0.0;
    double total_response = 0.0;

    for (int i = 0; i < count; i++) {

        total_waiting +=
            processes[i].waiting_time;

        total_turnaround +=
            processes[i].turnaround_time;

        total_response +=
            processes[i].response_time;
    }

    metrics.average_waiting =
        total_waiting / count;

    metrics.average_turnaround =
        total_turnaround / count;

    metrics.average_response =
        total_response / count;

    return metrics;
}

/*
 * Display results for each process.
 */
void display_results(
    Process processes[],
    int count,
    Metrics metrics
) {
    printf("\nProcess Results\n");

    printf(
        "----------------------------------------------------------------------------\n"
    );

    printf(
        "%-8s %-8s %-8s %-8s "
        "%-10s %-10s %-8s %-8s\n",

        "ID",
        "Arrival",
        "Burst",
        "Priority",
        "Completion",
        "Turnaround",
        "Waiting",
        "Response"
    );

    printf(
        "----------------------------------------------------------------------------\n"
    );

    for (int i = 0; i < count; i++) {

        Process *p =
            &processes[i];

        printf(
            "%-8s %-8d %-8d %-8d "
            "%-10d %-10d %-8d %-8d\n",

            p->id,
            p->arrival_time,
            p->burst_time,
            p->priority,
            p->completion_time,
            p->turnaround_time,
            p->waiting_time,
            p->response_time
        );
    }

    printf(
        "----------------------------------------------------------------------------\n"
    );

    printf(
        "Average Waiting Time    : %.2f\n",
        metrics.average_waiting
    );

    printf(
        "Average Turnaround Time : %.2f\n",
        metrics.average_turnaround
    );

    printf(
        "Average Response Time   : %.2f\n",
        metrics.average_response
    );
}

/*
 * Compare non-preemptive and preemptive scheduling.
 */
void display_comparison(
    Metrics non_preemptive,
    Metrics preemptive
) {
    printf("\nScheduling Comparison\n");

    printf(
        "---------------------------------------------------------\n"
    );

    printf(
        "%-25s %-15s %-15s\n",
        "Metric",
        "Non-Preemptive",
        "Preemptive"
    );

    printf(
        "---------------------------------------------------------\n"
    );

    printf(
        "%-25s %-15.2f %-15.2f\n",
        "Average Waiting Time",
        non_preemptive.average_waiting,
        preemptive.average_waiting
    );

    printf(
        "%-25s %-15.2f %-15.2f\n",
        "Average Turnaround Time",
        non_preemptive.average_turnaround,
        preemptive.average_turnaround
    );

    printf(
        "%-25s %-15.2f %-15.2f\n",
        "Average Response Time",
        non_preemptive.average_response,
        preemptive.average_response
    );

    printf(
        "---------------------------------------------------------\n"
    );
}
