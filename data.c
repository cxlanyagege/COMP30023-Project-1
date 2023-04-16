#include "data.h"

struct process
{
    int arrival;
    char name[MAX_PROCESS_NAME];
    int service;
    int memory;
};


// Read all process info from file
process_t **read_process(char *filename, int *num) {

    // open process list file
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        exit(1);
    }

    // count process lines
    char line[MAX_CHAR_LINE];
    int line_total = 0;
    while (fgets(line, sizeof(line), file) != NULL) {
        line_total++;
    }
    *num = line_total;

    // prepare storing process info
    process_t **processes = malloc(line_total * 
                                   sizeof(process_t *));
    fseek(file, 0, SEEK_SET);

    // store process into struct
    int i = 0;
    while (fgets(line, sizeof(line), file) != NULL) {
        processes[i] = malloc(sizeof(process_t));
        sscanf(line, "%d %s %d %d", &processes[i]->arrival,
                                    processes[i]->name,
                                    &processes[i]->service,
                                    &processes[i]->memory);
        i++;
    }

    // close file
    fclose(file);

    // return list of process
    return processes;

}


// Get process's arrival time
int get_arrival_time(process_t *process) {
    return process->arrival;
}


// Get process's name
char *get_process_name(process_t *process) {
    return process->name;
}


// Get process's servicing time
int get_service_time(process_t *process) {
    return process->service;
}


// Get process's memory requirement
int get_process_mem(process_t *process) {
    return process->memory;
}