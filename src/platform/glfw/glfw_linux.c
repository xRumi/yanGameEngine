#ifdef PLATFORM_LINUX
#include <time.h>
double platformGetTime() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}
void platformSleep(double time) {
    struct timespec ts;
    ts.tv_sec = (time_t)time;
    ts.tv_nsec = (time - ts.tv_sec) * 1e9;
    nanosleep(&ts, NULL);
}
#endif