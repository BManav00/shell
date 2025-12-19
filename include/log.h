#ifndef LOG_H
#define LOG_H

#define LOG_MAX 15
#define LOG_FILE ".log_history"

void log_init();
void log_load();
void log_save();
void log_add(const char *cmd);
void log_print();
void log_purge();
void log_execute(int index, char *out_cmd);
int log_is_duplicate(const char *cmd);
int log_is_log_cmd(const char *cmd);

#endif // LOG_H

//llm code