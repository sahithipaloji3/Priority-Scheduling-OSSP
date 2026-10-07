#ifndef SCHEDULER_H
#define SCHEDULER_H

#define MAX_PROCESSES 100
#define ID_LENGTH 20
#define MAX_SEGMENTS 1000

typedef enum {
    READY,
    RUNNING,
    TERMINATED
} ProcessState;

typedef struct {
    char id[ID_LENGTH];

    int arrival_time;
    int burst_time;
    int priority;

    int remaining_time;

    int completion_time;
    int turnaround_time;
    int waiting_time;
    int response_time;

    int first_start_time;

    ProcessState state;
    int completed;
} Process;

typedef struct {
    int start_time;
    int end_time;
    char process_id[ID_LENGTH];
} GanttSegment;

typedef struct {
    GanttSegment segments[MAX_SEGMENTS];
    int segment_count;
} ScheduleResult;

void reset_processes(Process processes[], int count);

void non_preemptive_priority(
    Process processes[],
    int count,
    int aging_enabled,
    int aging_threshold,
    ScheduleResult *result
);

void preemptive_priority(
    Process processes[],
    int count,
    int aging_enabled,
    int aging_threshold,
    ScheduleResult *result
);

void display_process_states(Process processes[], int count);
void display_execution_order(ScheduleResult *result);
void display_gantt_chart(ScheduleResult *result);

#endif
