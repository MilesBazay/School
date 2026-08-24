#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include "background.h"

bg_job_t *bg_head = NULL;

/* Add a background job to the linked list */
void add_bg_job(pid_t pid, const char *cmd) {
    bg_job_t *new_job = malloc(sizeof(bg_job_t));
    if (!new_job) return;

    new_job->pid = pid;
    strncpy(new_job->command, cmd, MAX_INPUT - 1);
    new_job->command[MAX_INPUT - 1] = '\0';
    new_job->next = bg_head;
    bg_head = new_job;
}

/* Remove a job from the list once it terminates */
void remove_bg_job(pid_t pid) {
    bg_job_t **curr = &bg_head;
    while (*curr) {
        if ((*curr)->pid == pid) {
            bg_job_t *tmp = *curr;
            *curr = (*curr)->next;
            free(tmp);
            return;
        }
        curr = &(*curr)->next;
    }
}

/* Check if any background jobs have finished */
void check_bg_jobs(void) {
    int status;
    pid_t pid;

    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        bg_job_t *curr = bg_head;
        while (curr) {
            if (curr->pid == pid) {
                printf("%d: %s has terminated.\n", pid, curr->command);
                remove_bg_job(pid);
                break;
            }
            curr = curr->next;
        }
    }
}

/* List all current background jobs */
void print_bg_jobs(void) {
    int count = 0;
    for (bg_job_t *curr = bg_head; curr; curr = curr->next) {
        printf("%d: %s\n", curr->pid, curr->command);
        count++;
    }
    printf("Total Background jobs: %d\n", count);
}

/* Notify system of terminated job */
void notify_job_terminated(pid_t pid, int status) {
    bg_job_t *curr = bg_head;
    while (curr) {
        if (curr->pid == pid) {
            printf("%d: %s has terminated.\n", pid, curr->command);
            fflush(stdout);
            remove_bg_job(pid);
            return;
        }
        curr = curr->next;
    }
}

/* Execute a background command */
void execute_background(char **args, char *original_input) {
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork failed");
        return;
    }

    if (pid == 0) {
        // Child process: restore default SIGINT and run
        signal(SIGINT, SIG_DFL);
        execvp(args[0], args);
        fprintf(stderr, "%s: No such file or directory\n", args[0]);
        exit(EXIT_FAILURE);
    } else {
        // Parent: record and return immediately
        add_bg_job(pid, original_input);
        printf("Started background job %d: %s\n", pid, original_input);
    }
}
