#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>

#include "defs.h"

int try_sys_call_int(int syscall_ret, char* msg_on_fail) {
	if (syscall_ret != -1) return syscall_ret;
	perror(msg_on_fail);
	exit(EXIT_FAILURE);
}

void* try_sys_call_ptr(void* syscall_ret, char* msg_on_fail) {
	if (syscall_ret != NULL) return syscall_ret;
	perror(msg_on_fail);
	exit(EXIT_FAILURE);
}

sem_t* sem;

void init_semaphore(){
    sem = sem_open ("pSem", O_CREAT | O_EXCL, 0644, 1);
    sem_unlink ("pSem");
    // 1 = Only 1 process at a time doing action on shm
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

void doSector(pilote* p, float time, int pipe){
    p->time = time;
    if(++(p->sector) == 3){
        p->sector = 0;
        p->lap_cnt++;
    }
    char status[] = "driving";
    write(pipe, status, sizeof(status));
}