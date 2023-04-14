#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "process.h"
#include "schedule.h"

int main(int argc, char *argv[]) {

    // receive arguments from cmdline
    char *filename = NULL;
    char *scheduler = NULL;
    char *mem_strategy = NULL;
    int quantum = 0;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-f") == 0) {
            i++;
            filename = argv[i];
        } else if (strcmp(argv[i], "-s") == 0) {
            i++;
            scheduler = argv[i];
        } else if (strcmp(argv[i], "-m") == 0) {
            i++;
            mem_strategy = argv[i];
        } else if (strcmp(argv[i], "-q") == 0) {
            i++;
            quantum = atoi(argv[i]);
        }
    }

    // read process list
    char **process_lines = read_process(filename);
    
    // start process scheduling

}