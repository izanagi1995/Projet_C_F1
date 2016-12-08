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
			while (!flag_race_stop) {

				//CRITICAL SECTION
				sem_wait(sem);
				// Variables:
				// pilote* myself: pointer to the section of shared memory usable by this pilote
				// int pipe: fd with write access to the server
				// ======= TODO ========
				//


				int intTime = rand()%10 + 5;
				float time = (float)(intTime);


				doSector(myself, time, pipe);

                if((rand() % 100) < 5){
					myself->status = out;
                    myself->has_changed = 1;
				}
				sem_post(sem);
                if(myself->status == out){
                    char status[] = "out";
                    write(pipe, status, sizeof(status));
                    break;
                }

				sleep(intTime);

			}
		}

		//printf("Pilote %d left the tournament! \n", myself->car_id);
		exit(EXIT_SUCCESS);


	/* Father */
	} else {
        initscr();
        cbreak();
        noecho();
        refresh();
        /*WINDOW* st_win = newwin(2, COLS-1, 0, 0);
        for(int widthIt = 0; widthIt < COLS; widthIt++){
            mvprintw(1, widthIt,"-");
        }
        for(int c = 0; c < cars_cnt; c++){
            mvprintw(c+2, 0, "%d", cars[c]);
        }
        wrefresh(st_win);*/
        initScreen();
        testView();
        writeStatus("Starting", 0, 0);
        refresh();



        int pipe = pipes[0];
        close(pipes[1]);

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
        free(cars);
		//CRITICAL SECTION
		sem_wait(sem);
		/* Attach the shared memory, fill the structure and free the array of cars */
		pilote* shm_addr = (pilote*) try_sys_call_ptr(shmat(shm_id, NULL, 0), "Shmat failure");
		for (i = 0; i < cars_cnt; i++) {
			shm_addr[i].car_id = cars[i];
			shm_addr[i].status = driving;
		}
		sem_post(sem);


		/* Debug */
		/* End debug */

		signal(SIGALRM, sighandler);
		
		for (race = 0; race < 7; race++) {

			/* Wait one second to sync everyone*/

            for (int car_counter = 0; car_counter < cars_cnt; car_counter++) {
                pilote *current = &shm_addr[car_counter];
                current->lap_cnt = 0;
                current->sector = 0;
                if(current->status == out){
                    current->status = driving;
                }
                updateCarStatus(car_counter, current->car_id, driving);
            }

			sleep(1);

            writeStatus("Running", race/3+1, race%3+1);

			/* Start a race */

            //Restore crashed car for test and qualification
            if(race / 3 == 0 || race / 3 == 1){
                for (int car_counter = 0; car_counter < cars_cnt; car_counter++) {
                    pilote *current = &shm_addr[car_counter];
                    current->status = driving;
                }
            }
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
                                    if(current->status == out ){
                                        updateCarStatus(car_counter, current->car_id, out);
                                        continue;
                                    }
                                    if(current->sector == 0){
                                        scb->last_lap[race]->time_s3 = current->time;

                                        //Set best lap
                                        if(scb->bestlaps[race].best_lap > scb->last_lap[race]->time_s1 + scb->last_lap[race]->time_s2 + scb->last_lap[race]->time_s3){
                                            scb->bestlaps[race].best_lap = scb->last_lap[race]->time_s1 + scb->last_lap[race]->time_s2 + scb->last_lap[race]->time_s3;
                                        }
                                        //Set best s1
                                        if(scb->bestlaps[race].best_s1 > scb->last_lap[race]->time_s1){
                                            scb->bestlaps[race].best_s1 = scb->last_lap[race]->time_s1;
                                        }
                                        //Set best s2
                                        if(scb->bestlaps[race].best_s2 > scb->last_lap[race]->time_s2){
                                            scb->bestlaps[race].best_s2 = scb->last_lap[race]->time_s2;
                                        }
                                        //Set best s3
                                        if(scb->bestlaps[race].best_s3 > scb->last_lap[race]->time_s3){
                                            scb->bestlaps[race].best_s3 = scb->last_lap[race]->time_s3;
                                        }


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

                            //UPDATE GUI

                        }


					}

                    //Calculate results
                    rank_item* unsorted_ranking = calloc(cars_cnt, sizeof(rank_item));
                    for(int rank_idx = 0; rank_idx < cars_cnt; rank_idx++){
                        scoreboard sc = scoreboards[rank_idx];
                        unsorted_ranking[rank_idx] = (rank_item){.car_id = sc.car_id, .bestlap = sc.bestlaps[race].best_lap};
                    }
                    mergesort(unsorted_ranking, cars_cnt, sizeof(rank_item), &compare_rank_item);
                    for(int rank_idx = 0; rank_idx < cars_cnt; rank_idx++){
                        rank_item ri = unsorted_ranking[rank_idx];
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
			for (i = 0; i < cars_cnt; i++) kill(pids[i], SIG_RACE_STOP);
		}

		/* Detache the shared memory */
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
