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

	pid_t pid;
	pid_t *pids = malloc(sizeof(pid_t) * cars_length);
	int shm_key;
	int temp_id;
	for (i = 0; i < cars_length; ++i) {
		do {
			shm_key = rand();
			temp_id = shmget(shm_key, sizeof(f1_pilote), IPC_CREAT | IPC_EXCL | 0660);
		} while (errno == EEXIST);
		pid = try_sys_call_int(fork(), "fork failure");
		if (pid == 0) break;
		pids[i] = pid
	}

	if (pid > 0) {
		// Father

		signal(SIGALARM, sig_handler);

		/* Init memory sharing
		%2		=> mem_id given by shmget()
		%2 + 1	=> att_id given by shmat()*/
		int *shmems = malloc(sizeof(int) * cars_length * 2);
		for (i = 0; i < cars_length * 2; i+=2) {
			msgrcv(msgqid, buffer, sizeof(int), 1, 0);
			shmems[i] = shmget((int) strtol(buffer->mtext, NULL, 10), sizeof(f1_pilote), 0660);
			shmems[i + 1] = shmatt(shmems[i], NULL, 0);
		}

		/* Init the first lap for test race 1 */
		sector_times *sector_times_T1 = malloc(sizeof(sector_times) * cars_length);
		for (i = 0; i < cars_length; i++) {
			f1_pilote *pilote = (f1_pilote *)shmems[i * 2 + 1];
			f1_pilote->pilote_id = cars[i];
			f1_pilote->times[0] = sector_times[i];
		}

		/* Start the race ! */
		for (i = 0; i < cars_length; i++) kill(pids[i], SIGTERM);
		alarm(T_T1);

		while (!flag) {
			// read pid from msgq
			// save f1_pilote.s([123]) to sector_times[pid].s$1
			// on s1 create a new sector_time, set pts
		}
		flag = 0;



	} else {
		// Child

		// Register a signal to call sig_handler

		char *shmkey_as_char = (char *) malloc(sizeof(int) * sizeof(char));
		sprintf(shmkey_as_char, "%d", shm_key);
		buffer = {1, shmkey_as_char}
		msgsnd(msgqid, &buffer, sizeof(int))

		while(true) {
			if (flag == 1) break;
			sleep(1);
		}
		flag = 0;


	}
}