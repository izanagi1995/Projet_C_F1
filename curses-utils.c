#include <ncurses.h>
#include "defs.h"

WINDOW* status_win;
WINDOW* cars_win;
WINDOW* cars_res;
WINDOW* cars_best;
WINDOW* best_win;
WINDOW* best_win_content;
int num_cars;

void initScreen(int cars){
    status_win = newwin(3, COLS-1, 0, 0);
    cars_win = newwin(cars+4, 11, 3, 0);
    cars_res = newwin(cars+4, COLS-41, 3, 11);
    cars_best = newwin(cars+4, 29, 3, COLS-30);
    best_win = newwin(LINES-cars-7, COLS-1, cars+7, 0);
    best_win_content = newwin(LINES-cars-9, COLS-3, cars+8, 1);
    scrollok(best_win_content, 1);
    start_color();
    init_pair(1, COLOR_BLACK, COLOR_WHITE);
    bkgd(COLOR_PAIR(1));
    wbkgd(status_win, COLOR_PAIR(1));
    wbkgd(cars_win, COLOR_PAIR(1));
    wbkgd(cars_res, COLOR_PAIR(1));
    wbkgd(cars_best, COLOR_PAIR(1));
    wbkgd(best_win, COLOR_PAIR(1));
    wbkgd(best_win_content, COLOR_PAIR(1));
    refresh();
    init_pair(14, COLOR_RED, COLOR_WHITE);
    init_pair(15, COLOR_GREEN, COLOR_WHITE);
    num_cars = cars;
}

void testView(){
    box(status_win, 0 , 0);
    box(cars_win, 0 , 0);
    box(cars_res, 0, 0);
    box(cars_best, 0, 0);
    box(best_win, 0, 0);
    wrefresh(status_win);
    wrefresh(cars_win);
    wrefresh(cars_res);
    wrefresh(cars_best);
    wrefresh(best_win);
    refresh();
}

void clearWin(WINDOW* win, int border){
    wclear(win);
    if(border){
        box(win, 0, 0);
    }
    wrefresh(win);
}

void resetWins(){
    clearWin(status_win, 1);
    clearWin(cars_win, 1);
    clearWin(cars_res, 1);
    clearWin(cars_best, 1);
    clearWin(best_win, 1);
    clearWin(best_win_content, 0);
}

void clearScreen(){
    clearWin(status_win, 0);
    clearWin(cars_win, 0);
    clearWin(cars_res, 0);
    clearWin(cars_best, 0);
    clearWin(best_win, 0);
    clearWin(best_win_content, 0);
}

void showResults(rank_item* ranks){
    clearScreen();
    box(stdscr, 0, 0);
    for(int i = 0; i < num_cars; i++){
        attron(A_BOLD);
        mvprintw(i+1, 1, "%2d : ", i);
        attroff(A_BOLD);
        mvprintw(i+1, 7, "Car n°%2d with best lap : %5.2f", ranks[i].car_id, ranks[i].bestlap);
    }
    refresh();
}

void writeStatus(char* status, int typeRace, int numRace){
    wclear(status_win);
    box(status_win, 0 , 0);
    mvwprintw(status_win, 1, 1, "Status : %s", status);
    mvwprintw(status_win, 1, (COLS/2)-4, "Projet F1");
    mvwprintw(status_win, 1, COLS-12, "Race: %d:%d", typeRace, numRace);
    wrefresh(status_win);
}

void updateCarStatus(int idx, int car_id, status stat){
    if(stat == driving){
        mvwprintw(cars_win, idx+1, 1, "%d (D)", car_id);
    }else if(stat == pitstop){
        mvwprintw(cars_win, idx+1, 1, "%d (P)", car_id);
    }else if(stat == out){
        mvwprintw(cars_win, idx+1, 1, "%d (O)", car_id);
        mvwchgat(cars_res, idx+1, 1, -1, A_BOLD, 14, NULL);
        wrefresh(cars_res);
    }else if(stat == eliminated){
        mvwprintw(cars_win, idx+1, 1, "%d (E)", car_id);
    }else{
        mvwprintw(cars_win, idx+1, 1, "%d (?)", car_id);
    }
    wrefresh(cars_win);
}

void updateCarLap(int idx, float lap, int lap_cnt){
    mvwprintw(cars_res, idx+1, (7*(lap_cnt-1))+2, "%.2f", lap);
    wrefresh(cars_res);
}


void writeHistory(char* text){
    wprintw(best_win_content, "%s\n", text);
    wrefresh(best_win_content);
}

void setBest(int idx, int bestIdx, bestlap lap){
    char* descriptor;
    float time;
    if(bestIdx == 1){
        descriptor = "S1";
        time = lap.best_s1;
    }
    if(bestIdx == 2){
        descriptor = "S2";
        time = lap.best_s2;
    }
    if(bestIdx == 3){
        descriptor = "S3";
        time = lap.best_s3;
    }
    if(bestIdx == 4){
        descriptor = "turn";
        time = lap.best_lap;
    }
    char buffer[512];
    sprintf(buffer, "Car %d has made a better %s : %5.2f", idx, descriptor, time);
    writeHistory(buffer);
    wrefresh(best_win_content);
}

void updateCarBest(int idx, int car_num, bestlap bestLap, int bestGlobalIdx){
    mvwprintw(cars_best, idx+1, 2, "%5.2f %5.2f %5.2f : ", bestLap.best_s1, bestLap.best_s2, bestLap.best_s3);
    mvwprintw(cars_best, idx+1, 22, "%5.2f ", bestLap.best_lap);
    if(bestGlobalIdx != 0){
        setBest(car_num, bestGlobalIdx, bestLap);
        mvprintw(LINES-1, 1, "%d : %d", idx, bestGlobalIdx);
        refresh();
    }
    wrefresh(cars_best);
}


