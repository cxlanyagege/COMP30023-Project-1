#ifndef _SCHEDULE_H_
#define _SCHEDULE_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "data.h"

void start_scheduling(process_t **lines, char *scheduler, 
                      int num, int quantum);

void do_sjf(process_t **p, int num, int q);

void do_rr(process_t **p, int num, int q);

int compare_time(const void *a, const void *b);

#endif