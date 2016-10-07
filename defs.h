#define MEM_KEY 20135
#define N_CARS 22

/*
 * Specs code
 * ----------
 *
 * 0 : Program ready. carCode must be 21 (the last one)
 * 1-3 : The car just left a sector
 * 4 : Car just entered the pitstop
 * 5 : Car just left the pitstop
 * 9 : Car abandoned the race
 */
typedef struct{
    unsigned int carCode;
    unsigned int eventCode;
}f1CarEvent;

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
