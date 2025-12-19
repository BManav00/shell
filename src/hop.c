#include "../include/hop.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>

void hop_command(int argc, char **argv, char *homedir, char *prevdir, char *currentdir) {
    int hop_used = 0;
    for (int i = 0; i < argc; i++) {
        if (strcmp(argv[i], "~") == 0 || argc == 0) {
            strcpy(prevdir, currentdir);
            if (chdir(homedir) != 0) printf("No such directory!\n");
            hop_used = 1;
        } else if (strcmp(argv[i], ".") == 0) {
            // Do nothing
        } else if (strcmp(argv[i], "..") == 0) {
            strcpy(prevdir, currentdir);
            if (chdir("..") != 0) printf("No such directory!\n");
            hop_used = 1;
        } else if (strcmp(argv[i], "-") == 0) {
            if (strlen(prevdir) == 0) {
                printf("No such directory!\n");
            } else {
                char temp[1024];
                strcpy(temp, currentdir);
                if (chdir(prevdir) != 0) printf("No such directory!\n");
                strcpy(prevdir, temp);
                hop_used = 1;
            }
        } else {
            char temp[1024];
            strcpy(temp, currentdir);
            if (chdir(argv[i]) != 0) printf("No such directory!\n");
            else {
                strcpy(prevdir, temp);
                hop_used = 1;
            }
        }
    }
    if (!hop_used && argc == 0) {
        if (chdir(homedir) != 0) printf("No such directory!\n");
    }
}

//llm code