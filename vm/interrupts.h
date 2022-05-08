#ifndef FY_INTERRUPTS_H
#define FY_INTERRUPTS_H

#include "vm.h"

#include <inttypes.h>

typedef void (*Fy_InterruptRunFunc)(Fy_VM *vm);
typedef struct Fy_InterruptDef Fy_InterruptDef;

struct Fy_InterruptDef {
    uint8_t opcode;
    Fy_InterruptRunFunc func;
};

extern Fy_InterruptDef *Fy_interruptDefs[];

void Fy_InterruptDef_run(Fy_InterruptDef *def, Fy_VM *vm);
Fy_InterruptDef *Fy_findInterruptDefByOpcode(uint8_t opcode);

#endif /* FY_INTERRUPTS_H */
