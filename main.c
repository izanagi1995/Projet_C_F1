#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <time.h>
#include <errno.h>
#include <limits.h>
#include <boolean.h>
#include "defs.h"
#include "tools.c"


volatile sig_atomic_t flag = 0;
void sig_handler(int sig) {
	flag = 1;
}

int main(int argc, char *argv[]){
	int status = S_INIT;

	srand(time(NULL));
	int i;

	int cars[22] = {44, 6, 5, 7, 3, 33, 19, 77, 11, 27, 26, 55, 14, 22, 9, 12, 20, 30, 8, 21, 31, 94};
	int cars_length = sizeof(cars) / sizeof(int); 

	int msgqkey;
	int msgqid;
	do {
		msgqkey = rand();
		msgqid = msgget(rand(), IPC_CREAT | IPC_EXCL | 0660);
	} while (errno == EEXIST)
	msgqid = try_sys_call_int(msgqid, "msgget failure");
	msgbuf buffer;

	int pid;
	int shm_key;
	int temp_id;
	for (i = 0; i < cars_length; ++i) {
		do {
			shm_key = rand();
			temp_id = shmget(shm_key, sizeof(f1_pilote), IPC_CREAT | IPC_EXCL | 0660);
		} while (errno == EEXIST);
		pid = try_sys_call_int(fork(), "fork failure");
		if (pid > 0) break;
	}

	if (pid > 0) {
		/* Register pid of child*/
		int *pids = malloc(sizeof(int) * cars_length);

		// read pids from message queue, not saved before for security purpose
		// send a signal to all children when all pid have been recieved

		/* Init memory sharing
		%2		=> mem_id given by shmget()
		%2 + 1	=> att_id given by shmat()*/
		int *shmems = malloc(sizeof(int) * cars_length * 2);

		// read keys from messages queue, not saved before for security purpose
		// set the status on S_TEST_RACES and send a signal to children

	} else {


		int shmem = try_sys_call_int(shmatt(shm_key, NULL, 0), "shmatt failure");
		pid = getpid();

		char *pid_as_char = (char *) malloc(sizeof(pid_t) * sizeof(char)); // fuck magic numbers :DDD
		sprintf(pid_as_char, "%d", (int)pid);
		buffer = {1, pid_as_char}

		msgsnd(msgqid, buffer, sizeof(pid_t), 0);

		while(true) if (flag == 1) break;
		flag = 0;

		char *shmkey_as_char = (char *) malloc(sizeof(int) * sizeof(char));
		sprintf(shmkey_as_char, "%d", shm_key);
		buffer = {2, shmkey_as_char}
		msgsnd(msgqid, buffer, sizeof(int))

		while(true) if (flag == 1) break;
		flag = 0;

		// S_TEST_RACE
	}
}