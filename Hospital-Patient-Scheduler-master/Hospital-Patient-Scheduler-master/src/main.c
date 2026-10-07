#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "scheduler.h"
#include "fileio.h"
#include "metrics.h"

static void print_title(void)
{
    printf("\n");
    printf("========================================\n");
    printf("       HOSPITAL PATIENT SCHEDULER\n");
    printf("========================================\n");
}

static void copy_processes(
    Process destination[],
    Process source[],
    int count
)
{
    for (int i = 0; i < count; i++) {
        destination[i] = source[i];
    }
}

static int get_main_menu_choice(void)
{
    int choice;

    print_title();

    printf("\n");
    printf("1. Non-Preemptive Priority\n");
    printf("2. Preemptive Priority\n");
    printf("3. Compare Both\n");
    printf("4. Exit\n");

    printf("\nEnter choice: ");

    if (scanf("%d", &choice) != 1) {
        while (getchar() != '\n');
        return -1;
    }

    return choice;
}

static int get_non_preemptive_input_choice(void)
{
    int choice;

    printf("\n");
    printf("========================================\n");
    printf("      NON-PREEMPTIVE PRIORITY\n");
    printf("========================================\n");

    printf("\n");
    printf("1. With Arrival Time\n");
    printf("2. Without Arrival Time\n");
    printf("3. Back to Main Menu\n");

    printf("\nEnter choice: ");

    if (scanf("%d", &choice) != 1) {
        while (getchar() != '\n');
        return -1;
    }

    return choice;
}

static int get_aging_choice(
    int *aging_enabled,
    int *aging_threshold
)
{
    int choice;

    printf("\n");
    printf("Aging Configuration\n");
    printf("-------------------\n");
    printf("1. Disable Aging\n");
    printf("2. Enable Aging\n");

    printf("\nEnter choice: ");

    if (scanf("%d", &choice) != 1) {
        while (getchar() != '\n');
        return -1;
    }

    if (choice == 1) {

        *aging_enabled = 0;

    } else if (choice == 2) {

        *aging_enabled = 1;

        printf("Enter aging threshold: ");

        if (scanf(
                "%d",
                aging_threshold
            ) != 1) {

            while (getchar() != '\n');

            printf(
                "Invalid aging threshold.\n"
            );

            return -1;
        }

        if (*aging_threshold <= 0) {

            printf(
                "Aging threshold must be greater than 0.\n"
            );

            return -1;
        }

    } else {

        printf("Invalid choice.\n");
        return -1;
    }

    return 0;
}

static void run_non_preemptive(
    Process original[],
    int count,
    int aging_enabled,
    int aging_threshold,
    const char *output_file
)
{
    Process processes[MAX_PROCESSES];

    ScheduleResult result;

    copy_processes(
        processes,
        original,
        count
    );

    non_preemptive_priority(
        processes,
        count,
        aging_enabled,
        aging_threshold,
        &result
    );

    printf("\n");
    printf("========================================\n");
    printf("  NON-PREEMPTIVE PRIORITY RESULTS\n");
    printf("========================================\n");

    display_process_states(
        processes,
        count
    );

    display_execution_order(
        &result
    );

    display_gantt_chart(
        &result
    );

    Metrics metrics =
        calculate_metrics(
            processes,
            count
        );

    display_results(
        processes,
        count,
        metrics
    );

    if (output_file != NULL) {

        if (write_results_to_file(
                output_file,
                processes,
                count,
                &result,
                "Non-Preemptive Priority"
            ) == 0) {

            printf(
                "\nResults written to: %s\n",
                output_file
            );
        }
    }
}

static Metrics run_preemptive(
    Process original[],
    int count,
    int aging_enabled,
    int aging_threshold,
    const char *output_file
)
{
    Process processes[MAX_PROCESSES];

    ScheduleResult result;

    copy_processes(
        processes,
        original,
        count
    );

    preemptive_priority(
        processes,
        count,
        aging_enabled,
        aging_threshold,
        &result
    );

    printf("\n");
    printf("========================================\n");
    printf("    PREEMPTIVE PRIORITY RESULTS\n");
    printf("========================================\n");

    display_process_states(
        processes,
        count
    );

    display_execution_order(
        &result
    );

    display_gantt_chart(
        &result
    );

    Metrics metrics =
        calculate_metrics(
            processes,
            count
        );

    display_results(
        processes,
        count,
        metrics
    );

    if (output_file != NULL) {

        if (write_results_to_file(
                output_file,
                processes,
                count,
                &result,
                "Preemptive Priority"
            ) == 0) {

            printf(
                "\nResults written to: %s\n",
                output_file
            );
        }
    }

    return metrics;
}

static void run_comparison(
    Process original[],
    int count,
    int aging_enabled,
    int aging_threshold,
    const char *output_file
)
{
    Process np_processes[MAX_PROCESSES];
    Process p_processes[MAX_PROCESSES];

    ScheduleResult np_result;
    ScheduleResult p_result;

    copy_processes(
        np_processes,
        original,
        count
    );

    copy_processes(
        p_processes,
        original,
        count
    );

    non_preemptive_priority(
        np_processes,
        count,
        aging_enabled,
        aging_threshold,
        &np_result
    );

    preemptive_priority(
        p_processes,
        count,
        aging_enabled,
        aging_threshold,
        &p_result
    );

    Metrics np_metrics =
        calculate_metrics(
            np_processes,
            count
        );

    Metrics p_metrics =
        calculate_metrics(
            p_processes,
            count
        );

    printf("\n");
    printf("========================================\n");
    printf("       SCHEDULING COMPARISON\n");
    printf("========================================\n");

    printf("\n");
    printf("----- NON-PREEMPTIVE -----\n");

    display_execution_order(
        &np_result
    );

    display_gantt_chart(
        &np_result
    );

    display_results(
        np_processes,
        count,
        np_metrics
    );

    printf("\n");
    printf("----- PREEMPTIVE -----\n");

    display_execution_order(
        &p_result
    );

    display_gantt_chart(
        &p_result
    );

    display_results(
        p_processes,
        count,
        p_metrics
    );

    display_comparison(
        np_metrics,
        p_metrics
    );

    if (output_file != NULL) {

        if (write_results_to_file(
                output_file,
                p_processes,
                count,
                &p_result,
                "Comparison - Preemptive Results"
            ) == 0) {

            printf(
                "\nComparison output written to: %s\n",
                output_file
            );
        }
    }
}

static int get_output_file(
    char **output_file
)
{
    int choice;

    printf("\n");
    printf("Save results to output file?\n");
    printf("1. No\n");
    printf("2. Yes\n");

    printf("\nEnter choice: ");

    if (scanf("%d", &choice) != 1) {

        while (getchar() != '\n');
        return -1;
    }

    if (choice == 1) {

        *output_file = NULL;
        return 0;
    }

    if (choice == 2) {

        static char filename[256];

        printf("Enter output file name: ");

        if (scanf(
                "%255s",
                filename
            ) != 1) {

            printf(
                "Invalid output file name.\n"
            );

            return -1;
        }

        *output_file = filename;

        return 0;
    }

    printf("Invalid choice.\n");

    return -1;
}

static int get_process_input(
    Process processes[],
    int *count,
    int algorithm_choice
)
{
    int input_choice;

    if (algorithm_choice == 1) {

        input_choice =
            get_non_preemptive_input_choice();

        if (input_choice == 3) {
            return 1;
        }

        if (input_choice == 1) {

            if (read_processes_from_keyboard(
                    processes,
                    count
                ) < 0) {

                return -1;
            }

            return 0;
        }

        if (input_choice == 2) {

            if (read_processes_without_arrival_time(
                    processes,
                    count
                ) < 0) {

                return -1;
            }

            return 0;
        }

        printf("Invalid choice.\n");

        return -1;
    }

    /*
     * Preemptive and comparison require
     * arrival times.
     */
    if (read_processes_from_keyboard(
            processes,
            count
        ) < 0) {

        return -1;
    }

    return 0;
}

int main(void)
{
    Process processes[MAX_PROCESSES];

    int count;

    while (1) {

        int main_choice =
            get_main_menu_choice();

        if (main_choice == 4) {

            printf("\n");
            printf(
                "Thank you for using Hospital Patient Scheduler.\n"
            );
            printf("Program exited.\n\n");

            return EXIT_SUCCESS;
        }

        if (main_choice < 1 ||
            main_choice > 4) {

            printf(
                "\nInvalid menu choice.\n"
            );

            continue;
        }

        /*
         * Non-preemptive has an additional
         * arrival-time choice.
         */
        if (main_choice == 1) {

            while (1) {

                int input_result =
                    get_process_input(
                        processes,
                        &count,
                        1
                    );

                if (input_result == 1) {
                    break;
                }

                if (input_result < 0) {
                    continue;
                }

                int aging_enabled = 0;
                int aging_threshold = 3;

                if (get_aging_choice(
                        &aging_enabled,
                        &aging_threshold
                    ) < 0) {

                    continue;
                }

                char *output_file = NULL;

                if (get_output_file(
                        &output_file
                    ) < 0) {

                    continue;
                }

                run_non_preemptive(
                    processes,
                    count,
                    aging_enabled,
                    aging_threshold,
                    output_file
                );

                break;
            }
        }

        /*
         * Preemptive priority.
         */
        else if (main_choice == 2) {

            int input_result =
                get_process_input(
                    processes,
                    &count,
                    2
                );

            if (input_result < 0) {
                continue;
            }

            int aging_enabled = 0;
            int aging_threshold = 3;

            if (get_aging_choice(
                    &aging_enabled,
                    &aging_threshold
                ) < 0) {

                continue;
            }

            char *output_file = NULL;

            if (get_output_file(
                    &output_file
                ) < 0) {

                continue;
            }

            run_preemptive(
                processes,
                count,
                aging_enabled,
                aging_threshold,
                output_file
            );
        }

        /*
         * Compare both algorithms.
         *
         * Comparison uses arrival times because
         * preemptive priority requires them.
         */
        else if (main_choice == 3) {

            int input_result =
                get_process_input(
                    processes,
                    &count,
                    3
                );

            if (input_result < 0) {
                continue;
            }

            int aging_enabled = 0;
            int aging_threshold = 3;

            if (get_aging_choice(
                    &aging_enabled,
                    &aging_threshold
                ) < 0) {

                continue;
            }

            char *output_file = NULL;

            if (get_output_file(
                    &output_file
                ) < 0) {

                continue;
            }

            run_comparison(
                processes,
                count,
                aging_enabled,
                aging_threshold,
                output_file
            );
        }
    }
}
