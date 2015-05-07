#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <bcm2835.h>

#define PIN_IN RPI_BPLUS_GPIO_J8_38
#define PIN_OUT RPI_BPLUS_GPIO_J8_40

int main() {
    //bcm2835_set_debug(1);

    if (!bcm2835_init()) {
        return 1;
    }

    pid_t pid = fork();

    if(pid < 0) {
        printf("Failed to start daemon\n");
    }

    if(pid > 0) {
        return 0;
    }

    // Set output pin to HIGH.
    // Informs power circuit that pi is on.
    bcm2835_gpio_fsel(PIN_OUT, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_write(PIN_OUT, HIGH);

    // Set PIN_IN to be an input with a pull-down resistor
    bcm2835_gpio_fsel(PIN_IN, BCM2835_GPIO_FSEL_INPT);
    bcm2835_gpio_set_pud(PIN_IN, BCM2835_GPIO_PUD_DOWN);

    // Setup a high detect
    // Switch will send a HIGH when it switch is rocked to off
    bcm2835_gpio_hen(PIN_IN);

    while (1)
    {
        if (bcm2835_gpio_eds(PIN_IN))
        {
            bcm2835_gpio_set_eds(PIN_IN);
            printf("PiSwitch shutting down\n");

            bcm2835_close();
            system("poweroff");

            return 0;
        }

        delay(2000);
    }
}