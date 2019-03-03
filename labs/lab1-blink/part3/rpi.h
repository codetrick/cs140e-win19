#ifndef __RPI_H__
#define __RPI_H__

// return *(volatile unsigned *)addr
unsigned (get32)(volatile void *addr);

// *(volatile unsigned *)addr = v
void (put32)(volatile void *addr, unsigned val);

// return *(volatile unsigned *)addr
unsigned short (get16)(volatile void *addr);

// *(volatile unsigned *)addr = v
void (put16)(volatile void *addr, unsigned short val);

// return *(volatile unsigned *)addr
unsigned char (get8)(volatile void *addr);

// *(volatile unsigned *)addr = v
void (put8)(volatile void *addr, unsigned char val);

#endif /* __RPI_H__ */
