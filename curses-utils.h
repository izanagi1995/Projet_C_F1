#ifndef PROJET_C_F1_CURSES_UTIL_H
#define PROJET_C_F1_CURSES_UTIL_H

void initScreen(int cars);
void borderView();
void writeStatus(char* status, int typeRace, int numRace);
void updateCarStatus(pilote* p);
void updateCarLap(pilote* p, scoreboard* sc, int race);
void updateCarBest(pilote* p, bestlap bestLap, int bestGlobalIdx);
void resetWins();
void showResults(rank_item* ranks);
void showFinalResults(pilote* sorted);
void writeHistory(char* text);
void writePosition(pilote* p);

#endif //PROJET_C_F1_CURSES_UTIL_H