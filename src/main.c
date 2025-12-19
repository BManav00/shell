#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#include "../include/prompt.h"
#include "../include/parser.h"
#include "../include/hop.h"
#include "../include/reveal.h"
#include "../include/log.h"
#include "../include/execute.h"
#include "../include/jobs.h"
#include <pwd.h> // Make sure to add this include at the top

volatile sig_atomic_t fg_pid = -1;

void sigint_handler(int sig) {
    if (fg_pid > 0) kill(-fg_pid, SIGINT);
}
void sigtstp_handler(int sig) {
    if (fg_pid > 0) {
        kill(-fg_pid, SIGTSTP);
        printf("\nStopped\n");
        // Add the suspended job to job tracking
        jobs_add(fg_pid, "foreground job");
        jobs_set_state(fg_pid, STOPPED);
        fg_pid = -1;
    }
}

int main() {
    char *username = getenv("USER");
if (!username) {
    struct passwd *pw = getpwuid(getuid());
    if (pw) username = pw->pw_name;
}

if (!username) {
    printf("NO USERNAME\n");
    return 1;
}
    char systemname[256];
    if (gethostname(systemname, sizeof(systemname)) != 0) { printf("NO HOSTNAME\n"); return 1; }
    char homedir[1024];
    if (!getcwd(homedir, sizeof(homedir))) { printf("NO DIRECTORY\n"); return 1; }
    char prevdir[1024] = "";
    log_init();
    jobs_init();
    signal(SIGINT, sigint_handler);
    signal(SIGTSTP, sigtstp_handler);
    while (1) {
        char currentdir[1024];
        if (!getcwd(currentdir, sizeof(currentdir))) { printf("NO DIRECTORY\n"); return 1; }
        show_prompt(username, systemname, homedir, currentdir);
        char input[1026];
        if (!fgets(input, sizeof(input), stdin)) {
            // jobs_killall(); // If you have this function, call it here
            printf("logout\n");
            exit(0);
        }
        input[strcspn(input, "\n")] = 0;
        if (strlen(input) == 0) continue;
        
        // Check for completed background jobs before processing new command
        jobs_update();
        
        tokenize(input);
        int tokencnt = 0; while (tokens[tokencnt].type != T_END) tokencnt++;
        if (tokens[0].type == T_END) continue;
        if (!parseok()) { printf("Invalid Syntax!\n"); continue; }
        
        // Check if command has pipes, redirections, or sequential/background operators
        int has_special_ops = 0;
        for (int i = 0; i < tokencnt; i++) {
            if (tokens[i].type == T_PIPE || tokens[i].type == T_INPUT || 
                tokens[i].type == T_OUTPUT || tokens[i].type == T_APPEND ||
                tokens[i].type == T_SEMI || tokens[i].type == T_AMP) {
                has_special_ops = 1;
                break;
            }
        }
        
        // If command has special operations, route to execute_command
        if (has_special_ops) {
            execute_command(tokens, tokencnt);
            log_add(input);
            continue;
        }
        
        // Intrinsics
        if (strcmp(tokens[0].text, "hop") == 0) {
            char *args[32]; int argc = 0;
            for (int i = 1; tokens[i].type != T_END; i++) args[argc++] = tokens[i].text;
            hop_command(argc, args, homedir, prevdir, currentdir);
            log_add(input);
        } else if (strcmp(tokens[0].text, "reveal") == 0) {
            int flagl = 0, flaga = 0, error = 0;
            char *targetdir = currentdir;
            int argcount = 0;
            for (int i = 1; tokens[i].type != T_END; i++) {
                if (tokens[i].text[0] == '-' && strlen(tokens[i].text) > 1 && tokens[i].text[1] != '\0') {
                    // This is a flag like -l, -a, -la
                    for (int j = 1; tokens[i].text[j]; j++) {
                        if (tokens[i].text[j] == 'l') flagl = 1;
                        if (tokens[i].text[j] == 'a') flaga = 1;
                    }
                } else {
                    argcount++;
                    if (strcmp(tokens[i].text, "~") == 0) targetdir = homedir;
                    else if (strcmp(tokens[i].text, "-") == 0) {
                        if (strlen(prevdir) == 0) error = 1;
                        else targetdir = prevdir;
                    } else if (strcmp(tokens[i].text, "..") == 0) targetdir = "..";
                    else if (strcmp(tokens[i].text, ".") == 0) targetdir = currentdir;
                    else targetdir = tokens[i].text;
                }
            }
            if (argcount > 1) {
                printf("reveal: Invalid Syntax!\n");
            } else {
                reveal_command(flagl, flaga, targetdir, error);
                log_add(input);
            }
        } else if (strcmp(tokens[0].text, "log") == 0) {
            if (tokens[1].type == T_END) {
                log_print();
            } else if (strcmp(tokens[1].text, "purge") == 0) {
                log_purge();
            } else if (strcmp(tokens[1].text, "execute") == 0 && tokens[2].type == T_NAME) {
                int idx = atoi(tokens[2].text);
                char cmd[1024] = "";
                log_execute(idx, cmd);
                if (strlen(cmd) > 0) {
                    tokenize(cmd);
                    int log_tokencnt = 0; while (tokens[log_tokencnt].type != T_END) log_tokencnt++;
                    if (!parseok()) { printf("Invalid Syntax!\n"); continue; }
                    execute_command(tokens, log_tokencnt);
                }
            }
        } else if (strcmp(tokens[0].text, "activities") == 0) {
            jobs_print();
        } else if (strcmp(tokens[0].text, "ping") == 0 && tokens[1].type == T_NAME && tokens[2].type == T_NAME) {
            pid_t pid = (pid_t)atoi(tokens[1].text);
            int sig = atoi(tokens[2].text);
            jobs_ping(pid, sig);
        } else if (strcmp(tokens[0].text, "fg") == 0) {
            int jobnum = -1;
            if (tokens[1].type == T_NAME) jobnum = atoi(tokens[1].text);
            job_t *job = jobs_find_by_index(jobnum);
            if (job) {
                // Send SIGCONT if stopped, then wait for job
                kill(-job->pid, SIGCONT);
                int status;
                waitpid(job->pid, &status, WUNTRACED);
                jobs_remove(job->pid);
            } else {
                printf("No such job\n");
            }
        } else if (strcmp(tokens[0].text, "bg") == 0) {
            int jobnum = -1;
            if (tokens[1].type == T_NAME) jobnum = atoi(tokens[1].text);
            job_t *job = jobs_find_by_index(jobnum);
            if (job) {
                // Only resume if stopped
                if (job->state == STOPPED) {
                    kill(-job->pid, SIGCONT);
                    printf("[%d] %s &\n", jobnum, job->cmd);
                } else {
                    printf("Job already running\n");
                }
            } else {
                printf("No such job\n");
            }
        } else if (strcmp(input, "STOP") == 0) {
            break;
        } else {
            // External commands, pipes, redirection, background/sequential
            // Split input at ';' and '&' and execute each group
            int start = 0;
            for (int i = 0; ; i++) {
                if (tokens[i].type == T_SEMI || tokens[i].type == T_AMP || tokens[i].type == T_END) {
                    int end = i;
                    int bg = (tokens[i].type == T_AMP);
                    // Prepare sub-tokens for this group
                    token subcmd[128];
                    int subcnt = 0;
                    for (int j = start; j < end; j++) {
                        subcmd[subcnt++] = tokens[j];
                    }
                    subcmd[subcnt].type = T_END;
                    strcpy(subcmd[subcnt].text, "");
                    if (subcnt > 0) {
                        if (bg) {
                            pid_t child_pid = fork();
                            if (child_pid == 0) {
                                setsid();
                                execute_command(subcmd, subcnt);
                                exit(0);
                            } else if (child_pid > 0) {
                                // Build command string from tokens
                                char cmd_str[1024] = "";
                                for (int k = 0; k < subcnt; k++) {
                                    if (k > 0) strcat(cmd_str, " ");
                                    strcat(cmd_str, subcmd[k].text);
                                }
                                // Add background job to tracking
                                jobs_add(child_pid, cmd_str);
                                // Print job start message after adding job
                                printf("[%d] %d\n", jobs_count(), child_pid);
                            }
                            // Do not wait for background jobs
                        } else {
                            pid_t child_pid = fork();
                            if (child_pid == 0) {
                                execute_command(subcmd, subcnt);
                                exit(0);
                            } else if (child_pid > 0) {
                                fg_pid = child_pid;
                                int status;
                                waitpid(child_pid, &status, WUNTRACED);
                                fg_pid = -1;
                            }
                        }
                        log_add(input);
                    }
                    if (tokens[i].type == T_END) break;
                    start = i+1;
                }
            }
        }
    }
    return 0;
}

//llm code