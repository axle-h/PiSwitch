#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <bcm2835.h>

#define PIN_IN RPI_BPLUS_GPIO_J8_38
#define PIN_OUT RPI_BPLUS_GPIO_J8_40
#define INIT_PIN_CHECKS 5

int main() {
    bcm2835_set_debug(1);

    if (!bcm2835_init()) {
        return 1;
    }

    // Set output pin to HIGH.
    // Informs power circuit that pi is on.
    bcm2835_gpio_fsel(PIN_OUT, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_write(PIN_OUT, HIGH);

    // Set PIN_IN to be an input with a pull-down resistor
    bcm2835_gpio_fsel(PIN_IN, BCM2835_GPIO_FSEL_INPT);
    bcm2835_gpio_set_pud(PIN_IN, BCM2835_GPIO_PUD_DOWN);

    // Check we have a high value on PIN_IN
    for(int i = 0; i < INIT_PIN_CHECKS; i++) {
        uint8_t value = bcm2835_gpio_lev(PIN_IN);
        if(value == LOW && i == (INIT_PIN_CHECKS - 1)) {
            printf("No high input on gpio %d\n", PIN_IN);
            return 1;
        }

        delay(1000);
    }

    printf("High input found on gpio %d\n", PIN_IN);
    printf("Starting daemon...\n");

    pid_t pid = fork();

    if(pid < 0) {
        printf("Failed to start daemon\n");
    }

    if(pid > 0) {
        return 0;
    }

    // Setup a low detect
    bcm2835_gpio_len(PIN_IN);

    while (1)
    {
        if (bcm2835_gpio_eds(PIN_IN))
        {
            bcm2835_gpio_set_eds(PIN_IN);
            printf("PiSwitch shutting down...\n");

            bcm2835_close();
            system("shutdown -P now");

            return 0;
        }

        delay(1000);
    }
}