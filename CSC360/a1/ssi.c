#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <unistd.h>
#include <limits.h>
#ifndef HOST_NAME_MAX
#define HOST_NAME_MAX 255
#endif
#include <signal.h>
#include <sys/types.h>
#include "background.h"
#include <sys/wait.h>


#define MAX_INPUT 1024

// Flag to control main loop
int running = 1;

// Handle Ctrl+C (SIGINT) at the prompt
void handle_sigint(int signo) {
    (void)signo; // unused

    // Print a newline to move to a new line after ^C
    write(STDOUT_FILENO, "\n", 1);

    // Reprint your shell prompt
    print_prompt();
}

// Display username@hostname:cwd >
void print_prompt(void) {
    char cwd[PATH_MAX];
    char hostname[HOST_NAME_MAX];
    char *username = getlogin();

    if (gethostname(hostname, sizeof(hostname)) != 0)
        strcpy(hostname, "unknown");
    if (getcwd(cwd, sizeof(cwd)) == NULL)
        strcpy(cwd, "?");

    printf("%s@%s: %s > ", username ? username : "user", hostname, cwd);
    fflush(stdout);
}

void execute_foreground(char *cmd) {
    char *args[64]; // arbitrary max number of args
    int i = 0;
    char *token = strtok(cmd, " ");
    while (token != NULL && i < 63) {
        args[i++] = token;
        token = strtok(NULL, " ");
    }
    args[i] = NULL; // execvp requires NULL-terminated array

    if (args[0] == NULL)
        return; // nothing to run

    // Check for built-in cd
    if (strcmp(args[0], "cd") == 0) {
        handle_cd(args);
        return;
    }

    // Step 2: Create a new process
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork failed");
        return;
    }

    if (pid == 0) {
        // Child process: run the command
        signal(SIGINT, SIG_DFL); // allow Ctrl+C to kill child

        execvp(args[0], args);

        // If execvp returns, it failed
        fprintf(stderr, "%s: No such file or directory\n", args[0]);
        exit(1);
    } else {
        // Parent process: wait for the child to finish
        int status;
        waitpid(pid, &status, 0);
    }
}

void handle_cd(char **args) {
    char *target = args[1];  // first argument after "cd"

    if (target == NULL || strcmp(target, "~") == 0) {
        // cd or cd ~ → home directory
        target = getenv("HOME");
        if (target == NULL) {
            fprintf(stderr, "cd: HOME not set\n");
            return;
        }
    }

    if (chdir(target) != 0) {
        perror("cd");
    }
}

void handle_sigchld(int sig) {
    (void)sig;  // unused
    int status;
    pid_t pid;

    // Reap all finished children
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        extern void notify_job_terminated(pid_t pid, int status);
        notify_job_terminated(pid, status);
        print_prompt();
    }
}

void execute_command(char *input) {
    //make copy of input to tokenize
    char input_copy[MAX_INPUT];
    strcpy(input_copy, input);
    
    char *args[64];
    int i = 0;
    char *token = strtok(input_copy, " ");
    while (token && i < 63) {
        args[i++] = token;
        token = strtok(NULL, " ");
    }
    args[i] = NULL;

    if (args[0] == NULL)
        return;

    // ---------- BUILT-IN COMMANDS ----------
    if (strcmp(args[0], "cd") == 0) {
        handle_cd(args);
        return;
    }

    if (strcmp(args[0], "bglist") == 0) {
        print_bg_jobs();
        return;
    }

    if (strcmp(args[0], "bg") == 0) {
        if (args[1] == NULL) {
            printf("Usage: bg <command>\n");
            return;
        }

        // Shift arguments left to remove "bg"
        for (int j = 1; args[j]; j++)
            args[j - 1] = args[j];
        args[i - 1] = NULL;

        // Recreate command string for display
        char command_copy[MAX_INPUT];
        strcpy(command_copy, input + 3); // skip "bg "

        execute_background(args, command_copy);
        return;
    }

    // ---------- DEFAULT: FOREGROUND COMMAND ----------
    execute_foreground(input);
}

int main(void) {
    // Set up signal handling for Ctrl+C
    signal(SIGINT, handle_sigint);
    // Set up signal handling for terminated background jobs
    signal(SIGCHLD, handle_sigchld);

    char input[MAX_INPUT];

    while (running) {
        check_bg_jobs(); // Check for any finished background jobs

        print_prompt();

        // Read user input (Ctrl+D produces NULL)
        if (fgets(input, sizeof(input), stdin) == NULL) {
            printf("\n");
            break;
        }

        // Trim newline
        input[strcspn(input, "\n")] = '\0';

        // Ignore empty input
        if (strlen(input) == 0)
            continue;

        // Exit command
        if (strcmp(input, "exit") == 0) {
            printf("Bye!\n");
            break;
        }

        execute_command(input);
    }

    return 0;
}