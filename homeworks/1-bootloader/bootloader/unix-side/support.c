#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

#include "demand.h"
#include "../shared-code/simple-boot.h"
#include "support.h"

// read entire file into buffer.  return it, write totat bytes to <size>
unsigned char *read_file(int *size, const char *name) {
    struct stat s;
    // get file size
    if (stat(name, &s) == -1) {
        switch (errno) {
            case EACCES:
                fprintf(stderr, "Search permission is denied for a component of the path prefix.\n");
                break;
            case EFAULT:
                fprintf(stderr, "Sb or name points to an invalid address.\n");
                break;
            case EIO:
                fprintf(stderr, "An I/O error occurs while reading from or writing to the file system.\n");
                break;
            case ELOOP:
                fprintf(stderr, "Too many symbolic links are encountered in translating the pathname.  This is taken to be indicative of a looping symbolic link.\n");
                break;
            case ENAMETOOLONG:
                fprintf(stderr, "A component of a pathname exceeds {NAME_MAX} characters, or an entire path name exceeds {PATH_MAX} characters.\n");
                break;
            case ENOENT:
                fprintf(stderr, "The named file does not exist.\n");
                break;
            case ENOTDIR:
                fprintf(stderr, "A component of the path prefix is not a directory.\n");
                break;
            default:
                fprintf(stderr, "Unknown error %d\n", errno);
        }
        *size = -1;
        return 0;
    }
    printf("The file is %d bytes\n", (int)s.st_size);
    // open file
    int fd;
    if ((fd = open(name, O_RDONLY)) == -1) {
        switch (errno) {
            case EACCES:
                fprintf(stderr, "EACCES\n");
                break;
            case EAGAIN:
                fprintf(stderr, "EAGAIN\n");
                break;
            case EFAULT:
                fprintf(stderr, "EFAULT\n");
                break;
            case EINTR:
                fprintf(stderr, "EINTR\n");
                break;
            case EIO:
                fprintf(stderr, "EIO\n");
                break;
            case ELOOP:
                fprintf(stderr, "ELOOP\n");
                break;
            default:
                fprintf(stderr, "Unknown error %d\n", errno);
        }
        *size = -1;
        return 0;
    }
    // read file into buffer
    unsigned int bufsize;
    unsigned char * buf;
    bufsize = ((s.st_size-1) / 4 + 1) * 4; // padding memory
    if ((buf = calloc(bufsize, sizeof(char))) == NULL) {
        fprintf(stderr, "Memory allocation error.\n");
        *size = -1;
        return 0;
    }
    // read the file
    read(fd, buf, s.st_size);
    // more padding
    memset(buf+s.st_size, 0, bufsize-s.st_size);
    *size = bufsize;
    // return the buffer
    return buf;
}

#define _SVID_SOURCE
#include <dirent.h>
const char *ttyusb_prefixes[] = {
    "ttyUSB",	// linux
    "tty.SLAB_USB", // mac os
    0
};

int select_ttyusb(const struct dirent * dev) {
    for (int i=0; ttyusb_prefixes[i]!=0; i++) {
        if (strstr(dev->d_name, ttyusb_prefixes[i]) == dev->d_name) {
            return 1;
        }
    }
    return 0;
}

// open the TTY-usb device:
//	- use <scandir> to find a device with a prefix given by ttyusb_prefixes
//	- returns an open fd to it
// 	- write the absolute path into <pathname> if it wasn't already
//	  given.
int open_tty(const char **portname) {
    struct dirent ** devlist;
    unsigned int num_dev;
    if (*portname == 0) {
        if ((num_dev = scandir("/dev", &devlist, select_ttyusb, NULL)) == -1) {
            fprintf(stderr, "Error occurred while listing devices.\n");
            return -1;
        }
        if (num_dev == 0) {
            fprintf(stderr, "No device found\n");
            return -1;
        }
        // construc the full path to the device
        char * full_path;
        full_path = (char *) malloc(strlen(devlist[0]->d_name)+6 * sizeof(char));
        strcpy(full_path, "/dev/");
        strcat(full_path, devlist[0]->d_name);
        *portname = (const char *)(full_path);
    }
    printf("Found device %s\n", *portname);

    int fd;
    if ((fd = open(*portname, O_RDWR)) == -1) {
        fprintf(stderr, "Error occurred while opening device %s.\n", devlist[0]->d_name);
        switch (errno) {
            case EACCES:
                fprintf(stderr, "EACCES\n");
                break;
            case EAGAIN:
                fprintf(stderr, "EAGAIN\n");
                break;
            case EFAULT:
                fprintf(stderr, "EFAULT\n");
                break;
            case EINTR:
                fprintf(stderr, "EINTR\n");
                break;
            case EIO:
                fprintf(stderr, "EIO\n");
                break;
            case ELOOP:
                fprintf(stderr, "ELOOP\n");
                break;
            case EINVAL:
                fprintf(stderr, "EINVAL\n");
                break;
            case ENOENT:
                fprintf(stderr, "ENOENT\n");
                break;
            default:
                fprintf(stderr, "Unknown error %d\n", errno);
        }
        return -1;
    }
    fprintf(stderr, "File descriptor is %d.\n", fd);
    return fd;
}
