#include "../include/log.h"
#include <stdio.h>
#include <string.h>

#define LOG_MAX 15
#define LOG_FILE ".log_history"

static char log_history[LOG_MAX][1024];
static int log_count = 0;
static int log_start = 0;

void log_init() {
    log_count = 0;
    log_start = 0;
    log_load();
}

void log_load() {
    FILE *f = fopen(LOG_FILE, "r");
    if (!f) return;
    log_count = 0;
    log_start = 0;
    char buf[1024];
    while (fgets(buf, sizeof(buf), f)) {
        buf[strcspn(buf, "\n")] = 0;
        if (strlen(buf) > 0) {
            strncpy(log_history[log_count++], buf, 1023);
            if (log_count == LOG_MAX) break;
        }
    }
    fclose(f);
}

void log_save() {
    FILE *f = fopen(LOG_FILE, "w");
    if (!f) return;
    for (int i = 0; i < log_count; i++) {
        int idx = (log_start + i) % LOG_MAX;
        fprintf(f, "%s\n", log_history[idx]);
    }
    fclose(f);
}

int log_is_duplicate(const char *cmd) {
    if (log_count == 0) return 0;
    int last_idx = (log_start + log_count - 1) % LOG_MAX;
    return strcmp(log_history[last_idx], cmd) == 0;
}

int log_is_log_cmd(const char *cmd) {
    // Check if first word is "log"
    char first[16];
    sscanf(cmd, "%15s", first);
    return strcmp(first, "log") == 0;
}

void log_add(const char *cmd) {
    if (log_is_duplicate(cmd) || log_is_log_cmd(cmd)) return;
    if (log_count < LOG_MAX) {
        strncpy(log_history[log_count++], cmd, 1023);
    } else {
        strncpy(log_history[log_start], cmd, 1023);
        log_start = (log_start + 1) % LOG_MAX;
    }
    log_save();
}

void log_print() {
    for (int i = 0; i < log_count; i++) {
        int idx = (log_start + i) % LOG_MAX;
        printf("%s\n", log_history[idx]);
    }
}

void log_purge() {
    log_count = 0;
    log_start = 0;
    log_save();
}

void log_execute(int index, char *out_cmd) {
    // index: 1 = newest, LOG_MAX = oldest
    if (index < 1 || index > log_count) {
        out_cmd[0] = '\0';
        return;
    }
    int idx = (log_start + log_count - index) % LOG_MAX;
    strncpy(out_cmd, log_history[idx], 1023);
}

//llm code