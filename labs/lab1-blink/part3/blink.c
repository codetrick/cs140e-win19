/*
 * blink for arbitrary pins.    
 * Implement:
 *	- gpio_set_output;
 *	- gpio_set_on;
 * 	- gpio_set_off.
 *
 *
 * - try deleting volatile.
 * - change makefile to use -O3
 * - get code to work by calling out to a set32 function to set the address.
 * - initialize a structure with all the fields.
 */

#include "rpi.h"

// see broadcomm documents for magic addresses.
#define PERIPHERAL_BASE       0x3F000000UL
#define GPIO_BASE       ( PERIPHERAL_BASE + 0x200000UL )
volatile unsigned *gpio_fsel0 = (volatile unsigned *)(GPIO_BASE + 0x00);
volatile unsigned *gpio_set0  = (volatile unsigned *)(GPIO_BASE + 0x1C);
volatile unsigned *gpio_clr0  = (volatile unsigned *)(GPIO_BASE + 0x28);

// XXX might need memory barriers.
void gpio_set_output(unsigned pin) {
	// gpio_fsel1  --- set 'pin' to output.
    put32(gpio_fsel0+pin/10, (get32(gpio_fsel0+pin/10) & ~(0b111 << 3*(pin%10))) | (0b001 << 3*(pin%10)));
}

void gpio_set_on(unsigned pin) {
	// use gpio_set0
    put32(gpio_set0+pin/32, 1 << pin%32);
}
void gpio_set_off(unsigned pin) {
	// use gpio_clr0
    put32(gpio_clr0+pin/32, 1 << pin%32);
}

// countdown 'ticks' cycles; the asm probably isn't necessary.
void delay(unsigned ticks) {
	while(ticks-- > 0)
		asm("add r1, r1, #0");
}

// define: dummy to immediately return and PUT32 as above.
 void reboot(void) {
      const int PM_RSTC = PERIPHERAL_BASE + 0x0010001c;
      const int PM_WDOG = PERIPHERAL_BASE + 0x00100024;
      const int PM_PASSWORD = 0x5a000000;
      const int PM_RSTC_WRCFG_FULL_RESET = 0x00000020;
      put32((volatile int *)PM_WDOG, PM_PASSWORD | 1);
      put32((volatile int *)PM_RSTC, PM_PASSWORD | PM_RSTC_WRCFG_FULL_RESET);
 }


int notmain ( void ) {
    int led = 20;

    gpio_set_output(led);
    for (int i=0; i<100; i++) {
            gpio_set_on(led);
            delay(1000000);
            gpio_set_off(led);
            delay(1000000);
    }
    reboot();
    return 0;
}
