#include "schedule.h"

void start_scheduling(process_t **process, char *scheduler, 
                      int num, int quantum) {

    // initiate current time from 0
    // assume all processes are not finished yet
    int current_time = 0;
    int is_finished[num];
    memset(is_finished, 0, sizeof(is_finished));

    // determine scheduler
    if (strcmp(scheduler, "SJF") == 0) {
        do_sjf(process, num, quantum, current_time, is_finished);
    } else if (strcmp(scheduler, "RR") == 0) {
        do_rr(process, num, quantum, current_time, is_finished);
    }

}

// Run processes in Shortest Job First
void do_sjf(process_t **p, int n, int q, int time, 
            int *is_finished) {

    // sort processes ascending by service time
    qsort(p, n, sizeof(*p), compare_time);

    // run in sjf scheduling
    for (int i = 0; i < n; i++) {
        int j;
        for (j = 0; j < n; j++) {
            if (get_arrival_time(p[j]) <= time && 
                is_finished[j] == 0) {
                print_running_msg(time, get_service_time(p[j]), 
                                  get_process_name(p[j]));
                time += get_service_time(p[j]);
                is_finished[j] = 1;
                break;
            }
        }

        // avoid readings beyond the array
        if (j == n) break;

        // print out finished result
        while (time % q != 0) time++;
        print_result_msg(n, q, time, p, is_finished, 
                         get_process_name(p[j]));
    }

}

void do_rr(process_t **p, int n, int q, int time, 
           int *is_finished) {

    // remain time for each process each round
    int remain_time[n];
    int all_finished = 0;

    // service time as remain time at beginning
    for (int i = 0; i < n; i++) {
        remain_time[i] = get_service_time(p[i]);
    }

    // run in rr scheduling
    char *last_process = NULL;
    while (all_finished == 0) {

        // run each process on given quantum
        for (int i = 0; i < n; i++) {
            if (get_arrival_time(p[i]) <= time && 
                is_finished[i] == 0) {

                if (last_process == NULL || 
                    strcmp(last_process, get_process_name(p[i])) != 0) {
                    print_running_msg(time, remain_time[i],
                                      get_process_name(p[i]));
                    last_process = get_process_name(p[i]);
                }
                
                time += q;
                remain_time[i] -= q;

                if (remain_time[i] <= 0) {
                    is_finished[i] = 1;       

                    print_result_msg(n, q, time, p, is_finished, 
                                     get_process_name(p[i]));
                }

            }
        }

        // check if all processes are complete
        int finished_count = 0;
        for (int i = 0; i < n; i++) {
            if (is_finished[i] == 1) finished_count++;
        }
        all_finished = finished_count == n ? 1 : 0;
    }

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

void print_running_msg(int time, int remain_time, char *name) {
    printf("%d,RUNNING,process_name=%s,remaining_time=%d\n", 
            time, name, remain_time);
}

void print_result_msg(int n, int q, int time, 
                      process_t **p, int *is_finished, char *name) {
    printf("%d,FINISHED,process_name=%s,proc_remaining=%d\n", 
            time, 
            name, 
            check_proc_remaining(n, q, time, p, is_finished));
}

int check_proc_remaining(int n, int q, int time, 
                         process_t **p, int *is_finished) {        

    // check reamining ready process       
    int remain = 0;
    for (int i = 0; i < n; i++) {
        if (is_finished[i] == 0) {

            // process may ready only when it arrives ahead
            if (get_arrival_time(p[i]) < time) {

                // process ready when meet current or previous quantum
                if ((get_arrival_time(p[i]) % q == 0) || 
                    (get_arrival_time(p[i]) / q == time / q - 1 && 
                     time % q != 0) ||
                    (get_arrival_time(p[i]) / q < time / q - 1)) {
                    remain++;
                }
            }
        }
    }

    return remain;
}