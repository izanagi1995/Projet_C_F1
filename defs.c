#define SIG_RACE_START (SIGUSR1)
#define SIG_RACE_STOP (SIGUSR2)

unsigned int test_times[] = {90, 90, 60};
unsigned int qualif_times[] = {18, 15, 12};
int race_laps = 50;

typedef enum status status;
enum status {
    driving,
    pitstop,
    out,
    end
};

typedef struct lap lap;
struct lap {
	lap* prevlap;
	lap* nextlap;
	float time_s1;
	float time_s2;
	float time_s3;
};

typedef struct races races;
struct races {
	int car_id;
	lap* races[7];
	struct bestlap {
		float time_s1;
		float time_s2;
		float time_s3;
		float time_lap;
	} bestlap;
};

typedef struct pilote pilote;
struct pilote {
	int car_id;
    int lap;
	int sector;
	float time;
	status status;
    races* races;
};
