#include "schedule.h"

void start_scheduling(process_t **process, char *scheduler, 
                      int num, int quantum) {

    // determine scheduler
    if (strcmp(scheduler, "SJF") == 0) {
        do_sjf(process, num, quantum);
    } else if (strcmp(scheduler, "RR") == 0) {
        do_rr(process, num, quantum);
    }

}

// Run processes in Shortest Job First
void do_sjf(process_t **p, int n, int q) {

    // initiate current time from 0
    // assume all processes are not finished yet
    int current_time = 0;
    int is_finished[n];
    memset(is_finished, 0, sizeof(is_finished));

    // sort processes ascending by service time
    qsort(p, n, sizeof(*p), compare_time);

    // run in sjf scheduling
    for (int i = 0; i < n; i++) {
        int j;
        for (j = 0; j < n; j++) {
            if (get_arrival_time(p[j]) <= current_time && is_finished[j] == 0) {
                printf("%d,RUNNING,process_name=%s,remaining_time=%d\n", 
                        current_time, 
                        get_process_name(p[j]), 
                        get_service_time(p[j]));
                current_time += get_service_time(p[j]);
                is_finished[j] = 1;
                break;
            }
        }

        // avoid readings beyond the array
        if (j == n) break;

        // check reamining ready process
        int remain = 0;
        for (int k = 0; k < n; k++) {
            if (is_finished[k] == 0) {

                // process may ready only when it arrives ahead
                if (get_arrival_time(p[k]) < current_time) {

                    // process ready when meet current or previous quantum
                    if ((get_arrival_time(p[k]) % q == 0) || 
                        (get_arrival_time(p[k]) / q == current_time / q - 1 && current_time % q != 0) ||
                        (get_arrival_time(p[k]) / q < current_time / q - 1)) {
                        remain++;
                    }
                }
            }
        }

        // print out finished log
        while (current_time % q != 0) current_time++;
        printf("%d,FINISHED,process_name=%s,proc_remaining=%d\n", 
                current_time, 
                get_process_name(p[j]), 
                remain);
    }

}

void do_rr(process_t **p, int num, int q) {

}

// Compare service time of candidate processes
int compare_time(const void *a, const void *b) {

    process_t *p1 = *(process_t **)a;
    process_t *p2 = *(process_t **)b;

    if (get_service_time(p1) < get_service_time(p2)) {
        return -1;
    } else if (get_service_time(p1) > get_service_time(p2)) {
        return 1;
    } else {
        return 0;
    }

}