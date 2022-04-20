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
    /* Address in which code starts */
    uint16_t code_offset;
    /* Size of code */
    uint16_t code_size;
    /* Address in which stack starts */
    uint16_t stack_offset;
    /* Size of stack (in bytes) */
    uint16_t stack_size;
    /* Registers */
    uint8_t reg_ax[2];
    uint8_t reg_bx[2];
    uint8_t reg_cx[2];
    uint8_t reg_dx[2];
    uint16_t reg_ip;
    uint16_t reg_sp;
    /* Is running? */
    bool running;
    uint8_t flags;
};

void Fy_VM_Init(uint8_t *generated, uint16_t length, uint16_t stack_size, Fy_VM *out);
uint16_t Fy_VM_getReg16(Fy_VM *vm, uint8_t reg);
void Fy_VM_setReg16(Fy_VM *vm, uint8_t reg, uint16_t value);
uint8_t Fy_VM_getReg8(Fy_VM *vm, uint8_t reg);
void Fy_VM_setReg8(Fy_VM *vm, uint8_t reg, uint8_t value);
void Fy_VM_runtimeError(Fy_VM *vm, Fy_RuntimeError err, char *additional, ...);
void Fy_VM_runAll(Fy_VM *vm);
void Fy_VM_setResult8InFlags(Fy_VM *vm, int8_t res);
void Fy_VM_setResult16InFlags(Fy_VM *vm, int16_t res);
void Fy_VM_setIpToRelAddress(Fy_VM *vm, uint16_t address);
void Fy_VM_pushToStack(Fy_VM *vm, uint16_t value);
uint16_t Fy_VM_popFromStack(Fy_VM *vm);

#endif /* FY_VM_H */
