#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
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

void handleCarEvent(){
    
}

void startCar(){
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

