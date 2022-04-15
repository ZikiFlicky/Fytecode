#ifndef FY_VM_H
#define FY_VM_H

#include <inttypes.h>
#include <stdbool.h>

typedef struct Fy_VM Fy_VM;

struct Fy_VM {
    /* Pointer to bottom of allocated memory space */
    uint8_t *mem_space_bottom;
    /* Size of code */
    uint16_t code_size;
    uint16_t reg_ax;
    uint16_t reg_bx;
    uint16_t reg_ip;
    /* Is running? */
    bool running;
};

void Fy_VM_Init(uint8_t *generated, uint16_t length, Fy_VM *out);
uint16_t *Fy_VM_getReg16Ptr(Fy_VM *vm, uint8_t reg);
void Fy_VM_runAll(Fy_VM *vm);

#endif /* FY_VM_H */
