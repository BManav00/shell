#ifndef JOBS_H
#define JOBS_H

#include <sys/types.h>
#define MAX_JOBS 32

typedef enum { RUNNING, STOPPED, DONE } job_state;

typedef struct {
    pid_t pid;
    char cmd[256];
    job_state state;
} job_t;

void jobs_init();
void jobs_add(pid_t pid, const char *cmd);
void jobs_update();
void jobs_print();
void jobs_remove(pid_t pid);
void jobs_set_state(pid_t pid, job_state state);
job_t* jobs_find(pid_t pid);
job_t* jobs_find_by_index(int idx); // 1-based index for fg/bg
int jobs_count();
void jobs_fg(int idx);
void jobs_bg(int idx);
void jobs_ping(pid_t pid, int sig);

#endif // JOBS_H

//llm code