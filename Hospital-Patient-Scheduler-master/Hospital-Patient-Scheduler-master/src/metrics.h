#ifndef METRICS_H
#define METRICS_H

#include "scheduler.h"

typedef struct {
    double average_waiting;
    double average_turnaround;
    double average_response;
} Metrics;

Metrics calculate_metrics(
    Process processes[],
    int count
);

void display_results(
    Process processes[],
    int count,
    Metrics metrics
);

void display_comparison(
    Metrics non_preemptive,
    Metrics preemptive
);

#endif
