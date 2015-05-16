#include <stdlib.h>
#include <bcm2835.h>
#include <sys/syslog.h>
#include "config.h"
#include "daemon.h"

#define LOG_IDENTITY "PiSwitch"
#define CONFIG_FILE "/etc/gpio/piswitch.cfg"

void SetupGpio(uint8_t gpioIn, uint8_t gpioOut);
void SignalHander(int signal);

bool running;

int main() {
    // Open the log file
    openlog(LOG_IDENTITY, LOG_PID, LOG_DAEMON);

    PiSwitchConfig config;
    if(!TryGetPiSwitchConfig(CONFIG_FILE, &config)) {
        syslog(LOG_ERR, "Failed to read config file %s", CONFIG_FILE);
        return -1;
    }

    syslog(LOG_INFO, "Debug mode: %s, Listening on gpio: %u, Writing to gpio: %u, Poll frequency: %u",
           config.DebugEnabled ? "true" : "false", config.GpioIn, config.GpioOut, config.PollFrequency);

    bcm2835_set_debug((uint8_t) config.DebugEnabled);

    running = true;

    if(!config.DebugEnabled) {
        StartDaemon(config.PidFile, SignalHander);
    }

    SetupGpio(config.GpioIn, config.GpioOut);

    bool powerOff = false;
    while (running)
    {
        if (bcm2835_gpio_eds(config.GpioIn))
        {
            bcm2835_gpio_set_eds(config.GpioIn);
            powerOff = true;
            break;
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

void SignalHander(int signal) {
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

    // Setup a high detect
    // Switch will send a HIGH when it switch is rocked to off
    bcm2835_gpio_hen(gpioIn);
}