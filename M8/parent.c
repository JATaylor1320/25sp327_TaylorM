#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

volatile sig_atomic_t child_exited = 0;
pid_t child_pid;

void sigusr1_handler(int signum, siginfo_t *info, void *context) {
    if (info->si_pid == child_pid) {
        printf("Parent: Received SIGUSR1 from Child (PID: %d). Sum = %d\n", info->si_pid, info->si_int);
        kill(child_pid, SIGUSR2);
    }
}

void sigchld_handler(int signum, siginfo_t *info, void *context) {
    if (info->si_pid == child_pid) {
        child_exited = 1;
    }
}

int main() {
    struct sigaction sa_usr1, sa_chld;

    child_pid = fork();
    if (child_pid == -1) {
        perror("fork");
        return EXIT_FAILURE;
    }

    if (child_pid == 0) {
        execl("./child", "./child", NULL);
        perror("execl");
        return EXIT_FAILURE;
    }

    printf("Parent: fork-exec successful. Child pid (%d)\n", child_pid);

    sa_usr1.sa_sigaction = sigusr1_handler;
    sa_usr1.sa_flags = SA_SIGINFO;
    sigemptyset(&sa_usr1.sa_mask);
    if (sigaction(SIGUSR1, &sa_usr1, NULL) == -1) {
        perror("sigaction SIGUSR1");
        return EXIT_FAILURE;
    }

    sa_chld.sa_sigaction = sigchld_handler;
    sa_chld.sa_flags = SA_SIGINFO | SA_NOCLDSTOP;
    sigemptyset(&sa_chld.sa_mask);
    if (sigaction(SIGCHLD, &sa_chld, NULL) == -1) {
        perror("sigaction SIGCHLD");
        return EXIT_FAILURE;
    }

    while (!child_exited) {
        printf("Parent: Working...\n");
        sleep(2);
    }

    int status;
    waitpid(child_pid, &status, 0);
    printf("Parent: Child (PID: %d) has terminated with status %d. Exiting.\n", child_pid, WEXITSTATUS(status));
    sleep(1);
    return EXIT_SUCCESS;
}