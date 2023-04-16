#include "schedule.h"

struct child {

    int pipe_to_child[2];
    int pipe_from_child[2];
    pid_t pid;
    uint32_t simulation_time_big_endian;

    int in_use;
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

    // 
    for (int i = 0; i < num; i++) {
        free(process[i]);
    }
    free(process);

}

// Run processes in Shortest Job First
void do_sjf(process_t **p, int n, int q, int time, 
            int *is_finished, char *strategy) {

    int turnaround = 0;
    double max_overhead = 0.0;
    double total_overhead = 0.0;


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

        int pipe_to_child[2];
        int pipe_from_child[2];
        pipe(pipe_to_child);
        pipe(pipe_from_child);

        pid_t pid;
        uint32_t simulation_time_big_endian;

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

                pid = fork();

                if (pid == 0) {

                    dup2(pipe_to_child[0], STDIN_FILENO);
                    dup2(pipe_from_child[1], STDOUT_FILENO);
                    close(pipe_to_child[1]);
                    close(pipe_from_child[0]);

                    // debug
                    //char *pargv[] = {"./process", "-v", get_process_name(p[j]), NULL};

                    // release
                    char *pargv[] = {"./process", get_process_name(p[j]), NULL};
                    execvp(pargv[0], pargv);

                } else {

                    close(pipe_to_child[0]);
                    close(pipe_from_child[1]);

                    simulation_time_big_endian = htonl(time);
                    write(pipe_to_child[1], &simulation_time_big_endian, sizeof(uint32_t));

                    uint8_t response;
                    read(pipe_from_child[0], &response, sizeof(response));

                    if (response != (time & 0xFF));
                    

                }

                process_running = 1;
                print_running_msg(time, get_service_time(p[j]), 
                                  get_process_name(p[j]));

                int start_time = time;

                time += q;

                while (time - start_time < get_service_time(p[j])) {
                    simulation_time_big_endian = htonl(time);

                    // 
                    // write(pipe_to_child[1], &simulation_time_big_endian, sizeof(uint32_t));
                    // kill(pid, SIGTSTP);

                    // int wstatus = 0;
                    // pid_t w = waitpid(pid, &wstatus, WUNTRACED);
                    // if (!WIFSTOPPED(wstatus)) {
                    //     while (!WIFSTOPPED(wstatus)) {
                    //         waitpid(pid, &wstatus, WUNTRACED);
                    //     }
                    // }

                    //printf("%d\n", p);

                    write(pipe_to_child[1], &simulation_time_big_endian, sizeof(uint32_t));
                    kill(pid, SIGCONT);
                    //sleep(1);

                    uint8_t response;
                    read(pipe_from_child[0], &response, sizeof(response));

                    if (response != (time & 0xFF));

                    time += q;
                }

                //time += get_service_time(p[j]);
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

        simulation_time_big_endian = htonl(time);
        write(pipe_to_child[1], &simulation_time_big_endian, sizeof(uint32_t));
        kill(pid, SIGTERM);

        // wait child to terminate
        int status;
        waitpid(pid, &status, 0);

        // read 64-byte string from child 
        char sha[65];
        read(pipe_from_child[0], sha, 64);
        sha[64] = '\0';

        // print
        printf("%d,FINISHED-PROCESS,process_name=%s,sha=%s\n", 
            time, get_process_name(p[j]), 
            sha);

        close(pipe_to_child[1]);
        close(pipe_from_child[0]);
        

        turnaround += (time - get_arrival_time(p[j]));

        total_overhead += (double)((time - get_arrival_time(p[j])) / (double)(get_service_time(p[j])));
        if ((double)(time - get_arrival_time(p[j])) / (get_service_time(p[j])) > max_overhead) {
            max_overhead = (double)(time - get_arrival_time(p[j])) / (get_service_time(p[j]));
        }
        
    }

    if (turnaround % n != 0) {
        turnaround /= n;
        turnaround ++;
    } else {
        turnaround /= n;
    }
    printf("Turnaround time %d\nTime overhead %.2lf %.2lf\nMakespan %d\n", 
            turnaround, round(max_overhead * 100) / 100, round(total_overhead * 100 / n) / 100, time);

}

void do_rr(process_t **p, int n, int q, int time, 
           int *is_finished, char *strategy) {


    int turnaround = 0;
    double max_overhead = 0.0;
    double total_overhead = 0.0;

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

    // initialize pipeline
    child_t **child = malloc(n * sizeof(child_t *));
    for (int i = 0; i < n; i++) {
        child[i] = malloc(sizeof(child_t));

        pipe(child[i]->pipe_from_child);
        pipe(child[i]->pipe_to_child);

        child[i]->in_use = 0;
    }

    // run in rr scheduling
    char *last_process = NULL;
    int last_process_index = 0;

    while (all_finished == 0) {

        int memstart[n];

        // run each process on given quantum
        for (int i = 0; i < n; i++) {
            if (get_arrival_time(p[i]) <= time && 
                is_finished[i] == 0) {

                // when using best-fit
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

                        if (last_process != NULL && child[last_process_index]->in_use) {
                            child[last_process_index]->simulation_time_big_endian = htonl(time);

                        write(child[last_process_index]->pipe_to_child[1], &child[last_process_index]->simulation_time_big_endian, sizeof(uint32_t));
                        kill(child[last_process_index]->pid, SIGTSTP);

                        int wstatus = 0;
                        waitpid(child[last_process_index]->pid, &wstatus, WUNTRACED);
                        while (!WIFSTOPPED(wstatus)) {
                            waitpid(child[last_process_index]->pid, &wstatus, WUNTRACED);
                        }
                        }

                    // process run first-time
                    if (child[i]->in_use == 0) {

                        child[i]->in_use = 1;
                        child[i]->pid = fork();

                        if (child[i]->pid == 0) {

                            dup2(child[i]->pipe_to_child[0], STDIN_FILENO);
                            dup2(child[i]->pipe_from_child[1], STDOUT_FILENO);
                            close(child[i]->pipe_to_child[1]);
                            close(child[i]->pipe_from_child[0]);

                            // debug
                            //char *pargv[] = {"./process", "-v", get_process_name(p[i]), NULL};

                            // release
                            char *pargv[] = {"./process", get_process_name(p[i]), NULL};
                            execvp(pargv[0], pargv);


                        } else {

                            close(child[i]->pipe_to_child[0]);
                            close(child[i]->pipe_from_child[1]);

                            child[i]->simulation_time_big_endian = htonl(time);
                            write(child[i]->pipe_to_child[1], &child[i]->simulation_time_big_endian, sizeof(uint32_t));

                            uint8_t response;
                            read(child[i]->pipe_from_child[0], &response, sizeof(response));

                            if (response != (time & 0xFF));
                    

                        }

                        print_running_msg(time, remain_time[i],
                                          get_process_name(p[i]));
                                      

                    } else {

                        

                        //printf("%d\n", p);

                        child[i]->simulation_time_big_endian = htonl(time);

                        write(child[i]->pipe_to_child[1], &child[i]->simulation_time_big_endian, sizeof(uint32_t));
                        kill(child[i]->pid, SIGCONT);
                        //sleep(1);

                        uint8_t response;
                        read(child[i]->pipe_from_child[0], &response, sizeof(response));

                        if (response != (time & 0xFF));

                        print_running_msg(time, remain_time[i],
                                          get_process_name(p[i]));
                                      

                    }

                    last_process = get_process_name(p[i]);
                    last_process_index = i;

                } else if (mem_allocated[i] && strcmp(last_process, get_process_name(p[i])) == 0) {

                    child[i]->simulation_time_big_endian = htonl(time);

                    write(child[i]->pipe_to_child[1], &child[i]->simulation_time_big_endian, sizeof(uint32_t));
                    kill(child[i]->pid, SIGCONT);
                    //sleep(1);

                    uint8_t response;
                    read(child[i]->pipe_from_child[0], &response, sizeof(response));

                    if (response != (time & 0xFF));

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

                    turnaround += (time - get_arrival_time(p[i]));

                    total_overhead += (double)((time - get_arrival_time(p[i])) / (double)(get_service_time(p[i])));

        child[i]->simulation_time_big_endian = htonl(time);
        write(child[i]->pipe_to_child[1], &child[i]->simulation_time_big_endian, sizeof(uint32_t));
        kill(child[i]->pid, SIGTERM);

        // wait child to terminate
        int status;
        waitpid(child[i]->pid, &status, 0);

        // read 64-byte string from child 
        char sha[65];
        read(child[i]->pipe_from_child[0], sha, 64);
        sha[64] = '\0';

        // print
        printf("%d,FINISHED-PROCESS,process_name=%s,sha=%s\n", 
            time, get_process_name(p[i]), 
            sha);

        close(child[i]->pipe_to_child[1]);
        close(child[i]->pipe_from_child[0]);

        child[i]->in_use = 0;

                    if ((double)(time - get_arrival_time(p[i])) / (get_service_time(p[i])) > max_overhead) {
                        max_overhead = (double)(time - get_arrival_time(p[i])) / (get_service_time(p[i]));
                    }
                    
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

    if (turnaround % n != 0) {
        turnaround /= n;
        turnaround ++;
    } else {
        turnaround /= n;
    }
    printf("Turnaround time %d\nTime overhead %.2lf %.2lf\nMakespan %d\n", 
            turnaround, round(max_overhead * 100) / 100, round(total_overhead / n * 100) / 100, time);

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