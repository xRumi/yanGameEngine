#ifdef PLATFORM_WINDOWS
#include "windows.h"
#include "logger.h"

double platformGetTime() {
    FILETIME ft;
    GetSystemTimePreciseAsFileTime(&ft);
    uint64_t time_100ns = ((uint64_t)ft.dwHighDateTime << 32) | ft.dwLowDateTime;
    return time_100ns / (double)1e7;
}
void platformSleep(double time) {
    HANDLE hTimer = CreateWaitableTimer(NULL, TRUE, NULL);
    if (hTimer == NULL) return;

    LARGE_INTEGER liDueTime;
    liDueTime.QuadPart = -(LONGLONG)(time * 10000000.0);

    if (SetWaitableTimer(hTimer, &liDueTime, 0, NULL, NULL, FALSE)) {
        WaitForSingleObject(hTimer, INFINITE);
    }

    CloseHandle(hTimer);
}
#endif