// child.c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>

volatile sig_atomic_t sigtstp_received = 0;
volatile sig_atomic_t sigusr2_received = 0;
volatile sig_atomic_t sigterm_received = 0;

pid_t parent_pid;
int sum = 0;

void sigtstp_handler(int signum, siginfo_t *info, void *context) {
    if (info->si_pid == parent_pid) {
        sigtstp_received = 1;
    }
}

void sigusr2_handler(int signum, siginfo_t *info, void *context) {
    if (info->si_pid == parent_pid) {
        sigusr2_received = 1;
    }
}

void sigterm_handler(int signum, siginfo_t *info, void *context) {
    sigterm_received = 1;
}

int main() {
    parent_pid = getppid();
    struct sigaction sa_tstp, sa_usr2, sa_term;

    sa_tstp.sa_sigaction = sigtstp_handler;
    sa_tstp.sa_flags = SA_SIGINFO;
    sigemptyset(&sa_tstp.sa_mask);
    if (sigaction(SIGTSTP, &sa_tstp, NULL) == -1) {
        perror("sigaction SIGTSTP");
        return EXIT_FAILURE;
    }

    sa_usr2.sa_sigaction = sigusr2_handler;
    sa_usr2.sa_flags = SA_SIGINFO;
    sigemptyset(&sa_usr2.sa_mask);
    if (sigaction(SIGUSR2, &sa_usr2, NULL) == -1) {
        perror("sigaction SIGUSR2");
        return EXIT_FAILURE;
    }

    sa_term.sa_sigaction = sigterm_handler;
    sa_term.sa_flags = SA_SIGINFO;
    sigemptyset(&sa_term.sa_mask);
    if (sigaction(SIGTERM, &sa_term, NULL) == -1) {
        perror("sigaction SIGTERM");
        return EXIT_FAILURE;
    }

    while (!sigterm_received) {
        printf("Child: Running... Use 'kill -SIGTSTP %d' to interrupt me.\n", getpid());
        sum += 10;
        sleep(1);

        if (sigtstp_received) {
            printf("Child: SIGTSTP received. Sending sum to parent.\n");
            union sigval val;
            val.sival_int = sum;
            if (sigqueue(parent_pid, SIGUSR1, val) == -1) {
                perror("sigqueue");
                return EXIT_FAILURE;
            }
            sigtstp_received = 0;
        }

        if (sigusr2_received) {
            printf("Child: Received SIGUSR2 from Parent (%d). Current sum = %d\n", parent_pid, sum);
            sigusr2_received = 0;
        }
    }

    printf("Child: Received SIGTERM, exiting...\n");

    return EXIT_SUCCESS;
}