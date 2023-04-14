#ifndef _SCHEDULE_H_
#define _SCHEDULE_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_PROCESS_NAME 8

char **start_scheduling(char **lines, char *scheduler, int num);

char **do_sjf(char **lines, int num);

char **do_rr(char **lines, int num);

#endif