#define RIG 1
#define RIG_OK 10
#define RIG_NOK 12

#define LOG 2
#define LOG_OK 20
#define LOG_NOK 21

#define QUR 3
#define QUR_OK 30
#define QUR_NOK 31

#define HIT 4
#define HIT_OK 40
#define HIT_NOK 41


typedef struct biao
{
	int type;
	char buf1[128];
	char buf2[256];
	char buf3[128];
	char *buf4[256];
}msg_t;

