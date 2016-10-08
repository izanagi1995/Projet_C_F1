#define MEM_KEY 20135

// Core status
#define S_INIT				0
#define S_READY				1
#define S_TEST_RACES		2
#define S_QUALIFYING_RACES	3
#define S_RACE				4

// Times for test races, qualif races and the race
#define T_T1	90
#define T_T2	90
#define T_T3	60
#define T_Q1	18
#define T_Q2	15
#define T_Q3	12
#define T_RACE	0 // Need value

/* time by sectors for a lap */
typedef struct sector_times {
	sector_times* previous_lap,
	sector_times* next_lap,
	float s1,
	float s2,
	float s3
};

/* all info for a pilote */
typedef struct f1_pilote {
	unsigned int id, // internaly used id [0; #PILOTES]
	unsigned int pilote_id, // printable id (pilote number)
	void* current_lap, // point a sector_times
	char current_sector // sector [1], sector [2], sector [3]
	char status, // [D]riving, [P]itstop, [O]ut
};

/* store the time for each pilote at the end of a race */
typedef struct stored_time {
	unsigned int pilote_id,
	void* first_lap
};