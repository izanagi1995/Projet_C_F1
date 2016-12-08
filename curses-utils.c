#include <ncurses.h>
#include "defs.h"

WINDOW* status_win;
WINDOW* cars_win;
WINDOW* cars_res;
WINDOW* cars_best;

void initScreen(){
    status_win = newwin(3, COLS-1, 0, 0);
    cars_win = newwin(LINES-4, 11, 3, 0);
    cars_res = newwin(LINES-4, COLS-27, 3, 11);
    cars_best = newwin(LINES-4, 15, 3, COLS-16);
}

void testView(){
    box(status_win, 0 , 0);
    box(cars_win, 0 , 0);
    box(cars_res, 0, 0);
    box(cars_best, 0, 0);
    wrefresh(status_win);
    wrefresh(cars_win);
    wrefresh(cars_res);
    wrefresh(cars_best);
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
    mvwprintw(cars_win, idx+1, 1,     "      ", car_id);
    if(stat == driving){
        mvwprintw(cars_win, idx+1, 1, "%d", car_id);
    }else if(stat == out){
        mvwprintw(cars_win, idx+1, 1, "%d (O)", car_id);
    }else if(stat == end){
        mvwprintw(cars_win, idx+1, 1, "%d (E)", car_id);
    }
    wrefresh(cars_win);
}

void updateCarLap(int idx, float lap, int lap_cnt){

}