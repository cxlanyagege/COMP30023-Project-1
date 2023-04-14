#include "schedule.h"

char **start_scheduling(char **lines, char *scheduler, int num) {

    // determine scheduler
    if (strcmp(scheduler, "SJF") == 0) {
        return do_sjf(lines, num);
    } else if (strcmp(scheduler, "RR") == 0) {
        return do_rr(lines, num);
    }

}

char **do_sjf(char **lines, int num) {

}

char **do_rr(char **lines, int num) {

}