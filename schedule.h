#ifndef _SCHEDULE_H_
#define _SCHEDULE_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "data.h"
#include "memory.h"

void start_scheduling(process_t **lines, char *scheduler, 
                      int num, int quantum, char *mem_strategy);

void do_sjf(process_t **p, int num, int q, int time, 
            int *is_finished, char *mem_strategy);

void do_rr(process_t **p, int num, int q, int time, 
           int *is_finished);

int compare_arrival_time(const void *a, const void *b);

int compare_service_time(const void *a, const void *b);

void print_running_msg(int time, int remain_time, char *name);

void print_result_msg(int n, int q, int time, 
                      process_t **p, int *is_finished, char *name);

void print_ready_msg(int time, char *name, int memstart);

int check_proc_remaining(int n, int q, int time, 
                         process_t **p, int *is_finished);

#endif