/* Fan Yang's implementation of the gpio functions.
 */
#include "gpio.h"

// see broadcomm documents for magic addresses.
#define PERIPHERAL_BASE       0x3F000000UL
#define GPIO_BASE       ( PERIPHERAL_BASE + 0x200000UL )
volatile unsigned *gpio_fsel0 = (volatile unsigned *)(GPIO_BASE + 0x00);
volatile unsigned *gpio_set0  = (volatile unsigned *)(GPIO_BASE + 0x1C);
volatile unsigned *gpio_clr0  = (volatile unsigned *)(GPIO_BASE + 0x28);
volatile unsigned *gpio_lev0  = (volatile unsigned *)(GPIO_BASE + 0x34);

void gpio_init(void) {
    // do nothing
}

void gpio_set_function(unsigned int pin, unsigned int function) {
    // see page 91 of BCM2835 documentation
    // may need to block. will fix later.
    if (pin < GPIO_PIN_FIRST || pin > GPIO_PIN_LAST)
        return;
    gpio_fsel0[pin/10] = (gpio_fsel0[pin/10] & ~(0b111 << 3*(pin%10))) | (function << 3*(pin%10));
}

unsigned int gpio_get_function(unsigned int pin) {
    // see page 91 of BCM2835 documentation
    if (pin < GPIO_PIN_FIRST || pin > GPIO_PIN_LAST)
        return GPIO_INVALID_REQUEST;
    return (gpio_fsel0[pin/10] >> 3*(pin%10)) & 0b111;
}

void gpio_set_input(unsigned int pin) {
    gpio_set_function(pin, GPIO_FUNC_INPUT);
}

void gpio_set_output(unsigned int pin) {
    // see page 91 of BCM2835 documentation
    gpio_set_function(pin, GPIO_FUNC_OUTPUT);
}

void gpio_write(unsigned int pin, unsigned int val) {
    if (pin < GPIO_PIN_FIRST || pin > GPIO_PIN_LAST)
        return;
    switch (val) {
        case 0:
            gpio_clr0[pin/32] |= 1 << pin%32;
            break;
        case 1:
            gpio_set0[pin/32] |= 1 << pin%32;
            break;
        default:
            // do nothing
            break;
    }
}

unsigned int gpio_read(unsigned int pin) {
    if (pin < GPIO_PIN_FIRST || pin > GPIO_PIN_LAST)
        return GPIO_INVALID_REQUEST;
    return (gpio_lev0[pin/32] >> (pin%32)) & 0b1;
}

void gpio_set_pud(unsigned pin, unsigned pud) {
    // not implemented
}

void gpio_set_pullup(unsigned pin) {
    // not implemented
}

void gpio_set_pulldown(unsigned pin) {
    // not implemented
}
