#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <bcm2835.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/syslog.h>
#include <string.h>

#define DEBUG 0
#define PIN_IN RPI_BPLUS_GPIO_J8_38
#define PIN_OUT RPI_BPLUS_GPIO_J8_40
#define PID_FILE "/var/run/PiSwitch.pid"

void StartDaemon();
void SetupSignals();
void SetupGpio();
int WaitForSignalOrSwitch();

void DoFork();
void TermSignalHandler(int signal);

int running;

int main() {
    bcm2835_set_debug(DEBUG);

    StartDaemon();
    running = 1;

    SetupSignals();

    SetupGpio();

    // Open the log file
    openlog("PiSwitch", LOG_PID, LOG_DAEMON);
    syslog(LOG_INFO, "Listening on gpio %d, Writing to gpio %d", PIN_IN, PIN_OUT);

    int powerOff = WaitForSignalOrSwitch();
    if(powerOff) {
        syslog(LOG_INFO, "gpio %d set to HIGH, shutting system down", PIN_IN);
    } else {
        syslog(LOG_INFO, "signal reveived, stopping");
    }

    closelog();
    bcm2835_close();

    if(powerOff) {
        system("poweroff");
    }

    return EXIT_SUCCESS;
}

void SetupGpio() {
    if (!bcm2835_init()) {
        exit(EXIT_FAILURE);
    }

    // Set output pin to HIGH.
    // Informs power circuit that pi is on.
    bcm2835_gpio_fsel(PIN_OUT, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_write(PIN_OUT, HIGH);

    // Set PIN_IN to be an input with a pull-down resistor
    bcm2835_gpio_fsel(PIN_IN, BCM2835_GPIO_FSEL_INPT);
    bcm2835_gpio_set_pud(PIN_IN, BCM2835_GPIO_PUD_DOWN);

    // Wait a while
    delay(2000);

    // Setup a high detect
    // Switch will send a HIGH when it switch is rocked to off
    bcm2835_gpio_hen(PIN_IN);
}

int WaitForSignalOrSwitch() {
    while (running)
    {
        if (bcm2835_gpio_eds(PIN_IN))
        {
            bcm2835_gpio_set_eds(PIN_IN);
            return 1;
        }

        delay(2000);
    }

    return 0;
}

void StartDaemon()
{
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
}

void SetupSignals() {
    // Catch, ignore and handle signals
    signal(SIGCHLD, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
    signal(SIGHUP, SIG_IGN);
    signal(SIGTERM, TermSignalHandler);
    signal(SIGINT, TermSignalHandler);

    // Record PID
    int lfp = open(PID_FILE, O_RDWR | O_CREAT, 0640);
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

void TermSignalHandler(int signal)
{
    running = 0;
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