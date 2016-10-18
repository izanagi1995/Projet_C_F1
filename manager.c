#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/select.h>
#include "defs.h"
#include "tools.h"
#include "manager.h"

int readStream = -1;
int* writeStreams = NULL;
void* shrMem = NULL;
int mustDie = 0;
f1CarEvent lastEvent;
int status;
int carCounter = 0;

void initReadStream(int readDesc, int* writeDesc){
    readStream = readDesc;
    writeStreams = writeDesc;
}

void initMemory(void* mem){
    shrMem = mem;
}

f1CarEvent buildEvent(int eventCode, int carCode){
    f1CarEvent event;
    event.eventCode = eventCode;
    event.carCode = carCode;
    return event;
}

void answer(f1CarEvent event){
    write(writeStreams[lastEvent.carCode], (void*)&event, sizeof(f1CarEvent));
}


void broadcast(f1CarEvent event){
    for(int i = 0; i < N_CARS; i++){
        write(writeStreams[i], (void*)&event, sizeof(f1CarEvent));
    }
    printf("DONE BROADCAST\r\n");
}

void handleEventManager(){
    printf("%d\r\n", lastEvent.eventCode);
    switch(lastEvent.eventCode){
        case 0:
            printf("In event 0\r\n");
            if(lastEvent.carCode != 21 && status == WAITING){
                printf("Nous n'avons pas recu la réponse du bon candidat! (%d)\r\n", lastEvent.carCode);
                exit(1);
            }
            if(status == WAITING){
                status = READY;
                printf("Debut de P1\r\n");
                f1CarEvent time = buildEvent(11, 90);
                f1CarEvent type = buildEvent(12, 0);
                broadcast(time);
                broadcast(type);
            }else{
            }
            break;
        default:
            printf("WARNING DEFAULT\r\n");
            printf("carCode : %d, eventCode : %d\r\n", lastEvent.carCode, lastEvent.eventCode);
    }
}


void startManager(){
    status = WAITING;
    fd_set fd;
    FD_ZERO(&fd);
    FD_SET(readStream, &fd);
    struct timeval tv;
    tv.tv_sec = 5;
    tv.tv_usec = 0;
    while(!mustDie){
        if (select(readStream+1, &fd, NULL, NULL, &tv)==1){
            read(readStream, &lastEvent, sizeof(f1CarEvent));
            handleEventManager();
            //debug();
            printf("Handle done\r\n");
        }else{
        }
    }
}



