#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    // Check for command line arguments
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <cmd1> [cmd2] ...\n", argv[0]);
        exit(EINVAL);
    }

    // pipe process id
    pid_t pid;

    // Create pipes
    int fd1[2];
    int fd2[2];

    if (pipe(fd1) == -1) {
        perror("pipe");
        exit(errno);
    }

    // run the first process, reading from nothing and writing to fd1
    pid = fork();

    if (pid == -1) {
        perror("fork");
        exit(errno);
    }

    // child process
    if (pid == 0) {
        // redirect stdout to fd1
        close(fd1[0]);
        if (dup2(fd1[1], STDOUT_FILENO) == -1) {
            perror("dup2");
            exit(errno);
        }
        close(fd1[1]);

        // execute first argument
        char *args[] = {argv[1], NULL};
        execvp(argv[1], args);
        perror("execvp");
        exit(errno);
    }

    // parent process
    else if (pid > 0) {
        close(fd1[1]);

        // start the rest of the processes, which will each read from fd1 and write
        // to fd2, which then duplicates itself to fd1
        for (int i = 2; i < argc; i++) {
            // Create pipe fd2
            if (pipe(fd2) == -1) {
                perror("pipe");
                exit(errno);
            }

            pid = fork();

            if (pid == -1) {
                perror("fork");
                exit(errno);
            }

            // child process
            if (pid == 0) {
                close(fd2[0]);

                // read from fd1
                if (dup2(fd1[0], STDIN_FILENO) == -1) {
                    perror("dup2");
                    exit(errno);
                }
                close(fd1[0]);

                // write to fd2 unless it is the last command, in that case, just write
                // to stdout
                if (i < argc - 1) {
                    if (dup2(fd2[1], STDOUT_FILENO) == -1) {
                        perror("dup2");
                        exit(errno);
                    }
                    close(fd2[1]);
                }

                // exec command
                char *args[] = {argv[i], NULL};
                execvp(argv[i], args);
                perror("execvp");
                exit(errno);
            }

            // parent process
            else if (pid > 0) {
                // close read end of fd1
                close(fd1[0]);
                // close write end of previous fd2 pipe
                close(fd2[1]);

                // copy fd2 contents to fd1 read side
                fd1[0] = fd2[0];
            }
        }
    }

    // clean up all child processes
    int final = 0;
    int status = 0;
    for (int i = 1; i < argc; i++) {
        if (wait(&status) == -1) {
            perror("wait");
            exit(errno);
        }
        if (WIFEXITED(status)) {
            if (WEXITSTATUS(status) != 0) {
                final = WEXITSTATUS(status);
            }
        } else {
            final = 1;
        }
    }
    return final;
}
