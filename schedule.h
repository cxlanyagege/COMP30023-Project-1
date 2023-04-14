#ifndef _SCHEDULE_H_
#define _SCHEDULE_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "data.h"

void start_scheduling(process_t **lines, char *scheduler, int num);

char **do_sjf(char **lines, int num);

char **do_rr(char **lines, int num);

#endif