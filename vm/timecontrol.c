#include "fy.h"

void Fy_Time_Init(Fy_Time *out) {
    struct timespec tm;
    clock_gettime(CLOCK_REALTIME, &tm);
    out->seconds = tm.tv_sec;
    out->milliseconds = tm.tv_nsec / 1000000;
}

void Fy_Time_getTimeSince(Fy_Time *start, Fy_Time *out) {
    Fy_Time now;
    Fy_Time_Init(&now);

    out->seconds = now.seconds - start->seconds;
    if (start->milliseconds > now.milliseconds) {
        --out->seconds;
        out->milliseconds = 1000 + now.milliseconds - start->milliseconds;
    } else {
        out->milliseconds =  now.milliseconds - start->milliseconds;
    }
}
