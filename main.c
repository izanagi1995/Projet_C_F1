#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <errno.h>
#include <time.h>
#include <semaphore.h>
#include "defs.h"
#include "func.h"
#include <ncurses.h>
#include "curses-utils.h"

int main(int argc, char *argv[]) {
	/* Cars have to be sent through the args of the cli */
	if (argc == 1) {
		printf("Usage: %s car_id [car_id [car_id ...]]\n", argv[0]);
		exit(1);
	}


	init_semaphore();



	/* Init variable */
	int i, loop_count, tmp, race, carToGet, carToGetAccumulator = 0;
	size_t cars_cnt;
	int* cars;
    int* startFinalRace;
	int pipes[2];
	int shm_key, shm_id;
	pid_t* pids;
    int bestLap = 999;
    int bestS1 = 999;
    int bestS2 = 999;
    int bestS3 = 999;

	srand((unsigned int) time(NULL));

	/* Read the args to get the cars */
	cars_cnt = (size_t) argc - 1;

	cars = (int*) try_sys_call_ptr(calloc(cars_cnt, sizeof(int)), "Malloc failure");
    startFinalRace = (int*) try_sys_call_ptr(calloc(cars_cnt, sizeof(int)), "Malloc failure");
	for (i = 0; i < cars_cnt; i++) {
		cars[i] = (int) strtol(argv[i + 1], (char **)NULL, 10);
	}

	/* Init the pipes */
    try_sys_call_int(pipe(pipes), "Pipe failure");
	for (i = 0; i < cars_cnt; i++) {

	}

	/*	Init the shamed memory with a random key. Fail after 30 try*/
	loop_count = 0;
	do {
		shm_key = rand();
		shm_id = shmget(shm_key, cars_cnt * sizeof(pilote), IPC_CREAT | IPC_EXCL | 0660);
	} while (loop_count++ < 30 && errno == EEXIST);
	try_sys_call_int(shm_id, "Shmget failure");

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
			flag_race_stop = 0;
			while (!flag_race_stop && myself->status != eliminated) {

				//CRITICAL SECTION
				sem_wait(sem);
				// Variables:
				// pilote* myself: pointer to the section of shared memory usable by this pilote
				// int pipe: fd with write access to the server
				// ======= TODO ========
				//


				int intTime = rand()%30 + 13;
				float time = (float)(intTime);

                if(myself->sector == 2){
                    if((rand() % 100) < 10){
                        myself->status = pitstop;
                        myself->has_changed = 1;
                    }
                }

				doSector(myself, time, pipe);

                if((rand() % 100) < 2){
					myself->status = out;
                    myself->has_changed = 1;
				}


				sem_post(sem);
                if(myself->status == out){
                    char status[] = "out";
                    write(pipe, status, sizeof(status));
                    break;
                }

				usleep(intTime * 1000000 / 60);

                if(myself->status == pitstop) myself->status = driving;

			}
		}

		//printf("Pilote %d left the tournament! \n", myself->car_id);
		exit(EXIT_SUCCESS);


	/* Father */
	} else {


        //CRITICAL SECTION
        sem_wait(sem);
        /* Attach the shared memory, fill the structure and free the array of cars */
        pilote* shm_addr = (pilote*) try_sys_call_ptr(shmat(shm_id, NULL, 0), "Shmat failure");
        for (i = 0; i < cars_cnt; i++) {
            shm_addr[i].car_id = cars[i];
            shm_addr[i].status = driving;
        }
        sem_post(sem);

        int pipe = pipes[0];
        close(pipes[1]);
        bestLap = 999;
        //Init scoreboard
        scoreboard* scoreboards = calloc(cars_cnt, sizeof(scoreboard));
        for(int sc_index = 0; sc_index < cars_cnt; sc_index++){
            scoreboard* sc = &scoreboards[sc_index];
            //Init races and bestLaps
            for(int race_index = 0; race_index < 7; race_index++){
                sc->car_id = cars[sc_index];
                sc->last_lap[race_index] = (lap*)(malloc(sizeof(lap)));
                sc->races[race_index] = sc->last_lap[race_index];
                sc->bestlaps[race_index] = (bestlap){ .best_s1 = 999, .best_s2 = 999, .best_s3 = 999, .best_lap = 999 };
            }
        }

        initscr();
        cbreak();
        noecho();
        refresh();
        initScreen(cars_cnt);
        testView();
        writeStatus("Starting", 0, 0);
        refresh();







		/* Debug */
		/* End debug */

		signal(SIGALRM, sighandler);
		
		for (race = 0; race < 7; race++) {

            resetWins();
            bestLap = 999;
            bestS1 = 999;
            bestS2 = 999;
            bestS3 = 999;

            if(race / 3 == 0 || race / 3 == 1){
                for (int car_counter = 0; car_counter < cars_cnt; car_counter++) {
                    pilote *current = &shm_addr[car_counter];
                    if(current->status == out){
                        current->status = driving;
                    }
                }
            }
            for(int sc_index = 0; sc_index < cars_cnt; sc_index++){
                scoreboard* sc = &scoreboards[sc_index];
                //Init races and bestLaps
                sc->car_id = cars[sc_index];
                sc->last_lap[race] = (lap*)(calloc(1, sizeof(lap)));
                sc->races[race] = sc->last_lap[race];
                sc->bestlaps[race] = (bestlap){ .best_s1 = 999, .best_s2 = 999, .best_s3 = 999, .best_lap = 999 };
            }

			/* Wait one second to sync everyone*/

            for (int car_counter = 0; car_counter < cars_cnt; car_counter++) {
                pilote *current = &shm_addr[car_counter];
                current->lap_cnt = 0;
                current->sector = 0;
                updateCarStatus(car_counter, current->car_id, current->status);
            }

			sleep(1);

            writeStatus("Running", race/3+1, race%3+1);

			/* Start a race */

            //Restore crashed car for test and qualification

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
                            for (int car_counter = 0; car_counter < cars_cnt; car_counter++){
                                pilote* current = &shm_addr[car_counter];
                                scoreboard* scb = &scoreboards[car_counter];
                                if(current->has_changed == 1){
                                    current->has_changed = 0;
                                    updateCarStatus(car_counter, current->car_id, current->status);
                                    if(current->status == out ){
                                        continue;
                                    }
                                    if(current->status == pitstop){
                                        scb->last_lap[race]->time_pit = current->time;
                                    }
                                    if(current->sector == 0){
                                        scb->last_lap[race]->time_s3 = current->time;

                                        //Set best lap
                                        if(scb->bestlaps[race].best_lap > scb->last_lap[race]->time_s1 + scb->last_lap[race]->time_s2 + scb->last_lap[race]->time_s3){
                                            scb->bestlaps[race].best_lap = scb->last_lap[race]->time_s1 + scb->last_lap[race]->time_s2 + scb->last_lap[race]->time_s3;
                                            if(bestLap > scb->bestlaps[race].best_lap){
                                                bestLap = scb->bestlaps[race].best_lap;
                                                updateCarBest(car_counter, current->car_id, scb->bestlaps[race], 4);
                                            }else{
                                                updateCarBest(car_counter, current->car_id, scb->bestlaps[race], 0);
                                            }

                                        }
                                        //Set best s1
                                        if(scb->bestlaps[race].best_s1 > scb->last_lap[race]->time_s1){
                                            scb->bestlaps[race].best_s1 = scb->last_lap[race]->time_s1;
                                            if(bestS1 > scb->bestlaps[race].best_s1){
                                                bestS1 = scb->bestlaps[race].best_s1;
                                                updateCarBest(car_counter, current->car_id, scb->bestlaps[race], 1);
                                            }else{
                                                updateCarBest(car_counter, current->car_id, scb->bestlaps[race], 0);
                                            }
                                        }
                                        //Set best s2
                                        if(scb->bestlaps[race].best_s2 > scb->last_lap[race]->time_s2){
                                            scb->bestlaps[race].best_s2 = scb->last_lap[race]->time_s2;
                                            if(bestS2 > scb->bestlaps[race].best_s2){
                                                bestS2 = scb->bestlaps[race].best_s2;
                                                updateCarBest(car_counter, current->car_id, scb->bestlaps[race], 2);
                                            }else{
                                                updateCarBest(car_counter, current->car_id, scb->bestlaps[race], 0);
                                            }
                                        }
                                        //Set best s3
                                        if(scb->bestlaps[race].best_s3 > scb->last_lap[race]->time_s3){
                                            scb->bestlaps[race].best_s3 = scb->last_lap[race]->time_s3;
                                            if(bestS3 > scb->bestlaps[race].best_s3){
                                                bestS3 = scb->bestlaps[race].best_s3;
                                                updateCarBest(car_counter, current->car_id, scb->bestlaps[race], 3);
                                            }else{
                                                updateCarBest(car_counter, current->car_id, scb->bestlaps[race], 0);
                                            }
                                        }

                                        updateCarLap(car_counter, scb->last_lap[race]->time_s1 + scb->last_lap[race]->time_s2 + scb->last_lap[race]->time_s3, current->lap_cnt);


                                        lap* temp = (lap*)malloc(sizeof(lap));
                                        temp->nextlap = NULL;
                                        scb->last_lap[race]->nextlap = temp;
                                        scb->last_lap[race] = temp;


                                    }else{
                                        switch (current->sector){
                                            case 1:
                                                scb->last_lap[race]->time_s1 = current->time;
                                                break;
                                            case 2:
                                                scb->last_lap[race]->time_s2 = current->time;
                                                break;
                                            case 3:
                                                scb->last_lap[race]->time_s3 = current->time;
                                                break;
                                        }
                                    }


                                }
                            }

                            sem_post(sem);
                        }


					}

                    //Calculate results
                    rank_item* unsorted_ranking = calloc(cars_cnt, sizeof(rank_item));
                    for(int rank_idx = 0; rank_idx < cars_cnt; rank_idx++){
                        scoreboard sc = scoreboards[rank_idx];
                        unsorted_ranking[rank_idx] = (rank_item){.car_id = sc.car_id, .bestlap = sc.bestlaps[race].best_lap};
                    }
                    mergesort(unsorted_ranking, cars_cnt, sizeof(rank_item), &compare_rank_item);
                    showResults(unsorted_ranking);
                    getch();


					break;
				case 3:

				case 4:

				case 5:
                    if(race == 3 || race == 4){
                        carToGet = 6;
                    }else{
                        carToGet = 10;
                    }
					/* Loop until the alarm signal is fired */
					alarm(qualif_times[race % 3]);
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
                            for (int car_counter = 0; car_counter < cars_cnt; car_counter++){
                                pilote* current = &shm_addr[car_counter];
                                if(current->status == eliminated){
                                    current->has_changed = 0;
                                    continue;
                                }

                                scoreboard* scb = &scoreboards[car_counter];
                                if(current->has_changed == 1){
                                    current->has_changed = 0;
                                    updateCarStatus(car_counter, current->car_id, current->status);
                                    if(current->status == out || current->status == end){
                                        continue;
                                    }
                                    if(current->status == pitstop){
                                        scb->last_lap[race]->time_pit = current->time;
                                    }
                                    if(current->sector == 0){
                                        scb->last_lap[race]->time_s3 = current->time;

                                        //Set best lap
                                        if(scb->bestlaps[race].best_lap > scb->last_lap[race]->time_s1 + scb->last_lap[race]->time_s2 + scb->last_lap[race]->time_s3){
                                            scb->bestlaps[race].best_lap = scb->last_lap[race]->time_s1 + scb->last_lap[race]->time_s2 + scb->last_lap[race]->time_s3;
                                            if(bestLap > scb->bestlaps[race].best_lap){
                                                bestLap = scb->bestlaps[race].best_lap;
                                                updateCarBest(car_counter, current->car_id, scb->bestlaps[race], 4);
                                            }else{
                                                updateCarBest(car_counter, current->car_id, scb->bestlaps[race], 0);
                                            }

                                        }
                                        //Set best s1
                                        if(scb->bestlaps[race].best_s1 > scb->last_lap[race]->time_s1){
                                            scb->bestlaps[race].best_s1 = scb->last_lap[race]->time_s1;
                                            if(bestS1 > scb->bestlaps[race].best_s1){
                                                bestS1 = scb->bestlaps[race].best_s1;
                                                updateCarBest(car_counter, current->car_id, scb->bestlaps[race], 1);
                                            }else{
                                                updateCarBest(car_counter, current->car_id, scb->bestlaps[race], 0);
                                            }
                                        }
                                        //Set best s2
                                        if(scb->bestlaps[race].best_s2 > scb->last_lap[race]->time_s2){
                                            scb->bestlaps[race].best_s2 = scb->last_lap[race]->time_s2;
                                            if(bestS2 > scb->bestlaps[race].best_s2){
                                                bestS2 = scb->bestlaps[race].best_s2;
                                                updateCarBest(car_counter, current->car_id, scb->bestlaps[race], 2);
                                            }else{
                                                updateCarBest(car_counter, current->car_id, scb->bestlaps[race], 0);
                                            }
                                        }
                                        //Set best s3
                                        if(scb->bestlaps[race].best_s3 > scb->last_lap[race]->time_s3){
                                            scb->bestlaps[race].best_s3 = scb->last_lap[race]->time_s3;
                                            if(bestS3 > scb->bestlaps[race].best_s3){
                                                bestS3 = scb->bestlaps[race].best_s3;
                                                updateCarBest(car_counter, current->car_id, scb->bestlaps[race], 3);
                                            }else{
                                                updateCarBest(car_counter, current->car_id, scb->bestlaps[race], 0);
                                            }
                                        }

                                        updateCarLap(car_counter, scb->last_lap[race]->time_s1 + scb->last_lap[race]->time_s2 + scb->last_lap[race]->time_s3, current->lap_cnt);


                                        lap* temp = (lap*)malloc(sizeof(lap));
                                        temp->nextlap = NULL;
                                        scb->last_lap[race]->nextlap = temp;
                                        scb->last_lap[race] = temp;


                                    }else{
                                        switch (current->sector){
                                            case 1:
                                                scb->last_lap[race]->time_s1 = current->time;
                                                break;
                                            case 2:
                                                scb->last_lap[race]->time_s2 = current->time;
                                                break;
                                            case 3:
                                                scb->last_lap[race]->time_s3 = current->time;
                                                break;
                                        }
                                    }


                                }
                            }

                            sem_post(sem);
                        }


                    }

                    //Calculate results
                    unsorted_ranking = calloc(cars_cnt, sizeof(rank_item));
                    for(int rank_idx = 0; rank_idx < cars_cnt; rank_idx++){
                        scoreboard sc = scoreboards[rank_idx];
                        pilote* p = getPiloteByCarId(sc.car_id, shm_addr, cars_cnt);
                        float time = p->status == eliminated ? 99999:(sc.bestlaps[race].best_lap);git
                        unsorted_ranking[rank_idx] = (rank_item){.car_id = sc.car_id, .bestlap = time};
                    }
                    mergesort(unsorted_ranking, cars_cnt, sizeof(rank_item), &compare_rank_item);

                    //Eliminate last cars
                    char debugbuffer[512];
                    sprintf(debugbuffer, "Loop from %d to %d, decrease, %d get, %d acc", cars_cnt - carToGetAccumulator - 1, cars_cnt - (carToGetAccumulator + carToGet), carToGet, carToGetAccumulator);
                    writeHistory(debugbuffer);
                    int cmp = (int)(cars_cnt - (carToGetAccumulator + carToGet));
                    for(int r = cars_cnt - carToGetAccumulator - 1; r >= cmp; r--){
                        if(r>=0){
                            startFinalRace[r] = unsorted_ranking[r].car_id;
                            pilote* p = getPiloteByCarId(unsorted_ranking[r].car_id, shm_addr, cars_cnt);
                            p->status = eliminated;
                            char buffer[512];
                            sprintf(buffer, "Car n°%d has been eleminated", p->car_id);
                            writeHistory(buffer);
                            updateCarStatus(p->car_id - 1, p->car_id, eliminated);
                        }
                    }
                    getch();

                    carToGetAccumulator += carToGet;
                    showResults(unsorted_ranking);

                    getch();


					break;
				case 6:

					// TODO

					break;
			}

			/* Stop the current race */
			for (i = 0; i < cars_cnt; i++) kill(pids[i], SIG_RACE_STOP);
		}

		/* Detach the shared memory */
		try_sys_call_int(shmdt(shm_addr), "Shmdet failure");

		/* Wait for children to stop */
		for (i = 0; i < cars_cnt; i++) {
			try_sys_call_int(waitpid(pids[i], &tmp, 0), "Waitpid failure");
		}
        echo();
        nocbreak();
        endwin();
		exit(EXIT_SUCCESS);
	}
}

/*
│Car n°17 has been eleminated                                                                                                 ││
│Car n°16 has been eleminated                                                                                                 ││
│Car n°15 has been eleminated                                                                                                 ││
│Car n°6 has been eleminated                                                                                                  ││
│Car n°4 has been eleminated                                                                                                  ││
│Car n°3 has been eleminated


│Car n°15 has been eleminated                                                                                                 ││
│Car n°14 has been eleminated                                                                                                 ││
│Car n°12 has been eleminated                                                                                                 ││
│Car n°9 has been eleminated                                                                                                  ││
│Car n°8 has been eleminated                                                                                                  ││
│Car n°6 has been eleminated



 */