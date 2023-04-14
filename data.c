#include "data.h"

char **read_process(char *filename, int *num) {

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
    num = line_total;

    // prepare storing process info
    char **lines = (char **)malloc(line_total * sizeof(char *));
    fseek(file, 0, SEEK_SET);

    // store process
    int i = 0;
    while (fgets(line, sizeof(line), file) != NULL) {
        // line[strcspn(line, "\n")] = 0;
        lines[i] = (char *)malloc(strlen(line) + 1);
        strcpy(lines[i], line);
        i++;
    }

    // close file
    fclose(file);

    // return list of process
    return lines;

}