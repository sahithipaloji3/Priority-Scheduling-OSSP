#include <stdio.h>
#include <string.h>
#include "scheduler.h"

/*
 * Add a segment to the Gantt chart.
 * If the same process continues running,
 * merge the two segments into one.
 */
static void add_segment(
    ScheduleResult *result,
    int start,
    int end,
    const char *process_id
) {
    if (result->segment_count >= MAX_SEGMENTS) {
        return;
    }

    if (result->segment_count > 0) {
        GanttSegment *last =
            &result->segments[result->segment_count - 1];

        if (strcmp(last->process_id, process_id) == 0 &&
            last->end_time == start) {

            last->end_time = end;
            return;
        }
    }

    GanttSegment *segment =
        &result->segments[result->segment_count];

    segment->start_time = start;
    segment->end_time = end;

    strncpy(
        segment->process_id,
        process_id,
        ID_LENGTH - 1
    );

    segment->process_id[ID_LENGTH - 1] = '\0';

    result->segment_count++;
}

/*
 * Calculate the effective priority of a process.
 *
 * Lower priority number means higher priority.
 *
 * If aging is enabled, a waiting process gets
 * a better priority as its waiting time increases.
 */
static int effective_priority(
    const Process *p,
    int current_time,
    int aging_enabled,
    int aging_threshold
) {
    int priority = p->priority;

    if (!aging_enabled || aging_threshold <= 0) {
        return priority;
    }

    if (current_time <= p->arrival_time) {
        return priority;
    }

    /*
     * Calculate how long the process has been waiting
     * since its arrival.
     */
    int waiting_time =
        current_time - p->arrival_time;

    /*
     * Every aging_threshold time units,
     * improve the effective priority by 1.
     */
    int aging_bonus =
        waiting_time / aging_threshold;

    priority -= aging_bonus;

    /*
     * Priority cannot become less than 1.
     */
    if (priority < 1) {
        priority = 1;
    }

    return priority;
}

/*
 * Decide whether candidate should be selected
 * instead of current.
 *
 * Scheduling rules:
 *
 * 1. Lower priority number = higher priority.
 * 2. If priority is equal, earlier arrival time wins.
 * 3. If arrival time is also equal, process ID wins.
 */
static int is_better_process(
    const Process *candidate,
    const Process *current,
    int current_time,
    int aging_enabled,
    int aging_threshold
) {
    int candidate_priority =
        effective_priority(
            candidate,
            current_time,
            aging_enabled,
            aging_threshold
        );

    int current_priority =
        effective_priority(
            current,
            current_time,
            aging_enabled,
            aging_threshold
        );

    /*
     * Compare priority first.
     */
    if (candidate_priority != current_priority) {
        return candidate_priority < current_priority;
    }

    /*
     * If priority is equal, compare arrival time.
     */
    if (candidate->arrival_time != current->arrival_time) {
        return candidate->arrival_time < current->arrival_time;
    }

    /*
     * If everything is equal, compare process ID.
     */
    return strcmp(
        candidate->id,
        current->id
    ) < 0;
}

/*
 * Reset all scheduling-related values.
 */
void reset_processes(
    Process processes[],
    int count
) {
    for (int i = 0; i < count; i++) {

        processes[i].remaining_time =
            processes[i].burst_time;

        processes[i].completion_time = 0;
        processes[i].turnaround_time = 0;
        processes[i].waiting_time = 0;
        processes[i].response_time = 0;

        processes[i].first_start_time = -1;

        processes[i].state = READY;
        processes[i].completed = 0;
    }
}

/*
 * ---------------------------------------------------------
 * NON-PREEMPTIVE PRIORITY SCHEDULING
 * ---------------------------------------------------------
 *
 * In non-preemptive priority scheduling:
 *
 * - Select the highest-priority ready process.
 * - Once it starts, it runs until its burst is complete.
 * - A newly arrived process cannot interrupt it.
 */
void non_preemptive_priority(
    Process processes[],
    int count,
    int aging_enabled,
    int aging_threshold,
    ScheduleResult *result
) {
    int current_time = 0;
    int completed = 0;

    result->segment_count = 0;

    reset_processes(
        processes,
        count
    );

    while (completed < count) {

        int selected = -1;

        /*
         * Find the best process that has already arrived.
         */
        for (int i = 0; i < count; i++) {

            if (processes[i].completed) {
                continue;
            }

            if (processes[i].arrival_time > current_time) {
                continue;
            }

            if (selected == -1 ||
                is_better_process(
                    &processes[i],
                    &processes[selected],
                    current_time,
                    aging_enabled,
                    aging_threshold
                )) {

                selected = i;
            }
        }

        /*
         * No process is ready.
         * Move the clock to the next arrival time.
         */
        if (selected == -1) {

            int next_arrival = -1;

            for (int i = 0; i < count; i++) {

                if (processes[i].completed) {
                    continue;
                }

                if (next_arrival == -1 ||
                    processes[i].arrival_time < next_arrival) {

                    next_arrival =
                        processes[i].arrival_time;
                }
            }

            /*
             * Record CPU idle time.
             */
            if (next_arrival > current_time) {

                add_segment(
                    result,
                    current_time,
                    next_arrival,
                    "IDLE"
                );

                current_time = next_arrival;
            }

            continue;
        }

        Process *p =
            &processes[selected];

        /*
         * Process enters Running state.
         */
        p->state = RUNNING;

        /*
         * Record first CPU start time.
         */
        if (p->first_start_time == -1) {

            p->first_start_time =
                current_time;

            p->response_time =
                current_time - p->arrival_time;
        }

        int start =
            current_time;

        /*
         * Non-preemptive scheduling:
         * execute the entire burst.
         */
        current_time +=
            p->burst_time;

        p->remaining_time = 0;

        /*
         * Calculate completion time.
         */
        p->completion_time =
            current_time;

        /*
         * Turnaround Time =
         * Completion Time - Arrival Time
         */
        p->turnaround_time =
            p->completion_time -
            p->arrival_time;

        /*
         * Waiting Time =
         * Turnaround Time - Burst Time
         */
        p->waiting_time =
            p->turnaround_time -
            p->burst_time;

        /*
         * Process has finished.
         */
        p->state = TERMINATED;
        p->completed = 1;

        /*
         * Add process to Gantt chart.
         */
        add_segment(
            result,
            start,
            current_time,
            p->id
        );

        completed++;
    }
}

/*
 * ---------------------------------------------------------
 * PREEMPTIVE PRIORITY SCHEDULING
 * ---------------------------------------------------------
 *
 * In preemptive priority scheduling:
 *
 * - The highest-priority ready process runs.
 * - A newly arrived higher-priority process can
 *   interrupt the currently running process.
 * - Scheduling decisions are made every time unit.
 */
void preemptive_priority(
    Process processes[],
    int count,
    int aging_enabled,
    int aging_threshold,
    ScheduleResult *result
) {
    int current_time = 0;
    int completed = 0;

    result->segment_count = 0;

    reset_processes(
        processes,
        count
    );

    while (completed < count) {

        int selected = -1;

        /*
         * Find the highest-priority ready process.
         */
        for (int i = 0; i < count; i++) {

            if (processes[i].completed) {
                continue;
            }

            if (processes[i].arrival_time > current_time) {
                continue;
            }

            if (processes[i].remaining_time <= 0) {
                continue;
            }

            if (selected == -1 ||
                is_better_process(
                    &processes[i],
                    &processes[selected],
                    current_time,
                    aging_enabled,
                    aging_threshold
                )) {

                selected = i;
            }
        }

        /*
         * No process is ready.
         */
        if (selected == -1) {

            int next_arrival = -1;

            for (int i = 0; i < count; i++) {

                if (processes[i].completed) {
                    continue;
                }

                if (next_arrival == -1 ||
                    processes[i].arrival_time < next_arrival) {

                    next_arrival =
                        processes[i].arrival_time;
                }
            }

            /*
             * CPU remains idle until the next process arrives.
             */
            if (next_arrival > current_time) {

                add_segment(
                    result,
                    current_time,
                    next_arrival,
                    "IDLE"
                );

                current_time =
                    next_arrival;
            }

            continue;
        }

        /*
         * Any previously running process that was not selected
         * becomes Ready because it has been preempted.
         */
        for (int i = 0; i < count; i++) {

            if (i != selected &&
                processes[i].state == RUNNING &&
                !processes[i].completed) {

                processes[i].state = READY;
            }
        }

        Process *p =
            &processes[selected];

        /*
         * Selected process enters Running state.
         */
        p->state = RUNNING;

        /*
         * Record the first time the process receives CPU time.
         */
        if (p->first_start_time == -1) {

            p->first_start_time =
                current_time;

            p->response_time =
                current_time -
                p->arrival_time;
        }

        /*
         * Execute one time unit.
         */
        add_segment(
            result,
            current_time,
            current_time + 1,
            p->id
        );

        p->remaining_time--;

        current_time++;

        /*
         * Check whether the process has completed.
         */
        if (p->remaining_time == 0) {

            p->completion_time =
                current_time;

            /*
             * Turnaround Time =
             * Completion Time - Arrival Time
             */
            p->turnaround_time =
                p->completion_time -
                p->arrival_time;

            /*
             * Waiting Time =
             * Turnaround Time - Burst Time
             */
            p->waiting_time =
                p->turnaround_time -
                p->burst_time;

            p->state = TERMINATED;
            p->completed = 1;

            completed++;
        }
    }
}

/*
 * Display the final state of every process.
 */
void display_process_states(
    Process processes[],
    int count
) {
    printf("\nProcess States\n");
    printf("------------------------------\n");

    for (int i = 0; i < count; i++) {

        const char *state_name;

        switch (processes[i].state) {

            case READY:
                state_name = "Ready";
                break;

            case RUNNING:
                state_name = "Running";
                break;

            case TERMINATED:
                state_name = "Terminated";
                break;

            default:
                state_name = "Unknown";
        }

        printf(
            "%-8s : %s\n",
            processes[i].id,
            state_name
        );
    }
}

/*
 * Display the order in which processes execute.
 */
void display_execution_order(
    ScheduleResult *result
) {
    printf("\nExecution Order\n");
    printf("------------------------------\n");

    char last_id[ID_LENGTH] = "";

    for (int i = 0;
         i < result->segment_count;
         i++) {

        /*
         * Print a process only when it changes.
         */
        if (strcmp(
                last_id,
                result->segments[i].process_id
            ) != 0) {

            if (last_id[0] != '\0') {
                printf(" -> ");
            }

            printf(
                "%s",
                result->segments[i].process_id
            );

            strncpy(
                last_id,
                result->segments[i].process_id,
                ID_LENGTH - 1
            );

            last_id[ID_LENGTH - 1] = '\0';
        }
    }

    printf("\n");
}

/*
 * Display a simple text-based Gantt chart.
 */
void display_gantt_chart(
    ScheduleResult *result
) {
    printf("\nGantt Chart\n");
    printf("--------------------------------------------------\n");

    /*
     * Display process names.
     */
    for (int i = 0;
         i < result->segment_count;
         i++) {

        printf(
            "| %-8s ",
            result->segments[i].process_id
        );
    }

    printf("|\n");

    /*
     * Display starting times.
     */
    for (int i = 0;
         i < result->segment_count;
         i++) {

        printf(
            "%-11d",
            result->segments[i].start_time
        );
    }

    /*
     * Display final ending time.
     */
    if (result->segment_count > 0) {

        printf(
            "%d\n",
            result->segments[
                result->segment_count - 1
            ].end_time
        );
    }

    printf("--------------------------------------------------\n");
}
