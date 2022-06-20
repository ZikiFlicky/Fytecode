#ifndef FY_INTERRUPTS_H
#define FY_INTERRUPTS_H

#include "vm.h"

#include <inttypes.h>

typedef void (*Fy_InterruptRunFunc)(Fy_VM *vm);

extern Fy_InterruptRunFunc Fy_interruptFuncs[];

Fy_InterruptRunFunc Fy_findInterruptFuncByOpcode(uint8_t opcode);

#endif /* FY_INTERRUPTS_H */
