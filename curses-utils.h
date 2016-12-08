#ifndef PROJET_C_F1_CURSES_UTIL_H
#define PROJET_C_F1_CURSES_UTIL_H

void initScreen();
void testView();
void writeStatus(char* status, int typeRace, int numRace);
void updateCarStatus(int idx, int car_id, status stat);

#endif //PROJET_C_F1_CURSES_UTIL_H