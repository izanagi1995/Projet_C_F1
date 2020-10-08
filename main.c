#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/mman.h>
#include <errno.h>
#include <sys/time.h>
#include <ncurses.h>
#include "defs.h"
#include "func.h"
#include "curses-utils.h"
#include "server.h"

int main(int argc, char *argv[]) {
	/* Cars have to be sent through the args of the cli */
	if (argc == 1) {
		printf("Usage: %s car_id [car_id [car_id ...]]\n", argv[0]);
		exit(1);
	}


	init_semaphore();



	/* Init variable */
	int i, loop_count, tmp, race, carToGet, carToGetAccumulator = 0, r;
	size_t cars_cnt;
	int* cars;
	pilote** startFinalRace;
	int pipes[2];
	int shm_key, shm_id;
	pid_t* pids;
	float bestLap, bestS1, bestS2, bestS3;
	int elimination_counter = 0;


	srand((unsigned int) time(NULL));

	/* Read the args to get the cars */
	cars_cnt = (size_t) argc - 1;

	cars = (int*) try_sys_call_ptr(calloc(cars_cnt, sizeof(int)), "Malloc failure");
	startFinalRace = (pilote**) try_sys_call_ptr(calloc(cars_cnt, sizeof(pilote*)), "Malloc failure");
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
		srand((unsigned int) (time(NULL) ^ (getpid() << 16)));

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
					if((rand() % 100) < 2){
						myself->status = pitstop;
						myself->has_changed = 1;
					}
				}



				float confirmed_time = doSector(myself, time, pipe);

				if((rand() % 1000) < 3){
					myself->status = out;
					myself->has_changed = 1;
				}


				sem_post(sem);
				if(myself->status == out){
					char status[] = "out";
					write(pipe, status, sizeof(status));
					break;
				}

				usleep((useconds_t) (confirmed_time * 1000000 / 60 * simulation_divider));

				if(myself->status == pitstop) myself->status = driving;

			}
		}

		//printf("Pilote %d left the tournament! \n", myself->car_id);
		exit(EXIT_SUCCESS);


	/* Father */
	} else {

		initServer(shm_id, pipes, cars, cars_cnt, pids);

		/* Debug */
		/* End debug */

		signal(SIGALRM, sighandler);
		
		for (race = 0; race < 7; race++) {

			resetWins();
			resetTimers();

			if(race / 3 == 0 || race / 3 == 1){
				sem_wait(sem);
				resetPilotesByStatus(out);
				sem_post(sem);
			}else{
				sem_wait(sem);
				resetPilotesByStatus(eliminated);
				sem_post(sem);
			}

			/* Wait one second to sync everyone*/
			critical(resetPilotes);
			sleep(1);
			writeStatus("Running", race/3+1, race%3+1);

			rewriteStatus();

			/* Start a race */
			startRace(race);




			/* Enter main loop with specific code by race type */
			switch (race) {
				case 0:
				case 1:
				case 2:
					/* Loop until the alarm signal is fired */
					alarm(test_times[race] * simulation_divider);
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
						int data = flush_pipe(myPipe);
						if(data != 0){
							//CRITICAL SECTION
							sem_wait(sem);
							for (int car_counter = 0; car_counter < cars_cnt; car_counter++){
								pilote* current = &shm_addr[car_counter];
								scoreboard* scb = current->scores;
								if(current->has_changed == 1){
									current->has_changed = 0;
									updateCarStatus(current);
									if(current->status == out ){
										continue;
									}
									if(current->status == pitstop){
										scb->last_lap[race]->time_pit = current->time;
									}
									if(current->sector == 0 && current->lap_cnt != 0){
										scb->last_lap[race]->time_s3 = current->time;

										//Set best lap
										if(scb->bestlaps[race].best_lap > scb->last_lap[race]->time_s1 + scb->last_lap[race]->time_s2 + scb->last_lap[race]->time_s3){
											scb->bestlaps[race].best_lap = scb->last_lap[race]->time_s1 + scb->last_lap[race]->time_s2 + scb->last_lap[race]->time_s3;
											if(bestLap > scb->bestlaps[race].best_lap){
												bestLap = scb->bestlaps[race].best_lap;
												updateCarBest(current, scb->bestlaps[race], 4);
											}else{
												updateCarBest(current, scb->bestlaps[race], 0);
											}

										}
										//Set best s1
										if(scb->bestlaps[race].best_s1 > scb->last_lap[race]->time_s1){
											scb->bestlaps[race].best_s1 = scb->last_lap[race]->time_s1;
											if(bestS1 > scb->bestlaps[race].best_s1){
												bestS1 = scb->bestlaps[race].best_s1;
												updateCarBest(current, scb->bestlaps[race], 1);
											}else{
												updateCarBest(current, scb->bestlaps[race], 0);
											}
										}
										//Set best s2
										if(scb->bestlaps[race].best_s2 > scb->last_lap[race]->time_s2){
											scb->bestlaps[race].best_s2 = scb->last_lap[race]->time_s2;
											if(bestS2 > scb->bestlaps[race].best_s2){
												bestS2 = scb->bestlaps[race].best_s2;
												updateCarBest(current, scb->bestlaps[race], 2);
											}else{
												updateCarBest(current, scb->bestlaps[race], 0);
											}
										}
										//Set best s3
										if(scb->bestlaps[race].best_s3 > scb->last_lap[race]->time_s3){
											scb->bestlaps[race].best_s3 = scb->last_lap[race]->time_s3;
											if(bestS3 > scb->bestlaps[race].best_s3){
												bestS3 = scb->bestlaps[race].best_s3;
												updateCarBest(current, scb->bestlaps[race], 3);
											}else{
												updateCarBest(current, scb->bestlaps[race], 0);
											}
										}

										updateCarLap(current, scb, race);


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
											default:
												//NEVER HAPPEN
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
						scoreboard* sc = (&shm_addr[rank_idx])->scores;
						unsorted_ranking[rank_idx] = (rank_item){.car = (&shm_addr[rank_idx]), .bestlap = sc->bestlaps[race].best_lap};
					}
					qsort(unsorted_ranking, cars_cnt, sizeof(rank_item), &compare_rank_item);
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
					alarm(qualif_times[race % 3] * simulation_divider);
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
						int data = flush_pipe(myPipe);
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
									updateCarStatus(current);
									if(current->status == out || current->status == end){
										continue;
									}
									if(current->status == pitstop){
										scb->last_lap[race]->time_pit = current->time;
									}
									if(current->sector == 0 && current->lap_cnt != 0){
										scb->last_lap[race]->time_s3 = current->time;

										//Set best lap
										if(scb->bestlaps[race].best_lap > scb->last_lap[race]->time_s1 + scb->last_lap[race]->time_s2 + scb->last_lap[race]->time_s3){
											scb->bestlaps[race].best_lap = scb->last_lap[race]->time_s1 + scb->last_lap[race]->time_s2 + scb->last_lap[race]->time_s3;
											if(bestLap > scb->bestlaps[race].best_lap){
												bestLap = scb->bestlaps[race].best_lap;
												updateCarBest(current, scb->bestlaps[race], 4);
											}else{
												updateCarBest(current, scb->bestlaps[race], 0);
											}

										}
										//Set best s1
										if(scb->bestlaps[race].best_s1 > scb->last_lap[race]->time_s1){
											scb->bestlaps[race].best_s1 = scb->last_lap[race]->time_s1;
											if(bestS1 > scb->bestlaps[race].best_s1){
												bestS1 = scb->bestlaps[race].best_s1;
												updateCarBest(current, scb->bestlaps[race], 1);
											}else{
												updateCarBest(current, scb->bestlaps[race], 0);
											}
										}
										//Set best s2
										if(scb->bestlaps[race].best_s2 > scb->last_lap[race]->time_s2){
											scb->bestlaps[race].best_s2 = scb->last_lap[race]->time_s2;
											if(bestS2 > scb->bestlaps[race].best_s2){
												bestS2 = scb->bestlaps[race].best_s2;
												updateCarBest(current, scb->bestlaps[race], 2);
											}else{
												updateCarBest(current, scb->bestlaps[race], 0);
											}
										}
										//Set best s3
										if(scb->bestlaps[race].best_s3 > scb->last_lap[race]->time_s3){
											scb->bestlaps[race].best_s3 = scb->last_lap[race]->time_s3;
											if(bestS3 > scb->bestlaps[race].best_s3){
												bestS3 = scb->bestlaps[race].best_s3;
												updateCarBest(current, scb->bestlaps[race], 3);
											}else{
												updateCarBest(current, scb->bestlaps[race], 0);
											}
										}

										updateCarLap(current, scb, race);

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
											default:
												//NEVER HAPPEN
												break;
										}
									}


								}
							}

							//REDO POSITION

							sem_post(sem);
						}


					}

					//Calculate results
					unsorted_ranking = calloc(cars_cnt, sizeof(rank_item));
					for(int rank_idx = 0; rank_idx < cars_cnt; rank_idx++){
						scoreboard* sc = (&shm_addr[rank_idx])->scores;
						unsorted_ranking[rank_idx] = (rank_item){.car = (&shm_addr[rank_idx]), .bestlap = sc->bestlaps[race].best_lap};
					}
					qsort(unsorted_ranking, cars_cnt, sizeof(rank_item), &compare_rank_item);

					//Eliminate last cars
					char debugbuffer[512];
					sprintf(debugbuffer, "Loop from %d to %d, decrease, %d get, %d acc", cars_cnt - carToGetAccumulator - 1, cars_cnt - (carToGetAccumulator + carToGet), carToGet, carToGetAccumulator);
					writeHistory(debugbuffer);
					int cmp = (int)(cars_cnt - (carToGetAccumulator + carToGet));

					for(r = (int) (cars_cnt - carToGetAccumulator - 1); r >= cmp; r--){
						if(r>=0){
							pilote* p = unsorted_ranking[r].car;
							p->status = eliminated;
							char buffer[512];
							sprintf(buffer, "Car n°%d has been eliminated", p->car_id);
							writeHistory(buffer);
							updateCarStatus(p);
							startFinalRace[r] = p;
						}
					}
					getch();

					carToGetAccumulator += carToGet;
					showResults(unsorted_ranking);


					getch();


					break;
				case 6:


					//Reorder by startFinalRace
					for(r = 0; r < cars_cnt; r++){
						startFinalRace[r]->position = r;
						updateCarStatus(startFinalRace[r]);
						writePosition(startFinalRace[r]);
					}
					while(finished_race_count != cars_cnt){

						int data = flush_pipe(myPipe);
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
									updateCarStatus(current);
									if(current->status == out){
										current->position = (int)cars_cnt - 1 - (elimination_counter++);
										finished_race_count++;
										continue;
									}
									if(current->status == pitstop){
										scb->last_lap[race]->time_pit = current->time;
									}
									if(current->sector == 0 && current->lap_cnt != 0){
										scb->last_lap[race]->time_s3 = current->time;

										//Set best lap
										if(scb->bestlaps[race].best_lap > scb->last_lap[race]->time_s1 + scb->last_lap[race]->time_s2 + scb->last_lap[race]->time_s3 + scb->last_lap[race]->time_pit){
											scb->bestlaps[race].best_lap = scb->last_lap[race]->time_s1 + scb->last_lap[race]->time_s2 + scb->last_lap[race]->time_s3 + scb->last_lap[race]->time_pit;
											if(bestLap > scb->bestlaps[race].best_lap){
												bestLap = scb->bestlaps[race].best_lap;
												updateCarBest(current, scb->bestlaps[race], 4);
											}else{
												updateCarBest(current, scb->bestlaps[race], 0);
											}

										}
										//Set best s1
										if(scb->bestlaps[race].best_s1 > scb->last_lap[race]->time_s1){
											scb->bestlaps[race].best_s1 = scb->last_lap[race]->time_s1;
											if(bestS1 > scb->bestlaps[race].best_s1){
												bestS1 = scb->bestlaps[race].best_s1;
												updateCarBest(current, scb->bestlaps[race], 1);
											}else{
												updateCarBest(current, scb->bestlaps[race], 0);
											}
										}
										//Set best s2
										if(scb->bestlaps[race].best_s2 > scb->last_lap[race]->time_s2){
											scb->bestlaps[race].best_s2 = scb->last_lap[race]->time_s2;
											if(bestS2 > scb->bestlaps[race].best_s2){
												bestS2 = scb->bestlaps[race].best_s2;
												updateCarBest(current, scb->bestlaps[race], 2);
											}else{
												updateCarBest(current, scb->bestlaps[race], 0);
											}
										}
										//Set best s3
										if(scb->bestlaps[race].best_s3 > scb->last_lap[race]->time_s3){
											scb->bestlaps[race].best_s3 = scb->last_lap[race]->time_s3;
											if(bestS3 > scb->bestlaps[race].best_s3){
												bestS3 = scb->bestlaps[race].best_s3;
												updateCarBest(current, scb->bestlaps[race], 3);
											}else{
												updateCarBest(current, scb->bestlaps[race], 0);
											}
										}

										updateCarLap(current, scb, race);

										lap* temp = (lap*)calloc(1, sizeof(lap)); //Trick to set all times to 0
										temp->nextlap = NULL;
										scb->last_lap[race]->nextlap = temp;
										scb->last_lap[race] = temp;

										if(race_laps == current->lap_cnt) {
											kill(pids[car_counter], SIG_RACE_STOP);
											finished_race_count++;
										}


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
											default:
												//NEVER HAPPEN
												break;
										}
									}


								}
							}
							sem_post(sem);
						}

						final_race_do_sort();
						final_race_do_update_pos();
					}

					for (i = 0; i < cars_cnt; i++) (&shm_addr[i])->status = end;
					//Calculate results

					mergesort(shm_addr, cars_cnt, sizeof(pilote), &compare_pilote_position);

					getch();

					showFinalResults(shm_addr);

					getch();

					break;
				default:
					//NEVER HAPPEN
					break;
			}

			/* Stop the current race */
			for (i = 0; i < cars_cnt; i++) kill(pids[i], SIG_RACE_STOP);
		}

		/* Detach the shared memory */
		try_sys_call_int(shmdt(shm_addr), "Shmdet failure");

		for (i = 0; i < cars_cnt; i++) kill(pids[i], SIGKILL);
		for (i = 0; i < cars_cnt; i++) wait(NULL);

		echo();
		nocbreak();
		endwin();


		exit(EXIT_SUCCESS);
	}

}
