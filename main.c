#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <time.h>
#include <errno.h>
#include <limits.h>
#include <signal.h>
#include "defs.h"
#include "tools.h"

volatile sig_atomic_t flag;
void sig_handler(int sig) {
	flag = 1;
}

/**
@argv[1] STATUS
@argv[2:] list of pilote_id
*/
int main(int argc, char *argv[]){
    if (argc < 2) {
        printf("Usage: %s [race_type {1,2,3}] [car_id [car_id ...]]\n", argv[0]);
        exit(1);
    }

    int i, j;

    // Get the status from argv[0]
	int status = (int) strtol(argv[1], (char **)NULL, 10);


    // Dynamically get the list of pilotes
	int *cars = (int*) malloc((argc - 1) * sizeof(int));
    int cars_cnt = argc - 2;
    for (i = 0; i < cars_cnt; i++) {
        cars[i] = (int) strtol(argv[i + 2], (char **)NULL, 10);
    }
    

    // Message queue
    int msgq_key;
    int msgq_id;

    // Shared memory
    int shm_key;
    int shm_id;
    pilote_status *shm_addrs;

    pid_t pid;
    pid_t *pids = (pid_t*) malloc(sizeof(pid_t) * cars_cnt);

    // Update the seed of random
    srand(time(NULL));


    // Get a free message queue
	do {
		msgq_key = rand();
		msgq_id = msgget(rand(), IPC_CREAT | IPC_EXCL | 0660);
	} while (errno == EEXIST);
	try_sys_call_int(msgq_id, "msgget failure");

    // Get a free shared memory segment
    do {
        shm_key = rand();
        shm_id = shmget(shm_key, sizeof(pilote_status), IPC_CREAT | IPC_EXCL | 0660);
    } while (errno == EEXIST);
    try_sys_call_int(shm_id, "shmget failure");

    // Fork once by pilote
	for (i = 0; i < cars_cnt; i++) {
        pid = try_sys_call_int(fork(), "fork failure");
        if (pid == 0) break;
        pids[i] = pid;
    }

	if (pid > 0) { // Father side
        if ((shm_addrs = (pilote_status*) shmat(shm_id, NULL, 0)) == (void*) -1){
            perror("shmat failure");
            exit(1);
        }

        pilote *pilotes = malloc(sizeof(pilote*) * cars_cnt);
        for (i = 0; i < cars_cnt; i++) {
            pilotes[i].process_id = pids[i];
            pilotes[i].pilote_id = cars[i];
            pilotes[i].status = &shm_addrs[i];
        }
        signal(SIGALRM, sig_handler);

        switch (status) {
            case S_TEST_RACES:
                break;
            case S_QUALIFYING_RACES:
                break;
            case S_RACE:
                break;

        }



	} else { // Child Side
        int car_index = i;
        int car = cars[car_index];
        free(cars);
        free(pids);
	}
}