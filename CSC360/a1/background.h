
#ifndef BACKGROUND_H
#define BACKGROUND_H

#include <sys/types.h>

#define MAX_INPUT 1024

// Data structure for a background job
typedef struct bg_job {
    pid_t pid;
    char command[MAX_INPUT];
    struct bg_job *next;
} bg_job_t;

// Function declarations
void add_bg_job(pid_t pid, const char *cmd);
void remove_bg_job(pid_t pid);
void execute_background(char **args, char *original_input);
void print_bg_jobs(void);
void check_bg_jobs(void);
void notify_job_terminated(pid_t pid, int status);


#endif // BACKGROUND_H
