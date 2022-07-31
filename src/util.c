#include <time.h>

#include "util.h"

void ssleep(int seconds)
{
	struct timespec ts = {seconds, 0};
	nanosleep(&ts, NULL);
}
