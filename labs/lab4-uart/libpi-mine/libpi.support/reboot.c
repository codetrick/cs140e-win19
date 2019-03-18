#include "rpi.h"

void reboot(void) {
        const int PM_RSTC = 0x3F10001c;
        const int PM_WDOG = 0x3F100024;
        const int PM_PASSWORD = 0x5a000000;
        const int PM_RSTC_WRCFG_FULL_RESET = 0x00000020;

        // give output time to flush.
 //       delay(10000);  Isn't working.

	int i;
        for(i = 0; i < 100000; i++)
                dummy(i);

        // timeout = 1/16th of a second? (whatever)
        PUT32(PM_WDOG, PM_PASSWORD | 1);
        PUT32(PM_RSTC, PM_PASSWORD | PM_RSTC_WRCFG_FULL_RESET);
	while(1); 
}

// print out message so bootloader exits
void clean_reboot(void) {
        puts("DONE!!!\n");
	delay_ms(30); 		// give a chance to get flushed
        reboot();
}
