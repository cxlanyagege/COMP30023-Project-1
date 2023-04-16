#include "schedule.h"

struct child {

    int in_use;
    int pipe_to_child[2];
    int pipe_from_child[2];

    pid_t pid;
    uint32_t simulation_time_big_endian;

};


// Scheduling mode decision
void start_scheduling(process_t **process, char *scheduler, 
                      int num, int quantum, char *mem_strategy) {

    // initiate current time from 0
    // assume all processes are not finished yet
    int current_time = 0;
    int is_finished[num];
    memset(is_finished, 0, sizeof(is_finished));

    // statistics
    int turnaround = 0;
    double max_overhead = 0.0;
    double total_overhead = 0.0;

    // determine scheduler
    if (strcmp(scheduler, "SJF") == 0) {
        do_sjf(process, num, quantum, &current_time, is_finished, 
               mem_strategy, &turnaround, &max_overhead, &total_overhead);
    } else if (strcmp(scheduler, "RR") == 0) {
        do_rr(process, num, quantum, &current_time, is_finished, 
              mem_strategy, &turnaround, &max_overhead, &total_overhead);
    }

    // print out statistics
    if (turnaround % num != 0) {
        turnaround /= num;
        turnaround ++;
    } else {
        turnaround /= num;
    }
    printf("Turnaround time %d\nTime overhead %.2lf %.2lf\nMakespan %d\n", 
            turnaround, 
            round(max_overhead * 100) / 100, 
            round(total_overhead * 100 / num) / 100, current_time);

    // free all process and process array
    for (int i = 0; i < num; i++) {
        free(process[i]);
    }
    free(process);

}

// Run processes in Shortest Job First
void do_sjf(process_t **p, int n, int q, int *time, 
            int *is_finished, char *strategy,
            int *turnaround, double *max_overhead, double *total_overhead) {

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

        // initialize child and pipeline
        int pipe_to_child[2];
        int pipe_from_child[2];
        pipe(pipe_to_child);
        pipe(pipe_from_child);

        pid_t pid;
        uint32_t simulation_time_big_endian;

        // search arrived process
        for (j = 0; j < n; j++) {
            if (get_arrival_time(p[j]) <= *time && 
                is_finished[j] == 0) {

                // alloc mem when using best-fit
                if (use_strategy) {
                    if (!mem_allocated[j]) {
                        memstart = allocate_mem(memory, 
                                                get_process_mem(p[j]));
                        print_ready_msg(*time, get_process_name(p[j]), 
                                        memstart);
                        mem_allocated[j] = 1;
                    }
                }

                // create child process
                pid = fork();

                // child process
                if (pid == 0) {

                    // open pipeline
                    dup2(pipe_to_child[0], STDIN_FILENO);
                    dup2(pipe_from_child[1], STDOUT_FILENO);
                    close(pipe_to_child[1]);
                    close(pipe_from_child[0]);

                    // run prebuilt process exec
                    char *pargv[] = {"./process", 
                                     get_process_name(p[j]), 
                                     NULL};
                    execvp(pargv[0], pargv);

                } 
                
                // main process
                else {

                    close(pipe_to_child[0]);
                    close(pipe_from_child[1]);

                    // send current time to child
                    simulation_time_big_endian = htonl(*time);
                    write(pipe_to_child[1], &simulation_time_big_endian, 
                          sizeof(uint32_t));

                    // get and validate response
                    uint8_t response;
                    read(pipe_from_child[0], &response, sizeof(response));

                    // exit if mismatched
                    if (response != (*time & 0xFF)) exit(EXIT_FAILURE);

                }

                // print process running message
                process_running = 1;
                print_running_msg(*time, get_service_time(p[j]), 
                                  get_process_name(p[j]));

                // quantum controlling on real process
                int start_time = *time;
                *time = *time + q;

                // continue running until finished
                while (*time - start_time < get_service_time(p[j])) {

                    simulation_time_big_endian = htonl(*time);

                    // send current time for child to continue
                    write(pipe_to_child[1], &simulation_time_big_endian, 
                          sizeof(uint32_t));
                    kill(pid, SIGCONT);

                    // get and validate response
                    uint8_t response;
                    read(pipe_from_child[0], &response, sizeof(response));

                    // exit if mismatched
                    if (response != (*time & 0xFF)) exit(EXIT_FAILURE);

                    *time = *time + q;

                }

                // process finished after service (+q) time
                is_finished[j] = 1;
                break;

            }
        }

        // patch if process not run but time counts
        if (!process_running) {
            *time = *time + 1;
            i--;
            continue;
        }

        // avoid readings beyond the process array
        if (j == n) break;

        // using best-fit when capable
        if (use_strategy) {

            // store process ready and alloc in runtime
            int count = 0;
            int *original_index = malloc(n * sizeof(int));
            process_t **runtime = malloc(n * sizeof(process_t *));

            for (int k = 0; k < n; k++) {

                if (mem_allocated[k]) continue;

                // check if new process are ready
                if (get_arrival_time(p[k]) <= *time - q) {
                    runtime[count] = p[k];
                    original_index[count] = k;
                    count++;
                }

            }

            // re-sort by arrival time to align input sequence
            // print process ready message
            qsort(runtime, count, sizeof(*runtime), compare_arrival_time);
            for (int x = 0; x < count; x++) {
                print_ready_msg(get_arrival_time(runtime[x]), 
                                get_process_name(runtime[x]), 
                                allocate_mem(memory, 
                                             get_process_mem(runtime[x])));
                mem_allocated[original_index[x]] = 1;
            }

            // clear current finished process mem block
            clear_mem(memory, memstart, get_process_mem(p[j]));

            // free temp runtime list 
            free(runtime);
            free(original_index);

        }

        // print process result
        print_result_msg(n, q, *time, p, is_finished, 
                         get_process_name(p[j]));

        // terminate child process
        simulation_time_big_endian = htonl(*time);
        write(pipe_to_child[1], &simulation_time_big_endian, 
                                 sizeof(uint32_t));
        kill(pid, SIGTERM);

        // wait child to terminate
        int status;
        waitpid(pid, &status, 0);

        // read 64-byte string from child 
        char sha[65];
        read(pipe_from_child[0], sha, 64);
        sha[64] = '\0';

        // print sha
        printf("%d,FINISHED-PROCESS,process_name=%s,sha=%s\n", 
            *time, get_process_name(p[j]), 
            sha);

        // close pipeline
        close(pipe_to_child[1]);
        close(pipe_from_child[0]);
        
        // calc stats when one process finish
        *turnaround = *turnaround + (*time - get_arrival_time(p[j]));
        if ((double)(*time - get_arrival_time(p[j])) / 
                      get_service_time(p[j]) > *max_overhead) {
            *max_overhead = (double)(*time - get_arrival_time(p[j])) / 
                                     (get_service_time(p[j]));
        }
        *total_overhead = *total_overhead + (double)
                                            (*time - get_arrival_time(p[j])) /
                                                     get_service_time(p[j]);
        
    }

    // free memory block's mem
    if (use_strategy) {
        free_mem(memory);
    }

}


// Run processes in Round Robin
void do_rr(process_t **p, int n, int q, int *time, 
           int *is_finished, char *strategy, 
           int *turnaround, double *max_overhead, double *total_overhead) {

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

    // initialize child and pipeline
    child_t **child = malloc(n * sizeof(child_t *));
    for (int i = 0; i < n; i++) {
        child[i] = malloc(sizeof(child_t));
        pipe(child[i]->pipe_from_child);
        pipe(child[i]->pipe_to_child);
        child[i]->in_use = 0;
    }

    // run until all finished
    char *last_process = NULL;
    int last_process_index = 0;
    while (all_finished == 0) {

        int memstart[n];

        // run each process on given quantum
        for (int i = 0; i < n; i++) {
            if (get_arrival_time(p[i]) <= *time && 
                is_finished[i] == 0) {

                // when using best-fit
                if (use_strategy) {
                    for (int j = 0; j < n; j++) {
                        if (!mem_allocated[j] && 
                             get_arrival_time(p[j]) <= 
                            *time && is_finished[i] == 0) {
                            memstart[j] = allocate_mem(memory, 
                                          get_process_mem(p[j]));

                            // alloc process mem
                            if (memstart[j] != -1) {
                                print_ready_msg(*time, 
                                get_process_name(p[j]), 
                                                 memstart[j]);
                                mem_allocated[j] = 1;
                            }
                        }
                    }
                } else {

                    // assume all mem alloc when in infinity
                    for (int j = 0; j < n; j ++) {
                        mem_allocated[j] = 1;
                    }

                }

                // start run process when its mem has alloc
                if (mem_allocated[i] && (last_process == NULL || 
                    strcmp(last_process, get_process_name(p[i])) != 0)) {

                        // suspend process when other ready runs
                        if (last_process != NULL && 
                            child[last_process_index]->in_use) {

                            // get current time
                            child[last_process_index]->
                            simulation_time_big_endian = htonl(*time);

                            // send time to process
                            write(child[last_process_index]->pipe_to_child[1], 
                                 &child[last_process_index]->
                                  simulation_time_big_endian, 
                                  sizeof(uint32_t));

                            // send process to suspend
                            kill(child[last_process_index]->pid, SIGTSTP);

                            // wait till process is been suspended
                            int wstatus = 0;
                            waitpid(child[last_process_index]->pid, &wstatus, 
                                    WUNTRACED);
                            while (!WIFSTOPPED(wstatus)) {
                                waitpid(child[last_process_index]->pid, 
                                       &wstatus, WUNTRACED);
                            }
                        }

                    // process run first-time
                    if (child[i]->in_use == 0) {

                        // create child process
                        child[i]->in_use = 1;
                        child[i]->pid = fork();

                        // child process
                        if (child[i]->pid == 0) {

                            // open pipeline
                            dup2(child[i]->pipe_to_child[0], STDIN_FILENO);
                            dup2(child[i]->pipe_from_child[1], STDOUT_FILENO);
                            close(child[i]->pipe_to_child[1]);
                            close(child[i]->pipe_from_child[0]);

                            // run prebuilt process exec
                            char *pargv[] = {"./process", 
                                              get_process_name(p[i]), 
                                              NULL};
                            execvp(pargv[0], pargv);

                        } 
                        
                        // main process
                        else {

                            close(child[i]->pipe_to_child[0]);
                            close(child[i]->pipe_from_child[1]);

                            // send current time to child
                            child[i]->simulation_time_big_endian = 
                                      htonl(*time);
                            write(child[i]->pipe_to_child[1], &child[i]->
                                  simulation_time_big_endian, 
                                  sizeof(uint32_t));

                            // get and validate response
                            uint8_t response;
                            read(child[i]->pipe_from_child[0], &response, 
                                 sizeof(response));
                            if (response != (*time & 0xFF)) {
                                exit(EXIT_FAILURE);
                            }
                        }

                        // print running message
                        print_running_msg(*time, remain_time[i],
                                           get_process_name(p[i]));
                                      
                    } 
                    
                    // continue run process
                    else {
                        send_cont_signal(child, time, i);
                    }

                    // store previous process key info
                    last_process = get_process_name(p[i]);
                    last_process_index = i;

                } 
                
                // same process run in another rr
                else if (mem_allocated[i] && 
                         strcmp(last_process, 
                         get_process_name(p[i])) == 0) {

                    send_cont_signal(child, time, i);
                } 
                
                // reject process when it is not arrive
                else if (!mem_allocated[i]) {
                    *time = *time - q;
                    remain_time[i] += q;
                }
                
                // update current time
                // update remain time for current process
                *time = *time + q;
                remain_time[i] -= q;

                // process finish
                if (remain_time[i] <= 0) {
                    is_finished[i] = 1;       

                    // print result
                    print_result_msg(n, q, *time, p, is_finished, 
                                     get_process_name(p[i]));

                    // terminate child process
                    child[i]->simulation_time_big_endian = htonl(*time);
                    write(child[i]->pipe_to_child[1], 
                         &child[i]->simulation_time_big_endian, 
                          sizeof(uint32_t));
                    kill(child[i]->pid, SIGTERM);

                    // wait child to terminate
                    int status;
                    waitpid(child[i]->pid, &status, 0);

                    // read 64-byte string from child 
                    char sha[65];
                    read(child[i]->pipe_from_child[0], sha, 64);
                    sha[64] = '\0';

                    // print sha
                    printf("%d,FINISHED-PROCESS,process_name=%s,sha=%s\n", 
                            *time, get_process_name(p[i]), 
                            sha);

                    // close pipelines
                    close(child[i]->pipe_to_child[1]);
                    close(child[i]->pipe_from_child[0]);
                    child[i]->in_use = 0;

                    // calc stats when one process finish
                    *turnaround = *turnaround + 
                                 (*time - get_arrival_time(p[i]));
                    if ((double)(*time - get_arrival_time(p[i])) / 
                                (get_service_time(p[i])) > *max_overhead) {
                        *max_overhead = (double)
                                        (*time - get_arrival_time(p[i])) / 
                                                 get_service_time(p[i]);
                    }
                    *total_overhead = *total_overhead + 
                                      (double)
                                      (*time - get_arrival_time(p[i])) /
                                               get_service_time(p[i]);
                    
                    // clear process memory block
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

    // free child memory
    for (int i = 0; i < n; i++) {
        free(child[i]);
    }
    free(child);

    // free memory block's mem
    if (use_strategy) {
        free_mem(memory);
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


// Print process is running message
void print_running_msg(int time, int remain_time, char *name) {
    printf("%d,RUNNING,process_name=%s,remaining_time=%d\n", 
            time, name, remain_time);
}


// Print result given process running
void print_result_msg(int n, int q, int time, 
                      process_t **p, int *is_finished, char *name) {
    printf("%d,FINISHED,process_name=%s,proc_remaining=%d\n", 
            time, name, 
            check_proc_remaining(n, q, time, p, is_finished));
}


// Print a process is ready message
void print_ready_msg(int time, char *name, int memstart) {
    printf("%d,READY,process_name=%s,assigned_at=%d\n", 
            time, name, memstart);
}


// Check remaining processes in ready
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


// Send a continue signal to process
void send_cont_signal(child_t **child, int *time, int i) {

    child[i]->simulation_time_big_endian = htonl(*time);

    // send time for child to continue
    write(child[i]->pipe_to_child[1], &child[i]->simulation_time_big_endian, 
          sizeof(uint32_t));
    kill(child[i]->pid, SIGCONT);

    // get and validate response
    uint8_t response;
    read(child[i]->pipe_from_child[0], &response, sizeof(response));

    // exit if mismatched
    if (response != (*time & 0xFF)) exit(EXIT_FAILURE);
}