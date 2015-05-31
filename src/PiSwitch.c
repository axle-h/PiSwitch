#include <stdlib.h>
#include <bcm2835.h>
#include <sys/syslog.h>
#include <signal.h>
#include <stdio.h>
#include "config.h"
#include "daemon.h"

#define LOG_IDENTITY "PiSwitch"
#define CONFIG_FILE "/etc/gpio/piswitch.cfg"

void SetupGpio(uint8_t gpioIn, uint8_t gpioOut);
void SetupSignals();
void SignalHandler(int signal);

bool running;

int main(int argc, char **argv) {
    PiSwitchConfig config;
    if(!TryGetPiSwitchConfig(CONFIG_FILE, argc, argv, &config)) {
        fprintf(stderr, "Configuration error\n");
        return -1;
    }

    // Open the log file
    openlog(LOG_IDENTITY, LOG_PID, config.RunAsDaemon ? LOG_DAEMON : LOG_USER);

    syslog(LOG_INFO, "Listening on gpio: %u, Writing to gpio: %u, Poll frequency: %u",
           config.GpioIn, config.GpioOut, config.PollFrequency);

    bcm2835_set_debug((uint8_t) config.DebugEnabled);

    running = true;

    if(config.RunAsDaemon) {
        StartDaemon(config.PidFile);
    }

    SetupSignals();

    SetupGpio(config.GpioIn, config.GpioOut);

    bool powerOff = false;
    while (running)
    {
        if(bcm2835_gpio_lev(config.GpioIn) == HIGH) {

            // Check it again!
            bcm2835_delay(500);
            if(bcm2835_gpio_lev(config.GpioIn) == HIGH) {
                powerOff = true;
                break;
            }
        }

        bcm2835_delay(config.PollFrequency);
    }

    if(powerOff) {
        syslog(LOG_INFO, "gpio %u set to HIGH, shutting system down", config.GpioIn);
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

void SetupSignals() {
    // Catch, ignore and handle signals
    signal(SIGCHLD, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
    signal(SIGHUP, SignalHandler);
    signal(SIGTERM, SignalHandler);
    signal(SIGINT, SignalHandler);
}

void SignalHandler(int signal) {
    running = false;
}

void SetupGpio(uint8_t gpioIn, uint8_t gpioOut) {
    if (!bcm2835_init()) {
        exit(EXIT_FAILURE);
    }

    // Set output pin to HIGH.
    // Informs power circuit that pi is on.
    bcm2835_gpio_fsel(gpioOut, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_write(gpioOut, HIGH);

    // Set PIN_IN to be an input with a pull-down resistor
    bcm2835_gpio_fsel(gpioIn, BCM2835_GPIO_FSEL_INPT);
    bcm2835_gpio_set_pud(gpioIn, BCM2835_GPIO_PUD_DOWN);

    // Wait a while
    bcm2835_delay(100);
}