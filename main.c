#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <errno.h>
#include <time.h>
#include <sys/wait.h>
#include "defs.h"
#include "func.h"

int main(int argc, char *argv[]) {
	/* Cars have to be sent through the args of the cli */
	if (argc == 1) {
		printf("Usage: %s car_id [car_id [car_id ...]]\n", argv[0]);
        exit(1);
	}

	/* Init variable */
	int i, loop_count, tmp, race;
	size_t cars_cnt;
	int* cars;
	int* pipes;
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
	pipes = (int*) try_sys_call_ptr(calloc(cars_cnt * 2, sizeof(int)), "Malloc failure");
	for (i = 0; i < cars_cnt; i++) {
		try_sys_call_int(pipe(&pipes[i * 2]), "Pipe failure");
	}
    printf("%d pipes initialized.\n", (int) cars_cnt * 2);

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
		pipe = pipes[car_idx * 2 + 1];
		for (i = 0; i < cars_cnt; i++) {
			try_sys_call_int(close(pipes[i * 2]), "Pipe close failure");
			if (i != car_idx) {
				try_sys_call_int(close(pipes[i * 2 + 1]), "Pipe close failure");
			}
			
		}
		free(pipes);

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
                sleep(1);

                // Variables:
                // pilote* myself: pointer to the section of shared memory usable by this pilote
                // int pipe: fd with write access to the server
                // ======= TODO ========
                //

            }
		}

        printf("Pilote %d end the tournament !", myself->car_id);
        try_sys_call_int(shmdt(shm_addr), "Shmdet failure");
        exit(EXIT_SUCCESS);


	/* Father */
	} else {
        printf("Server started\n");

        /* Close pipes with write access
         * Rearrange pipes with read access from the head of the array
         * realloc to free spaces */
        for (i = 0; i < cars_cnt * 2; i++) {
            close(pipes[i * 2 + 1]);
            pipes[i] = pipes[i * 2];
        }
        realloc(pipes, cars_cnt * sizeof(int));


		/* Attach the shared memory, fill the structure and free the array of cars */
		pilote* shm_addr = (pilote*) try_sys_call_ptr(shmat(shm_id, NULL, 0), "Shmat failure");
		for (i = 0; i < cars_cnt; i++) {
			shm_addr[i].car_id = cars[i];
            shm_addr[i].status = driving;
		}
		free(cars);

        /* Debug */
        char buffer[256];
        printf("Press ENTER to continue ");
        scanf(buffer);
        /* End debug */

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
                         * int pipes[]: array of pipe with read access
                         * pid_t pids[]: array of child's process
                         * int cars_cnt: number of pilotes
                         * int race: the id of the race (0-2: test, 3-5: qualif, 6: race)
                         * flag_alarm: is 1 when the countdown for the current race reach 0, otherwise 0*/

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