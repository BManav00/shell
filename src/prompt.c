#define _GNU_SOURCE
#include "../include/prompt.h"
#include <stdio.h>
#include <string.h>

void show_prompt(const char *username, const char *systemname, const char *homedir, const char *currentdir) {
    if (strncmp(currentdir, homedir, strlen(homedir)) == 0) {
        // Show tilde for home directory
        printf("<%s@%s:~%s> ", username, systemname, currentdir + strlen(homedir));
    } else {
        printf("<%s@%s:%s> ", username, systemname, currentdir);
    }
}

//llm code