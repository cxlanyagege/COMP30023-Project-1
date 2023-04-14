#include "schedule.h"

void start_scheduling(process_t **lines, char *scheduler, int num) {

    // determine scheduler
    if (strcmp(scheduler, "SJF") == 0) {
        do_sjf(lines, num);
    } else if (strcmp(scheduler, "RR") == 0) {
        do_rr(lines, num);
    }

}

char **do_sjf(char **lines, int num) {

}

char **do_rr(char **lines, int num) {

}