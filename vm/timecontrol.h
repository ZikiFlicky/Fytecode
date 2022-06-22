#ifndef FY_TIMECONTROL_H
#define FY_TIMECONTROL_H

#include <inttypes.h>

typedef struct Fy_Time Fy_Time;

struct Fy_Time {
    uint16_t seconds;
    uint16_t milliseconds;
};

void Fy_Time_Init(Fy_Time *out);
void Fy_Time_getTimeSince(Fy_Time *start, Fy_Time *out);

#endif /* FY_TIMECONTROL_H */
