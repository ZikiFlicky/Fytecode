#ifndef FY_EXITSIGNAL_H
#define FY_EXITSIGNAL_H

#include <stdbool.h>

extern bool Fy_hadExitSignal;

bool Fy_SetSignalHandlers(void);

#endif /* FY_EXITSIGNAL_H */
