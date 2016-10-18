#include <stdbool.h>

// Core status
#define S_INIT              0
#define S_TEST_RACES        1
#define S_QUALIFYING_RACES  2
#define S_RACE              3

// Times for test races, qualif races and the race
#define T_T1    90
#define T_T2    90
#define T_T3    60
#define T_Q1    18
#define T_Q2    15
#define T_Q3    12
#define T_RACE  0 // Need value

// Event code
#define E_SHM_KEY   1 

/* time by sectors for a lap */
typedef struct sector_times sector_times;
struct sector_times {
    sector_times *previous_lap;    // May be null if it's the first lap
    sector_times *next_lap;        // May be null if it's the last lap
    float s1;                      // Time in sector one
    float s2;                      // Time in sector two
    float s3;                      // Time in sector three
};

/* all info a child process can write */
typedef struct pilote_status pilote_status;
struct pilote_status {
    bool lock;                 // Prevent a process from reading when another is writing
    float current_time;        // Time in seconds, set before event fired
    int current_sector;        // Sector from 0 to 2, set before event fired
    char status;               // Value from: {[D]riving, [P]itstop, [O]ut} is set before an event is fired
};

/* all info private to the server */
typedef struct pilote pilote;
struct pilote {
    pid_t process_id;           // A process id to use with signals
    unsigned int pilote_id;     // A car id to use with event
    pilote_status *status;      // Point the shared memory used by both process
    sector_times *times[7];     // Record for sector times in the tests (3) + the qualifs (3) + the race (1). May be NULL pointer if the pilote didn't race 
};
