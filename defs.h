#ifndef DEFS_H_INCLUDED
#define DEFS_H_INCLUDED


#define MEM_KEY 20135
#define N_CARS 22

//CORE STATUS
#define WAITING       0
#define READY         1
#define ESSAI         2
#define QUALIFICATION 3
#define COURSE        4

//CAR STATUS
//#define WAITING     0
#define GOT_MODE      1 //Got mode but not time or laps
#define RACE          2

/*
 * Specs code
 * ==========
 * FROM CARS
 * ---------
 * 0 : Program ready. carCode must be 21 (the last one)
 * 1-3 : The car just left a sector
 * 4 : Car just entered the pitstop
 * 5 : Car just left the pitstop
 * 9 : Car abandoned the race
 * FROM MANAGER
 * ------------
 * 11 : Specify in carCode the next race time or laps
 * 12 : Specify mode in eventCode (0: time, 1: laps)
 */
typedef struct{
    unsigned int carCode;
    unsigned int eventCode;
}f1CarEvent;

typedef struct{
    float s1;
    float s2;
    float s3;        
    float pit;
}f1Times;

typedef struct{
    unsigned char    writingLock; 
    unsigned int     carNumber; //Will be given by the main process...
    unsigned int     racingAbandoned;
    unsigned int     atPitstop;
    unsigned int     currentSector;
    unsigned int     lapsCount; 
             f1Times timesLap; 
}f1Car;


#endif
