#ifndef _DATA_H_
#define _DATA_H_

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#define MAX_PROCESS_NAME 8
#define MAX_CHAR_LINE 32

typedef struct process process_t;

process_t **read_process(char *filename, int *num);

int get_arrival_time(process_t *process);

char *get_process_name(process_t *process);

int get_service_time(process_t *process);

#endif