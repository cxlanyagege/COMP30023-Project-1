#include "schedule.h"

struct node
{
    process_t *process;
    int remain_time;
    node_t *next;
};

struct queue
{
    node_t *head;
    node_t *tail;
};



void start_scheduling(process_t **process, char *scheduler, 
                      int num, int quantum, char *mem_strategy) {

    // initiate current time from 0
    // assume all processes are not finished yet
    int current_time = 0;
    int is_finished[num];
    memset(is_finished, 0, sizeof(is_finished));

    // determine scheduler
    if (strcmp(scheduler, "SJF") == 0) {
        do_sjf(process, num, quantum, current_time, is_finished, 
               mem_strategy);
    } else if (strcmp(scheduler, "RR") == 0) {
        do_rr(process, num, quantum, current_time, is_finished, 
              mem_strategy);
    }

}

// Run processes in Shortest Job First
void do_sjf(process_t **p, int n, int q, int time, 
            int *is_finished, char *strategy) {

    // sort processes ascending by service time
    qsort(p, n, sizeof(*p), compare_service_time);

    // create and initialize mem allocation
    int use_strategy = 0;
    int mem_allocated[n];
    memory_t **memory;
    if (strcmp(strategy, "best-fit") == 0) {
        use_strategy = 1;
        memory = create_mem_table();
        memset(mem_allocated, 0, sizeof(mem_allocated));
    }

    // run in sjf scheduling
    for (int i = 0; i < n; i++) {
        int process_running = 0;
        int j, memstart;
        for (j = 0; j < n; j++) {
            if (get_arrival_time(p[j]) <= time && 
                is_finished[j] == 0) {

                if (use_strategy) {
                    if (!mem_allocated[j]) {
                        memstart = allocate_mem(memory, get_process_mem(p[j]));
                        print_ready_msg(time, get_process_name(p[j]), 
                                        memstart);
                        mem_allocated[j] = 1;
                    }
                }

                process_running = 1;
                print_running_msg(time, get_service_time(p[j]), 
                                  get_process_name(p[j]));
                time += get_service_time(p[j]);
                is_finished[j] = 1;
                break;
            }
        }

        if (!process_running) {
            time++;
            i--;
            continue;
        }

        // avoid readings beyond the array
        if (j == n) break;

        // print out finished result
        while (time % q != 0) time++;

        if (use_strategy) {
        int count = 0;
        int *original_index = malloc(n * sizeof(int));
        process_t **runtime = malloc(n * sizeof(process_t *));
        for (int k = 0; k < n; k++) {
            if (mem_allocated[k]) continue;

            if (get_arrival_time(p[k]) <= time - q) {
                runtime[count] = p[k];
                original_index[count] = k;
                count++;
            }
        }

        //runtime = realloc(runtime, count * sizeof(int));
        qsort(runtime, count, sizeof(*runtime), compare_arrival_time);
        for (int x = 0; x < count; x++) {
            print_ready_msg(get_arrival_time(runtime[x]), get_process_name(runtime[x]), 
                            allocate_mem(memory, get_process_mem(runtime[x])));
            mem_allocated[original_index[x]] = 1;
        }

        clear_mem(memory, memstart, get_process_mem(p[j]));

        }

        print_result_msg(n, q, time, p, is_finished, 
                         get_process_name(p[j]));
                         
        
    }

}

void do_rr(process_t **p, int n, int q, int time, 
           int *is_finished, char *strategy) {

    // remain time for each process each round
    int remain_time[n];
    int all_finished = 0;

    // create and initialize mem allocation
    int use_strategy = 0;
    int mem_allocated[n];
    memory_t **memory;
    if (strcmp(strategy, "best-fit") == 0) {
        use_strategy = 1;
        memory = create_mem_table();
        memset(mem_allocated, 0, sizeof(mem_allocated));
    }

    // service time as remain time at beginning
    for (int i = 0; i < n; i++) {
        remain_time[i] = get_service_time(p[i]);
    }

    // run in rr scheduling
    char *last_process = NULL;

    while (all_finished == 0) {

        int memstart[n];

        // run each process on given quantum
        for (int i = 0; i < n; i++) {
            if (get_arrival_time(p[i]) <= time && 
                is_finished[i] == 0) {

                if (use_strategy) {
                    for (int j = 0; j < n; j++) {
                        if (!mem_allocated[j] && get_arrival_time(p[j]) <= time && is_finished[i] == 0) {
                            memstart[j] = allocate_mem(memory, get_process_mem(p[j]));

                            if (memstart[j] != -1) {
                                print_ready_msg(time, get_process_name(p[j]), 
                                                memstart[j]);
                                mem_allocated[j] = 1;
                            }
                        }
                    }
                } else {
                    for (int j = 0; j < n; j ++) {
                        mem_allocated[j] = 1;
                    }
                }

                if (mem_allocated[i] && (last_process == NULL || 
                    strcmp(last_process, get_process_name(p[i])) != 0)) {
                    print_running_msg(time, remain_time[i],
                                      get_process_name(p[i]));
                    last_process = get_process_name(p[i]);
                } else if (!mem_allocated[i]) {
                    time -= q;
                    remain_time[i] += q;
                }
                
                time += q;
                remain_time[i] -= q;

                if (remain_time[i] <= 0) {
                    is_finished[i] = 1;       

                    print_result_msg(n, q, time, p, is_finished, 
                                     get_process_name(p[i]));
                    
                    if (use_strategy) {
                        clear_mem(memory, memstart[i], get_process_mem(p[i]));
                        break;
                    }
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


// Compare arrival time of candidate processes
int compare_arrival_time(const void *a, const void *b) {

    process_t *p1 = *(process_t **)a;
    process_t *p2 = *(process_t **)b;

    if (get_arrival_time(p1) < get_arrival_time(p2)) {
        return -1;
    } else if (get_arrival_time(p1) > get_arrival_time(p2)) {
        return 1;
    } else {
        return 0;
    }

}

// Compare service time of candidate processes
int compare_service_time(const void *a, const void *b) {

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
            time, name, 
            check_proc_remaining(n, q, time, p, is_finished));
}

void print_ready_msg(int time, char *name, int memstart) {
    printf("%d,READY,process_name=%s,assigned_at=%d\n", 
            time, name, memstart);
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