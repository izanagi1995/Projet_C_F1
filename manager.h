/* This file was automatically generated.  Do not edit! */
void startManager(void);
void handleEventManager(void);
void broadcast(f1CarEvent event);
void answer(f1CarEvent event);
f1CarEvent buildEvent(int eventCode,int carCode);
void initMemory(void *mem);
void initReadStream(int readDesc,int *writeDesc);
extern int carCounter;
extern int status;
extern f1CarEvent lastEvent;
extern int mustDie;
extern void *shrMem;
extern int *writeStreams;
extern int readStream;
