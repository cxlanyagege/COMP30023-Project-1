#include "data.h"

struct process
{
    int arrival;
    char name[MAX_PROCESS_NAME];
    int service;
    int memory;
};


process_t **read_process(char *filename, int *num) {

    // open process list file
    FILE *file = fopen(filename, "r");
    assert(file);
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

    // store process
    int i = 0;
    while (fgets(line, sizeof(line), file) != NULL) {
        // line[strcspn(line, "\n")] = 0;
        processes[i] = malloc(sizeof(process_t *));
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

int get_arrival_time(process_t *process) {
    return process->arrival;
}

char *get_process_name(process_t *process) {
    return process->name;
}

int get_service_time(process_t *process) {
    return process->service;
}

int get_process_mem(process_t *process) {
    return process->memory;
}