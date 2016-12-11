#ifndef PROJET_C_F1_CURSES_UTIL_H
#define PROJET_C_F1_CURSES_UTIL_H

void initScreen(int cars);
void testView();
void writeStatus(char* status, int typeRace, int numRace);
void updateCarStatus(int idx, int car_id, status stat);
void updateCarLap(int idx, float lap, int lap_cnt);
void updateCarBest(int idx, int car_num, bestlap bestLap, int bestGlobal);
void resetWins();
void showResults(rank_item* ranks);
void writeHistory(char* text);

#endif //PROJET_C_F1_CURSES_UTIL_H