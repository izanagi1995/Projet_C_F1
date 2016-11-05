#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

int try_sys_call_int(int syscall_ret, char* msg_on_fail) {
	if (syscall_ret != -1) return syscall_ret;
	perror(msg_on_fail);
	exit(EXIT_FAILURE);
}

void* try_sys_call_ptr(void* syscall_ret, char* msg_on_fail) {
	if (syscall_ret != NULL) return syscall_ret;
	perror(msg_on_fail);
}

volatile sig_atomic_t flag_alarm;
volatile sig_atomic_t flag_race_stop;
void sighandler(int sig) {
    switch (sig) {
        case SIG_RACE_STOP:
            flag_race_stop = 1;
            break;
        case SIGALRM:
            flag_alarm = 1;
            break;
    }
}