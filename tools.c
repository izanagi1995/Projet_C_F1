#include <unistd.h>
#include "defs.h"
#include "tools.h"

f1Car readMem(void* mem, int index){
    f1Car* memCar = (f1Car *)mem;
    while(memCar[index].writingLock == 1){
        usleep(50);
    }
    return memCar[index];
}

void writeMem(void* mem, int index, f1Car car){
   f1Car* memCar = ((f1Car *)mem);
   memCar[index].writingLock = 1;
   car.writingLock = 1;
   memCar[index] = car;
   memCar[index].writingLock = 0;
   mem = (void *)memCar;
}
