#include <stdio.h>
#include <stdlib.h>
#include <bcm2835.h>

#define PIN_IN RPI_GPIO_P1_23
#define PIN_OUT RPI_GPIO_P1_24

int main() {
    bcm2835_set_debug(1);

    if (!bcm2835_init()) {
        return 1;
    }

    // Set output pin to HIGH.
    // Informs power circuit that pi is on.
    bcm2835_gpio_fsel(PIN_OUT, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_write(PIN_OUT, HIGH);

    // Set PIN_IN to be an input, with a pull-up resistor and a low detect enable
    bcm2835_gpio_fsel(PIN_IN, BCM2835_GPIO_FSEL_INPT);
    bcm2835_gpio_set_pud(PIN_IN, BCM2835_GPIO_PUD_UP);
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