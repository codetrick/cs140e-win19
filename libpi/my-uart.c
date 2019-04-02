#include "rpi.h"
#include "gpio.h"

// #define PUT32(a,b) (*((volatile unsigned *) a) = b)
// #define GET32(a) (*((volatile unsigned *) a))

#define AUX_BASE 0x3F215000UL
#define AUX_ENB (AUX_BASE + 0x04)
#define AUX_MU_IO_REG (AUX_BASE + 0x40)
#define AUX_MU_IER_REG (AUX_BASE + 0x44)
#define AUX_MU_IIR_REG (AUX_BASE + 0x48)
#define AUX_MU_LCR_REG (AUX_BASE + 0x4C)
#define AUX_MU_MCR_REG (AUX_BASE + 0x50)
#define AUX_MU_LSR_REG (AUX_BASE + 0x54)
#define AUX_MU_CNTL_REG (AUX_BASE + 0x60)
#define AUX_MU_STAT_REG (AUX_BASE + 0x64)
#define AUX_MU_BAUD (AUX_BASE + 0x68)

#define AUX_RX_FIFO_CLR (1<<1)
#define AUX_TX_FIFO_CLR (1<<2)

#define AUX_RX_READY (1<<0)
#define AUX_TX_EMPTY (1<<5)

// use this if you need memory barriers.
void dev_barrier(void) {
	dmb();
	dsb();
}

void uart_init(void) {
    // set up GPIO pin 14 (Tx) and 15 (Rx)
    gpio_set_function(GPIO_TX, GPIO_FUNC_ALT5);
    gpio_set_function(GPIO_RX, GPIO_FUNC_ALT5);
    // barrier
    dev_barrier();
    // first enable uart
    PUT8(AUX_ENB, GET32(AUX_ENB) | 0x01);
    // disable Tx and Rx while we set things up
    PUT8(AUX_MU_CNTL_REG, GET8(AUX_MU_CNTL_REG) & ~0x03);
    // disable interrupts
    PUT8(AUX_MU_IER_REG, 0x00);
    // disable RTS
    PUT8(AUX_MU_MCR_REG, 0x00);
    // clear receive and transmit FIFO
    PUT8(AUX_MU_IIR_REG, AUX_RX_FIFO_CLR | AUX_TX_FIFO_CLR);
    // set the UART in 8bit mode
    PUT8(AUX_MU_LCR_REG, 0x03); // The manual lied. Had to set 0b11 to get 8bit.
    // set the baud rate to 115200 according to p11
    PUT32(AUX_MU_BAUD, 270);
    // now enable Tx and Rx
    PUT8(AUX_MU_CNTL_REG, GET8(AUX_MU_CNTL_REG) | 0x03);
    // we are done!
}

int uart_getc(void) {
    // block until there's incoming data
    while (!(GET8(AUX_MU_LSR_REG) & AUX_RX_READY)); //check data ready
    return GET8(AUX_MU_IO_REG);
}

void uart_putc(unsigned c) {
    // block until there's space in the Tx FIFO
    while (!(GET8(AUX_MU_LSR_REG) & AUX_TX_EMPTY)); // check Tx empty
    PUT8(AUX_MU_IO_REG, c);
}
