#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <bcm2835.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/syslog.h>

#define PIN_IN RPI_BPLUS_GPIO_J8_38
#define PIN_OUT RPI_BPLUS_GPIO_J8_40

void TermSignalHandler(int signal);
void StartDaemon();

int running;

int main() {
    //bcm2835_set_debug(1);

    if (!bcm2835_init()) {
        return EXIT_FAILURE;
    }

    StartDaemon();

    running = 1;
    syslog(LOG_INFO, "Listening on gpio %d, Writing to gpio %d", PIN_IN, PIN_OUT);

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

    while (running)
    {
        if (bcm2835_gpio_eds(PIN_IN))
        {
            bcm2835_gpio_set_eds(PIN_IN);
            syslog(LOG_INFO, "gpio %d set to HIGH, shutting system down", PIN_IN);

            closelog();
            bcm2835_close();
            system("poweroff");

            return EXIT_SUCCESS;
        }

        delay(2000);
    }

    syslog(LOG_INFO, "stopping", PIN_IN);
    closelog();
    bcm2835_close();
    return EXIT_SUCCESS;
}

void TermSignalHandler(int signal)
{
    running = 0;
}

void StartDaemon()
{
    pid_t pid;

    /* Fork off the parent process */
    pid = fork();

    /* An error occurred */
    if (pid < 0) {
        exit(EXIT_FAILURE);
    }

    /* Success: Let the parent terminate */
    if (pid > 0) {
        exit(EXIT_SUCCESS);
    }

    /* On success: The child process becomes session leader */
    if (setsid() < 0) {
        exit(EXIT_FAILURE);
    }

    /* Catch, ignore and handle signals */
    signal(SIGCHLD, SIG_IGN);
    signal(SIGHUP, SIG_IGN);
    signal(SIGTERM, TermSignalHandler);
    signal(SIGKILL, TermSignalHandler);
    signal(SIGINT, TermSignalHandler);

    /* Fork off for the second time*/
    pid = fork();

    /* An error occurred */
    if (pid < 0) {
        exit(EXIT_FAILURE);
    }

    /* Success: Let the parent terminate */
    if (pid > 0) {
        exit(EXIT_SUCCESS);
    }

    /* Set new file permissions */
    umask(0);

    /* Change the working directory to the root directory */
    chdir("/");

    /* Close all open file descriptors */
    for (int x = getdtablesize(); x>0; x--)
    {
        close (x);
    }

    /* Open the log file */
    openlog ("PiSwitch", LOG_PID, LOG_DAEMON);
}