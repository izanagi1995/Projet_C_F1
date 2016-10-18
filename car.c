#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/select.h>
#include "defs.h"
#include "tools.h"
#include "car.h"

f1Car _car;
void* _mem;
int _slot;
int _rPipe;
int _wPipe;
int carStatus;
int raceMode;
int remaining;

f1CarEvent lastEvent;
int carMustDie = 0;
fd_set fd;

void initCar(f1Car car, void* mem, int memSlot, int read, int write){
    _car = car;
    _mem = mem;
    _slot = memSlot;
    _rPipe = read;
    _wPipe = write;
    FD_ZERO(&fd);
    FD_SET(_rPipe, &fd);
}

void doSector(){
        
}

void run(){
    while(remaining > 0){
        doSector();
    }
}



void handleCarEvent(){
    switch(lastEvent.eventCode){
        case 11: //remaining = carCode
            if(carStatus == WAITING){
                carStatus = GOT_MODE;
                remaining = lastEvent.carCode;
            }else{
                printf("Warning : Got timeEvent on bad status (%d)\r\n", carStatus);
            }
            break;
        case 12: //raceMode = carCode
            if(carStatus == GOT_MODE){
                raceMode = lastEvent.carCode;
                run();
            }else{
                printf("Warning : Got modeEvent on bad status (%d)\r\n", carStatus);
            }  
        break;
        default:
            printf("Warning : %d event is not defined\r\n", lastEvent.eventCode);
    }
}

void startCar(int rand){
    srand(rand);
    carStatus = WAITING;
    printf("%d\r\n", readMem(_mem, _slot).carNumber);
    struct timeval tv;
    tv.tv_sec = 5;
    tv.tv_usec = 0;
    while(!carMustDie){
        if (select(_rPipe+1, &fd, NULL, NULL, &tv)==1){
            read(_rPipe, &lastEvent, sizeof(f1CarEvent));
            handleCarEvent();
        }else{
            carMustDie = 1;
        }
    }
}

