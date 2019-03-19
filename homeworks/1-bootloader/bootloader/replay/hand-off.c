// simplest example of handing off and running process: open the tty, dup
// the fd, fork/exec my-install, wait and print exit code.
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <termios.h>

#include "trace.h"
#include "tty.h"
#include "support.h"
#include "demand.h"

// synchronously wait for <pid> to exit.  Return its exit code.
static int exit_code(int pid) {
    int wstatus;
    waitpid(pid, &wstatus, 0);
    return WEXITSTATUS(wstatus);
}

// run:
// 	1. fork
// 	2. in child:
// 		a. remap <fd> to <TRACE_FD_HANDOFF>
//		b. execvp the process in <argv[]>
//	3. in parent: 
//		a. wait for child.
//		b. print its exit code or if it crashed.
//
//  Note: that when you run my-install with tracing, the output
//  should be identical to running it raw.
void run(int fd, char *argv[]) {
	int pid = 0;
    switch (pid = fork()) {
        case -1:
            // error
            sys_die(fork, forking failed);
        case 0:
            // child
            if (dup2(fd, TRACE_FD_HANDOFF)<0)
                sys_die(dup2, dup2 failed);
            if (close(fd)<0)
                sys_die(close, child cannot close fd);
            execvp((const char *)argv[0], argv);
            fprintf(stderr, "Execvp failed.\n");
            return;
        default:
            // parent
            if (close(fd)<0)
                sys_die(close, parent cannot close fd);
            fprintf(stderr, "child %d: exited with: %d\n", pid, exit_code(pid));
    }

}

int main(int argc, char *argv[]) {
        // open tty dev, set it to be 8n1  and 115200 baud
	const char *portname = 0;
	int fd = open_tty(&portname);
	fd = set_tty_to_8n1(fd, B115200, 1);

	run(fd, &argv[1]);
	return 0;
}
