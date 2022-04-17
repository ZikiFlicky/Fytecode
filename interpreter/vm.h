#ifndef FY_VM_H
#define FY_VM_H

#include <inttypes.h>
#include <stdbool.h>

#define FY_FLAGS_ZERO (1 << 0)
#define FY_FLAGS_SIGN (1 << 1)

typedef struct Fy_VM Fy_VM;

typedef enum Fy_RuntimeError {
    Fy_RuntimeError_RegNotFound = 1,
    Fy_RuntimeError_InvalidOpcode
} Fy_RuntimeError;

struct Fy_VM {
    /* Pointer to bottom of allocated memory space */
    uint8_t *mem_space_bottom;
    /* Size of code */
    uint16_t code_size;
    /* Registers */
    uint16_t reg_ax;
    uint16_t reg_bx;
    uint16_t reg_cx;
    uint16_t reg_dx;
    uint16_t reg_ip;
    /* Is running? */
    bool running;
    uint8_t flags;
};

void Fy_VM_Init(uint8_t *generated, uint16_t length, Fy_VM *out);
uint16_t *Fy_VM_getReg16Ptr(Fy_VM *vm, uint8_t reg);
void Fy_VM_runtimeError(Fy_VM *vm, Fy_RuntimeError err);
void Fy_VM_runtimeErrorAdditionalText(Fy_VM *vm, Fy_RuntimeError err, char *additional, ...);
void Fy_VM_runAll(Fy_VM *vm);
void Fy_VM_setResult16InFlags(Fy_VM *vm, int16_t res);

#endif /* FY_VM_H */
