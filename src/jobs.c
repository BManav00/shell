#include "../include/jobs.h"
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>

static job_t jobs[MAX_JOBS];
static int job_count = 0;

void jobs_init() { job_count = 0; }

void jobs_add(pid_t pid, const char *cmd) {
    if (job_count < MAX_JOBS) {
        jobs[job_count].pid = pid;
        strncpy(jobs[job_count].cmd, cmd, 255);
        jobs[job_count].state = RUNNING;
        job_count++;
    }
}

void jobs_update() {
    for (int i = 0; i < job_count; i++) {
        if (jobs[i].state == RUNNING) {
            int status;
            pid_t result = waitpid(jobs[i].pid, &status, WNOHANG | WUNTRACED);
            if (result == jobs[i].pid) {
                if (WIFSTOPPED(status)) {
                    jobs[i].state = STOPPED;
                    printf("[%d] Stopped %s\n", jobs[i].pid, jobs[i].cmd);
                } else {
                    jobs[i].state = DONE;
                    printf("%s with pid %d exited normally\n", jobs[i].cmd, jobs[i].pid);
                }
            }
        }
    }
}

void jobs_print() {
    for (int i = 0; i < job_count; i++) {
        if (jobs[i].state != DONE) {
            printf("[%d] : %s - %s\n", jobs[i].pid, jobs[i].cmd,
                jobs[i].state == RUNNING ? "Running" : "Stopped");
        }
    }
}

void jobs_remove(pid_t pid) {
    for (int i = 0; i < job_count; i++) {
        if (jobs[i].pid == pid) {
            for (int j = i; j < job_count-1; j++) jobs[j] = jobs[j+1];
            job_count--;
            break;
        }
    }
}

void jobs_set_state(pid_t pid, job_state state) {
    for (int i = 0; i < job_count; i++) {
        if (jobs[i].pid == pid) {
            jobs[i].state = state;
            break;
        }
    }
}

job_t* jobs_find(pid_t pid) {
    for (int i = 0; i < job_count; i++) {
        if (jobs[i].pid == pid) return &jobs[i];
    }
    return NULL;
}

job_t* jobs_find_by_index(int idx) {
    int count = 0;
    for (int i = 0; i < job_count; i++) {
        if (jobs[i].state != DONE) {
            count++;
            if (count == idx) return &jobs[i];
        }
    }
    return NULL;
}

int jobs_count() {
    int count = 0;
    for (int i = 0; i < job_count; i++) {
        if (jobs[i].state != DONE) count++;
    }
    return count;
}

void jobs_fg(int idx) {
    job_t *job = jobs_find_by_index(idx);
    if (!job) { printf("No such job\n"); return; }
    if (job->state == STOPPED) kill(job->pid, SIGCONT);
    printf("Bringing job [%d] %s to foreground\n", job->pid, job->cmd);
    int status;
    waitpid(job->pid, &status, WUNTRACED);
    if (WIFSTOPPED(status)) job->state = STOPPED;
    else job->state = DONE;
}

void jobs_bg(int idx) {
    job_t *job = jobs_find_by_index(idx);
    if (!job) { printf("No such job\n"); return; }
    if (job->state == RUNNING) { printf("Job already running\n"); return; }
    kill(job->pid, SIGCONT);
    job->state = RUNNING;
    printf("[%d] %s &\n", job->pid, job->cmd);
}

void jobs_ping(pid_t pid, int sig) {
    job_t *job = jobs_find(pid);
    if (!job) { printf("No such process found\n"); return; }
    int actual_signal = sig % 32;
    if (kill(pid, actual_signal) == 0)
        printf("Sent signal %d to process with pid %d\n", sig, pid);
    else
        printf("No such process found\n");
}

//llm code