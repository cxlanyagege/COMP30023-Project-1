#ifndef _SCHEDULE_H_
#define _SCHEDULE_H_

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <arpa/inet.h>

#include "data.h"
#include "memory.h"

#define IMPLEMENTS_REAL_PROCESS

typedef struct child child_t;

void start_scheduling(process_t **lines, char *scheduler, 
                      int num, int quantum, char *mem_strategy);

void do_sjf(process_t **p, int num, int q, int *time, 
            int *is_finished, char *mem_strategy, int *turnaround, 
            double *max_overhead, double *total_overhead);

void do_rr(process_t **p, int num, int q, int *time, 
           int *is_finished, char *mem_strategy, int *turnaround, 
           double *max_overhead, double *total_overhead);

int compare_arrival_time(const void *a, const void *b);

int compare_service_time(const void *a, const void *b);

void print_running_msg(int time, int remain_time, char *name);

void print_result_msg(int n, int q, int time, 
                      process_t **p, int *is_finished, char *name);

void print_ready_msg(int time, char *name, int memstart);

int check_proc_remaining(int n, int q, int time, 
                         process_t **p, int *is_finished);

void send_cont_signal(child_t **child, int *time, int i);

#endif