#define MEM_KEY 20135
#define N_CARS 22
typedef struct{
    float s1;
    float s2;
    float s3;        
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
