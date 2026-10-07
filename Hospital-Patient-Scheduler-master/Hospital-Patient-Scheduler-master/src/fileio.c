#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#include "fileio.h"

static int write_all(
    int fd,
    const char *buffer,
    size_t length
)
{
    size_t total = 0;

    while (total < length) {

        ssize_t written =
            write(
                fd,
                buffer + total,
                length - total
            );

        if (written < 0) {
            perror("write");
            return -1;
        }

        total += (size_t)written;
    }

    return 0;
}

static int process_id_exists(
    Process processes[],
    int count,
    const char *id
)
{
    for (int i = 0; i < count; i++) {

        if (strcmp(
                processes[i].id,
                id
            ) == 0) {

            return 1;
        }
    }

    return 0;
}


/*
 * Read processes from a file.
 *
 * Format:
 * ID ArrivalTime BurstTime Priority
 */
int read_processes_from_file(
    const char *filename,
    Process processes[],
    int *count
)
{
    int fd = open(filename, O_RDONLY);

    if (fd < 0) {
        perror("open input file");
        return -1;
    }

    size_t capacity = 4096;
    size_t length = 0;

    char *buffer = malloc(capacity);

    if (buffer == NULL) {
        perror("malloc");
        close(fd);
        return -1;
    }

    while (1) {

        if (length + 1024 >= capacity) {

            capacity *= 2;

            char *new_buffer =
                realloc(buffer, capacity);

            if (new_buffer == NULL) {
                perror("realloc");
                free(buffer);
                close(fd);
                return -1;
            }

            buffer = new_buffer;
        }

        ssize_t bytes_read =
            read(
                fd,
                buffer + length,
                1024
            );

        if (bytes_read < 0) {
            perror("read");
            free(buffer);
            close(fd);
            return -1;
        }

        if (bytes_read == 0) {
            break;
        }

        length += (size_t)bytes_read;
    }

    if (close(fd) < 0) {
        perror("close input file");
        free(buffer);
        return -1;
    }

    buffer[length] = '\0';

    *count = 0;

    char *line = strtok(buffer, "\n");

    while (line != NULL) {

        char *ptr = line;

        while (*ptr == ' ' ||
               *ptr == '\t' ||
               *ptr == '\r') {
            ptr++;
        }

        if (*ptr != '\0' &&
            *ptr != '#') {

            char id[ID_LENGTH];

            int arrival;
            int burst;
            int priority;
            char extra;

            int fields =
                sscanf(
                    ptr,
                    "%19s %d %d %d %c",
                    id,
                    &arrival,
                    &burst,
                    &priority,
                    &extra
                );

            if (fields != 4) {

                fprintf(
                    stderr,
                    "Invalid input line: %s\n",
                    line
                );

                free(buffer);
                return -1;
            }

            if (*count >= MAX_PROCESSES) {

                fprintf(
                    stderr,
                    "Too many processes. Maximum is %d.\n",
                    MAX_PROCESSES
                );

                free(buffer);
                return -1;
            }

            if (arrival < 0) {

                fprintf(
                    stderr,
                    "Arrival time cannot be negative: %s\n",
                    id
                );

                free(buffer);
                return -1;
            }

            if (burst <= 0) {

                fprintf(
                    stderr,
                    "Burst time must be positive: %s\n",
                    id
                );

                free(buffer);
                return -1;
            }

            if (priority <= 0) {

                fprintf(
                    stderr,
                    "Priority must be positive: %s\n",
                    id
                );

                free(buffer);
                return -1;
            }

            if (process_id_exists(
                    processes,
                    *count,
                    id
                )) {

                fprintf(
                    stderr,
                    "Duplicate process ID: %s\n",
                    id
                );

                free(buffer);
                return -1;
            }

            Process *p =
                &processes[*count];

            strncpy(
                p->id,
                id,
                ID_LENGTH - 1
            );

            p->id[ID_LENGTH - 1] = '\0';

            p->arrival_time = arrival;
            p->burst_time = burst;
            p->priority = priority;

            (*count)++;
        }

        line = strtok(NULL, "\n");
    }

    free(buffer);

    if (*count == 0) {

        fprintf(
            stderr,
            "No valid processes found in input file.\n"
        );

        return -1;
    }

    return 0;
}


/*
 * Runtime input WITH arrival time.
 *
 * Used by:
 * - Preemptive Priority
 * - Comparison
 * - Non-Preemptive with arrival time
 */
int read_processes_from_keyboard(
    Process processes[],
    int *count
)
{
    int n;

    printf("\n");
    printf("========================================\n");
    printf("          PROCESS INPUT\n");
    printf("========================================\n");

    printf(
        "Enter number of processes (1-%d): ",
        MAX_PROCESSES
    );

    if (scanf("%d", &n) != 1) {

        fprintf(
            stderr,
            "Invalid number of processes.\n"
        );

        return -1;
    }

    if (n < 1 || n > MAX_PROCESSES) {

        fprintf(
            stderr,
            "Invalid process count.\n"
        );

        return -1;
    }

    *count = n;

    for (int i = 0; i < n; i++) {

        Process *p = &processes[i];

        printf("\n");
        printf(
            "---------- Process %d ----------\n",
            i + 1
        );

        printf("Enter Process ID: ");

        if (scanf(
                "%19s",
                p->id
            ) != 1) {

            fprintf(
                stderr,
                "Invalid Process ID.\n"
            );

            return -1;
        }

        if (process_id_exists(
                processes,
                i,
                p->id
            )) {

            fprintf(
                stderr,
                "Duplicate Process ID: %s\n",
                p->id
            );

            return -1;
        }

        printf("Enter Arrival Time: ");

        if (scanf(
                "%d",
                &p->arrival_time
            ) != 1) {

            fprintf(
                stderr,
                "Invalid arrival time.\n"
            );

            return -1;
        }

        printf("Enter Burst Time: ");

        if (scanf(
                "%d",
                &p->burst_time
            ) != 1) {

            fprintf(
                stderr,
                "Invalid burst time.\n"
            );

            return -1;
        }

        printf("Enter Priority: ");

        if (scanf(
                "%d",
                &p->priority
            ) != 1) {

            fprintf(
                stderr,
                "Invalid priority.\n"
            );

            return -1;
        }

        if (p->arrival_time < 0) {

            fprintf(
                stderr,
                "Arrival time cannot be negative.\n"
            );

            return -1;
        }

        if (p->burst_time <= 0) {

            fprintf(
                stderr,
                "Burst time must be greater than 0.\n"
            );

            return -1;
        }

        if (p->priority <= 0) {

            fprintf(
                stderr,
                "Priority must be greater than 0.\n"
            );

            return -1;
        }
    }

    return 0;
}


/*
 * Runtime input WITHOUT arrival time.
 *
 * All processes are assumed to arrive at time 0.
 */
int read_processes_without_arrival_time(
    Process processes[],
    int *count
)
{
    int n;

    printf("\n");
    printf("========================================\n");
    printf("   NON-PREEMPTIVE WITHOUT ARRIVAL TIME\n");
    printf("========================================\n");

    printf(
        "Enter number of processes (1-%d): ",
        MAX_PROCESSES
    );

    if (scanf("%d", &n) != 1) {

        fprintf(
            stderr,
            "Invalid number of processes.\n"
        );

        return -1;
    }

    if (n < 1 || n > MAX_PROCESSES) {

        fprintf(
            stderr,
            "Invalid process count.\n"
        );

        return -1;
    }

    *count = n;

    for (int i = 0; i < n; i++) {

        Process *p = &processes[i];

        printf("\n");
        printf(
            "---------- Process %d ----------\n",
            i + 1
        );

        printf("Enter Process ID: ");

        if (scanf(
                "%19s",
                p->id
            ) != 1) {

            fprintf(
                stderr,
                "Invalid Process ID.\n"
            );

            return -1;
        }

        if (process_id_exists(
                processes,
                i,
                p->id
            )) {

            fprintf(
                stderr,
                "Duplicate Process ID: %s\n",
                p->id
            );

            return -1;
        }

        /*
         * No arrival time is requested.
         * All processes arrive at time 0.
         */
        p->arrival_time = 0;

        printf("Enter Burst Time: ");

        if (scanf(
                "%d",
                &p->burst_time
            ) != 1) {

            fprintf(
                stderr,
                "Invalid burst time.\n"
            );

            return -1;
        }

        printf("Enter Priority: ");

        if (scanf(
                "%d",
                &p->priority
            ) != 1) {

            fprintf(
                stderr,
                "Invalid priority.\n"
            );

            return -1;
        }

        if (p->burst_time <= 0) {

            fprintf(
                stderr,
                "Burst time must be greater than 0.\n"
            );

            return -1;
        }

        if (p->priority <= 0) {

            fprintf(
                stderr,
                "Priority must be greater than 0.\n"
            );

            return -1;
        }
    }

    return 0;
}


/*
 * Write results to an output file.
 *
 * Uses:
 * open()
 * write()
 * close()
 */
int write_results_to_file(
    const char *filename,
    Process processes[],
    int count,
    ScheduleResult *result,
    const char *algorithm
)
{
    int fd =
        open(
            filename,
            O_WRONLY | O_CREAT | O_TRUNC,
            0644
        );

    if (fd < 0) {
        perror("open output file");
        return -1;
    }

    char output[32768];

    int used = 0;

    used += snprintf(
        output + used,
        sizeof(output) - (size_t)used,

        "Hospital Patient Scheduler\n"
        "===========================\n"
        "Algorithm: %s\n\n",

        algorithm
    );

    used += snprintf(
        output + used,
        sizeof(output) - (size_t)used,

        "%-8s %-8s %-8s %-8s "
        "%-8s %-10s %-8s %-8s\n",

        "ID",
        "Arrival",
        "Burst",
        "Priority",
        "Complete",
        "Turnaround",
        "Waiting",
        "Response"
    );

    for (int i = 0; i < count; i++) {

        Process *p = &processes[i];

        used += snprintf(
            output + used,
            sizeof(output) - (size_t)used,

            "%-8s %-8d %-8d %-8d "
            "%-8d %-10d %-8d %-8d\n",

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

    used += snprintf(
        output + used,
        sizeof(output) - (size_t)used,

        "\nExecution Order:\n"
    );

    char last_id[ID_LENGTH] = "";

    for (int i = 0;
         i < result->segment_count;
         i++) {

        if (strcmp(
                last_id,
                result->segments[i].process_id
            ) != 0) {

            if (last_id[0] != '\0') {

                used += snprintf(
                    output + used,
                    sizeof(output) - (size_t)used,
                    " -> "
                );
            }

            used += snprintf(
                output + used,
                sizeof(output) - (size_t)used,
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

    used += snprintf(
        output + used,
        sizeof(output) - (size_t)used,

        "\n\nGantt Chart:\n"
    );

    for (int i = 0;
         i < result->segment_count;
         i++) {

        used += snprintf(
            output + used,
            sizeof(output) - (size_t)used,

            "| %-8s ",

            result->segments[i].process_id
        );
    }

    used += snprintf(
        output + used,
        sizeof(output) - (size_t)used,
        "|\n"
    );

    for (int i = 0;
         i < result->segment_count;
         i++) {

        used += snprintf(
            output + used,
            sizeof(output) - (size_t)used,

            "%-11d",

            result->segments[i].start_time
        );
    }

    if (result->segment_count > 0) {

        used += snprintf(
            output + used,
            sizeof(output) - (size_t)used,

            "%d\n",

            result->segments[
                result->segment_count - 1
            ].end_time
        );
    }

    if (write_all(
            fd,
            output,
            (size_t)used
        ) < 0) {

        close(fd);
        return -1;
    }

    if (close(fd) < 0) {

        perror("close output file");
        return -1;
    }

    return 0;
}
