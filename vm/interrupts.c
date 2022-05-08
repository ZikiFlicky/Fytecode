#include "fy.h"

static void Fy_interruptPutNumber_run(Fy_VM *vm);
static void Fy_interruptPutChar_run(Fy_VM *vm);
static void Fy_interruptPutString_run(Fy_VM *vm);

Fy_InterruptDef Fy_interruptPutNumber = {
    .opcode = 0,
    .func = Fy_interruptPutNumber_run
};

Fy_InterruptDef Fy_interruptPutChar = {
    .opcode = 1,
    .func = Fy_interruptPutChar_run
};

Fy_InterruptDef Fy_interruptPutString = {
    .opcode = 2,
    .func = Fy_interruptPutString_run
};

Fy_InterruptDef *Fy_interruptDefs[] = {
    &Fy_interruptPutNumber,
    &Fy_interruptPutChar,
    &Fy_interruptPutString,
};

static void Fy_interruptPutNumber_run(Fy_VM *vm) {
    uint16_t number;
    Fy_VM_getReg16(vm, Fy_Reg16_Ax, &number);
    printf("%d", number);
}

static void Fy_interruptPutChar_run(Fy_VM *vm) {
    uint8_t c;
    Fy_VM_getReg8(vm, Fy_Reg8_Al, &c);
    printf("%c", c);
}

static void Fy_interruptPutString_run(Fy_VM *vm) {
    uint16_t addr;
    uint16_t i = 0;
    uint8_t c;
    Fy_VM_getReg16(vm, Fy_Reg16_Ax, &addr);
    do {
        c = Fy_VM_getMem8(vm, addr + i);
        putchar(c);
        ++i;
    } while (c != 0); // Until reached NUL
}

void Fy_InterruptDef_run(Fy_InterruptDef *def, Fy_VM *vm) {
    assert(def->func);
    def->func(vm);
}

Fy_InterruptDef *Fy_findInterruptDefByOpcode(uint8_t opcode) {
    for (size_t i = 0; i < sizeof(Fy_interruptDefs) / sizeof(Fy_InterruptDef*); ++i) {
        Fy_InterruptDef *possible_interrupt = Fy_interruptDefs[i];
        if (opcode == possible_interrupt->opcode)
            return possible_interrupt;
    }
    return NULL;
}
