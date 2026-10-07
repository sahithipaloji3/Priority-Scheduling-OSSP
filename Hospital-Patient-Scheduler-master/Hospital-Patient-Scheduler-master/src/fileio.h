#ifndef FILEIO_H
#define FILEIO_H

#include "scheduler.h"

int read_processes_from_file(
    const char *filename,
    Process processes[],
    int *count
);

int read_processes_from_keyboard(
    Process processes[],
    int *count
);

int read_processes_without_arrival_time(
    Process processes[],
    int *count
);

int write_results_to_file(
    const char *filename,
    Process processes[],
    int count,
    ScheduleResult *result,
    const char *algorithm
);

#endif
