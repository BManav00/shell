#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <ctype.h>
#include "../include/reveal.h"

// Case-sensitive compare for qsort
static int ci_cmp(const void *a, const void *b) {
    const char *sa = *(const char **)a;
    const char *sb = *(const char **)b;
    return strcmp(sa, sb);
}

void reveal_command(int flagl, int flaga, const char *path, int error) {
    if (error) {
        printf("No such directory!\n");
        return;
    }
    struct dirent **namelist;
    int n = scandir(path, &namelist, NULL, alphasort);
    if (n < 0) {
        printf("No such directory!\n");
        return;
    }
    // Collect entries
    char *entries[n+2];
    int count = 0;
    for (int i = 0; i < n; i++) {
        if (!flaga && namelist[i]->d_name[0] == '.') {
            free(namelist[i]);
            continue;
        }
        entries[count++] = strdup(namelist[i]->d_name);
        free(namelist[i]);
    }
    free(namelist);
    // Include . and .. only if -a flag is set
    if (flaga) {
        int already_dot = 0, already_dotdot = 0;
        for (int i = 0; i < count; i++) {
            if (strcmp(entries[i], ".") == 0) already_dot = 1;
            if (strcmp(entries[i], "..") == 0) already_dotdot = 1;
        }
        if (!already_dot) entries[count++] = strdup(".");
        if (!already_dotdot) entries[count++] = strdup("..");
    }
    // Sort case-insensitively
    qsort(entries, count, sizeof(char *), ci_cmp);
    // Print
    for (int i = 0; i < count; i++) {
        if (flagl) {
            printf("%s\n", entries[i]);
        } else {
            printf("%s%s", entries[i], (i == count-1) ? "" : "..");
        }
        free(entries[i]);
    }
    if (!flagl) printf("\n");
}

//llm code