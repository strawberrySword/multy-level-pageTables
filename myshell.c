#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#define BACKGROUND_PROCESS 0
#define INPUT_REDIRECTION 1
#define OUTPUT_REDIRECTION 2

int single_process_handle(int mode, int split, char **arglist);
int double_process_handle(int split, char **);

int process_arglist(int count, char **arglist)
{
    if (strcmp(arglist[count - 1], "&") == 0)
        return single_process_handle(BACKGROUND_PROCESS, count - 1, arglist);

    for (int i = 0; i < count; i++)
    {
        if (strcmp(arglist[i], "|") == 0)
            return double_process_handle(i, arglist);

        if (strcmp(arglist[i], "<") == 0)
            return single_process_handle(INPUT_REDIRECTION, i, arglist);

        if (strcmp(arglist[i], ">>") == 0)
            return single_process_handle(OUTPUT_REDIRECTION, i, arglist);
    }
    return single_process_handle(-1, count, arglist);
}

int prepare(void)
{
    // Signal handler to prevent zombies.
    // Whenever a children process that not been waited for is done the SIGCHLS signal is called and by ignoring it, the zombie process is removed from the table.
    if (signal(SIGCHLD, SIG_IGN) == SIG_ERR)
    {
        perror("failed ceating signal handler for SIGCHLD");
        return -1;
    }

    // Signal handler to prevent shell from terminating when crtl+c is pressed. (we write a new line instead).
    // IMPORTANT: this signal handler is overridden when creating a children process
    if (signal(SIGINT, SIG_IGN) == SIG_ERR)
    {
        perror("failed ceating signal handler for SIGCHLD");
        return -1;
    }

    return 0;
}
int finalize(void)
{
    return 0;
}

int single_process_handle(int mode, int split, char **arglist)
{
    arglist[split] = NULL;

    pid_t pid = fork();
    if (pid < 0)
    {
        perror("failed to create child process");
        exit(1);
    }
    if (pid == 0)
    {
        if (mode != BACKGROUND_PROCESS)
        {
            if (signal(SIGINT, SIG_DFL) == SIG_ERR)
            {
                perror("failed ceating signal handler for SIGCHLD");
                exit(1);
            }
        }
        if (mode == INPUT_REDIRECTION)
        {
            int file_desc = open(arglist[split + 1], O_RDONLY);
            if (file_desc == -1)
            {
                perror("failed opening file");
                exit(1);
            }
            if (dup2(file_desc, STDIN_FILENO) == -1)
            {
                perror("failed dup");
                exit(1);
            }
            if (close(file_desc) == -1)
            {
                perror("failed closing file");
                exit(1);
            }
        }
        if (mode == OUTPUT_REDIRECTION)
        {
            int file_desc = open(arglist[split + 1], O_WRONLY | O_CREAT | O_APPEND);
            if (file_desc == -1)
            {
                perror("failed opening file ");
                exit(1);
            }
            if (dup2(file_desc, STDOUT_FILENO) == -1)
            {
                perror("failed dup");
                exit(1);
            }

            if (close(file_desc) == -1)
            {
                perror("failed closing file");
                exit(1);
            }
        }
        execvp(arglist[0], arglist);
        perror("the command or one of the arguments are invalid");
        exit(1);
    }
    else
    {
        if (mode != BACKGROUND_PROCESS)
            if (waitpid(pid, NULL, 0) == -1 && errno != ECHILD && errno != EINTR)
            {
                perror("failure in waiting");
                exit(1);
            }
        return 1;
    }
}

int double_process_handle(int split, char **arglist)
{
    arglist[split] = NULL;
    int pfds[2];
    if (pipe(pfds) == -1)
    {
        perror("pipe");
        exit(1);
    }
    pid_t pid_first = fork();
    if (pid_first == 0)
    {
        if (signal(SIGINT, SIG_DFL) == SIG_ERR)
        {
            perror("failed ceating signal handler for SIGCHLD");
            exit(1);
        }
        // change stdout to the pipe and close the pipe read
        if (close(pfds[0]) == -1)
        {
            perror("failed closing pipe");
            exit(1);
        }
        if (dup2(pfds[1], STDOUT_FILENO) == -1)
        {
            perror("failure in dup");
            exit(1);
        }
        if (close(pfds[1]) == -1)
        {
            perror("failed closing pipe");
            exit(1);
        }
        execvp(arglist[0], arglist);
        // if execvp succeed we will not reach this part of the code
        perror("the command or one of the arguments are invalid");
        exit(1);
    }
    pid_t pid_second = fork();
    if (pid_second == 0)
    {
        if (signal(SIGINT, SIG_DFL) == SIG_ERR)
        {
            perror("failed ceating signal handler for SIGCHLD");
            exit(1);
        }
        // change stdin to the pipe and close the pipe write
        if (close(pfds[1]) == -1)
        {
            perror("failed closing pipe");
            exit(1);
        }
        if (dup2(pfds[0], STDIN_FILENO) == -1)
        {
            perror("failure in dup");
            exit(1);
        }

        if (close(pfds[0]) == -1)
        {
            perror("failed closing pipe");
            exit(1);
        }
        execvp(arglist[split + 1], arglist + split + 1);
        perror("the command or one of the arguments are invalid");
        exit(1);
    }
    close(pfds[0]);
    close(pfds[1]);
    if (waitpid(pid_first, NULL, 0) == -1 && errno != ECHILD && errno != EINTR)
    {
        perror("failure in waiting");
        exit(1);
    }
    if (waitpid(pid_second, NULL, 0) == -1 && errno != ECHILD && errno != EINTR)
    {
        perror("failure in waiting");
        exit(1);
    }
    return 1;
}