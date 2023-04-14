#ifndef _DATA_H_
#define _DATA_H_

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#define MAX_PROCESS_NAME 8
#define MAX_CHAR_LINE 16

typedef struct process process_t;


process_t **read_process(char *filename, int *num);

#endif