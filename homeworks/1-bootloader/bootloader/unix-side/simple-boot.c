#include <assert.h>
#include <fcntl.h>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include "demand.h"
#include "trace.h"

#define __SIMPLE_IMPL__
#include "../shared-code/simple-boot.h"

static void send_byte(int fd, unsigned char b) {
	if(write(fd, &b, 1) < 0)
		panic("write failed in send_byte\n");
}
static unsigned char get_byte(int fd) {
	unsigned char b;
	int n;
	if((n = read(fd, &b, 1)) != 1)
		panic("read failed in get_byte: expected 1 byte, got %d\n",n);
	return b;
}

// NOTE: the other way to do is to assign these to a char array b and 
//	return *(unsigned)b
// however, the compiler doesn't have to align b to what unsigned 
// requires, so this can go awry.  easier to just do the simple way.
// we do with |= to force get_byte to get called in the right order 
// 	(get_byte(fd) | get_byte(fd) << 8 ...) 
// isn't guaranteed to be called in that order b/c | is not a seq point.
static unsigned get_uint(int fd) {
        unsigned u = get_byte(fd);
        u |= get_byte(fd) << 8;
        u |= get_byte(fd) << 16;
        u |= get_byte(fd) << 24;
        trace_read32(u);
        return u;
}

void put_uint(int fd, unsigned u) {
        trace_write32(u);
	// mask not necessary.
        send_byte(fd, (u >> 0)  & 0xff);
        send_byte(fd, (u >> 8)  & 0xff);
        send_byte(fd, (u >> 16) & 0xff);
        send_byte(fd, (u >> 24) & 0xff);
}

// simple utility function to check that a u32 read from the 
// file descriptor matches <v>.
void expect(const char *msg, int fd, unsigned v) {
	unsigned x = get_uint(fd);
	if(x != v) 
		panic("%s: expected %x, got %x\n", msg, v,x);
}

// unix-side bootloader: send the bytes, using the protocol.
// read/write using put_uint() get_unint().
void simple_boot(int fd, const unsigned char * buf, unsigned n) { 
    unsigned checksum = crc32(buf, n);
    put_uint(fd, SOH);
    put_uint(fd, n);
    put_uint(fd, checksum);
    usleep(1000); // sleep for 5 ms
    printf("Header sent\n");
    // check for errors
    switch (get_uint(fd)) {
        case BAD_START:
            panic("header bad start!\n");
        case TOO_BIG:
            panic("Program too big!\n");
        case SOH:
            if (get_uint(fd) != crc32((const void *)&n, 4))
                panic("CRC32 checksum is wrong.\n");
            if (get_uint(fd) != checksum)
                panic("CRC32 disagree.\n");
            break;
        default:
            panic("Did not receive SOH from rpi.\n");
    }
    for (int i=0; i<n; i+=4) {
        put_uint(fd, *((unsigned *)&buf[i]));
    }
    put_uint(fd, EOT);
    printf("EOT sent!\n");
    usleep(1000); // sleep for 1 ms
    switch (get_uint(fd)) {
        case BAD_END:
            panic("Bad end of transmission!\n");
        case BAD_CKSUM:
            panic("Data corruption detected via checksum.\n");
        case ACK:
            printf("Got ACK!\n");
            break;
        default:
            panic("Did not receive ACK from rpi.\n");
    }
    printf("Bootloading success!\n");
    return;
}
