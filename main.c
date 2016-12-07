#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <errno.h>
#include <time.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>
#include "defs.h"
#include "func.h"

int main(int argc, char *argv[]) {
	/* Cars have to be sent through the args of the cli */
	if (argc == 1) {
		printf("Usage: %s car_id [car_id [car_id ...]]\n", argv[0]);
		exit(1);
	}

	init_semaphore();

	/* Init variable */
	int i, loop_count, tmp, race;
	size_t cars_cnt;
	int* cars;
	int pipes[2];
	int shm_key, shm_id;
	pid_t* pids;

	srand((unsigned int) time(NULL));

	/* Read the args to get the cars */
	cars_cnt = (size_t) argc - 1;
	cars = (int*) try_sys_call_ptr(calloc(cars_cnt, sizeof(int)), "Malloc failure");
	for (i = 0; i < cars_cnt; i++) {
		cars[i] = (int) strtol(argv[i + 1], (char **)NULL, 10);
		printf("Got car #%d: %d\n", i, cars[i]);
	}

	/* Init the pipes */
    try_sys_call_int(pipe(pipes), "Pipe failure");
	for (i = 0; i < cars_cnt; i++) {

	}
	printf("pipes initialized.\n");

	/*	Init the shamed memory with a random key. Fail after 30 try*/
	loop_count = 0;
	do {
		shm_key = rand();
		shm_id = shmget(shm_key, cars_cnt * sizeof(pilote), IPC_CREAT | IPC_EXCL | 0660);
	} while (loop_count++ < 30 && errno == EEXIST);
	try_sys_call_int(shm_id, "Shmget failure");
	printf("Shared memory has ID: %d\n", shm_id);

	/* Fork */
	pids = (pid_t*) try_sys_call_ptr(malloc(cars_cnt * sizeof(pid_t)), "Malloc failure");
	for (i = 0; i < cars_cnt; i++) {
		pids[i] = try_sys_call_int(fork(), "Fork failure");
		if (pids[i] == 0) break;
	}

	/* Child */
	if (i < cars_cnt) {

        srand(time(NULL) ^ (getpid()<<16));

		/* Init variable */
		int sig, car_idx, pipe;
		sigset_t sigset;

		/* Save the index in cars and free useless arrays */
		car_idx = i;
		free(cars);
		free(pids);

		/* Attach the shared memory and get our structure */
		pilote* shm_addr = (pilote*) try_sys_call_ptr(shmat(shm_id, NULL, 0), "Shmat failure");
		pilote* myself = &shm_addr[car_idx];

		/* Save one pipe with write acces to the server
		 * close all the other
		 * free the memory segment of pipes */
		pipe = pipes[1];
		close(pipes[0]);
		//free(pipes);

		printf("Process ID %d is car at index %d and has access to %p.\n", getpid(), car_idx, myself);

		/* Signals handled by signal syscall */
		signal(SIG_RACE_STOP, sighandler);

		/* Signals handled by sigwait syscall */
		sigemptyset(&sigset);
		sigaddset(&sigset, SIG_RACE_START);
		sigprocmask(SIG_BLOCK, &sigset, NULL);

		/* Loop until the pilote loose */
		while (myself->status != end) {
			/* Wait for the race start */
			if (sigwait(&sigset, &sig) != 0) {
				perror("Sigwait failure");
				exit(EXIT_FAILURE);
			}

			/* Enter main loop, break when SIG_RACE_STOP is fired */
			printf("Race started\n");
			flag_race_stop = 0;
			while (!flag_race_stop) {

				//CRITICAL SECTION
				sem_wait(sem);
				// Variables:
				// pilote* myself: pointer to the section of shared memory usable by this pilote
				// int pipe: fd with write access to the server
				// ======= TODO ========
				//


				//int intTime = rand() % 50 + 50;
                int intTime = 1;
				float time = (float)(intTime);

				doSector(myself, time, pipe);

                if((rand() % 50) < 10){
					myself->status = end;
                    myself->has_changed = 1;
				}
				sem_post(sem);
                if(myself->status == end){
                    printf("Pilote %d left the tournament! \n", myself->car_id);
                    char status[] = "end";
                    write(pipe, status, sizeof(status));
                    break;
                }

				sleep(intTime);

			}
		}

		//printf("Pilote %d left the tournament! \n", myself->car_id);
		try_sys_call_int(shmdt(shm_addr), "Shmdet failure");
		exit(EXIT_SUCCESS);


	/* Father */
	} else {
		printf("Server started\n");
        int pipe = pipes[0];
        close(pipes[1]);

        //Init scoreboard
        scoreboard* scoreboards = calloc(cars_cnt, sizeof(scoreboard));

		//CRITICAL SECTION
		sem_wait(sem);
		/* Attach the shared memory, fill the structure and free the array of cars */
		pilote* shm_addr = (pilote*) try_sys_call_ptr(shmat(shm_id, NULL, 0), "Shmat failure");
		for (i = 0; i < cars_cnt; i++) {
			shm_addr[i].car_id = cars[i];
			shm_addr[i].status = driving;
		}
		sem_post(sem);
		free(cars);

		/* Debug */
		char buffer[256];
		printf("Press ENTER to continue ");
		scanf(buffer);
		/* End debug */

		signal(SIGALRM, sighandler);
		
		printf("[S] Enter main loop\n");
		for (race = 0; race < 7; race++) {

			/* Wait one second to sync everyone*/
			printf("[S] Sync cars...\n");
			sleep(1);

			/* Start a race */
			printf("[S] Start race %d:%d !\n", race / 3 + 1, race % 3 + 1);
			for (i = 0; i < cars_cnt; i++) kill(pids[i], SIG_RACE_START);



			/* Enter main loop with specific code by race type */
			switch (race) {
				case 0:
				case 1:
				case 2:
					/* Loop until the alarm signal is fired */
					alarm(test_times[race]);
					flag_alarm = 0;
					while (!flag_alarm) {
						/* Main loop for test races */

						/* TODO
						 * available variable are:
						 * int pipe: the pipe
						 * pid_t pids[]: array of child's process
						 * int cars_cnt: number of pilotes
						 * int race: the id of the race (0-2: test, 3-5: qualif, 6: race)
						 * flag_alarm: is 1 when the countdown for the current race reach 0, otherwise 0*/
                        int data = flush_pipe(pipe);
                        if(data != 0){
                            //CRITICAL SECTION
                            sem_wait(sem);

                            for (i = 0; i < cars_cnt; i++){
                                pilote* current = &shm_addr[i];
                                if(current->has_changed == 1){
                                    printf("Pilote %d has updated\n", current->car_id);
                                    current->has_changed = 0;
                                }
                            }

                            sem_post(sem);
                        }




					}
					break;
				case 3:
				case 4:
				case 5:
					/* Loop until the alarm signal is fired */
					alarm(qualif_times[race % 3]);
					flag_alarm = 0;
					while (!flag_alarm) {
						/* Main loop for qualification races */

						// TODO

					}
					break;
				case 6:

					// TODO

					break;
			}

			/* Stop the current race */
			printf("End race !\n");
			for (i = 0; i < cars_cnt; i++) kill(pids[i], SIG_RACE_STOP);
		}

		/* Detache the shared memory */
		try_sys_call_int(shmdt(shm_addr), "Shmdet failure");

		/* Wait for children to stop */
		for (i = 0; i < cars_cnt; i++) {
			printf("[S] Wait for child %d to stop.\n", pids[i]);
			try_sys_call_int(waitpid(pids[i], &tmp, 0), "Waitpid failure");
		}

		exit(EXIT_SUCCESS);
	}
}
