#ifndef PROJET_C_F1_SERVER_H
#define PROJET_C_F1_SERVER_H

extern pilote* shm_addr;
extern pilote* sortArray;
extern int bestLap, bestS1, bestS2, bestS3;
extern scoreboard* scoreboards;
extern int cars_cnt;
extern int finished_race_count;
extern int myPipe;

void initServer(int shm_id, int pipes[2], int* _cars, int car_cnt, int* pids_list);
void initPilotes();
void prepareScoreboards();
void prepareCLI();
void destroyCLI();
void critical(void (*call)(void));

void resetPilotesByStatus(status s);
void resetPilotes();

void resetTimers();
void rewriteStatus();

void startRace(int race_id);
void final_race_do_sort();
void final_race_do_update_pos();

#endif //PROJET_C_F1_SERVER_H
