#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>

#include "daemon.h"

void DoFork();

void StartDaemon(const char *pidFile, TermSignalHandler termSignalHandler) {
    DoFork();

    // On success: The child process becomes session leader
    if (setsid() < 0) {
        exit(EXIT_FAILURE);
    }

    // Fork off for the second time
    DoFork();

    // Set new file permissions
    umask(0);

    // Change the working directory to the root directory
    chdir("/");

    // Close all open file descriptors
    for (int x = getdtablesize(); x>0; x--)
    {
        close (x);
    }

    // Catch, ignore and handle signals
    signal(SIGCHLD, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
    signal(SIGHUP, SIG_IGN);
    signal(SIGTERM, termSignalHandler);
    signal(SIGINT, termSignalHandler);

    // Record PID
    int lfp = open(pidFile, O_RDWR | O_CREAT, 0640);
    if (lfp < 0) {
        exit(EXIT_FAILURE);
    }

    if (lockf(lfp, F_TLOCK, 0) < 0) {
        // Can not lock
        exit(EXIT_SUCCESS);
    }

    char str[10];
    sprintf(str, "%d\n", getpid());
    write(lfp, str, strlen(str));
}

void DoFork() {
    pid_t pid;

    // Fork off the parent process
    pid = fork();

    // An error occurred
    if (pid < 0) {
        exit(EXIT_FAILURE);
    }

    // Success: Let the parent terminate
    if (pid > 0) {
        exit(EXIT_SUCCESS);
    }
}