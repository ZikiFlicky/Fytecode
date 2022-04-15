#include "fy.h"

void Fy_VM_Init(uint8_t *generated, uint16_t length, Fy_VM *out) {
    out->mem_space_bottom = malloc((1 << 16) * sizeof(uint8_t));
    memcpy(out->mem_space_bottom, generated, length * sizeof(uint8_t));
    out->code_size = length;
    out->reg_ax = 0;
    out->reg_bx = 0;
    out->reg_ip = 0;
    out->running = true;
}

uint16_t *Fy_VM_getReg16Ptr(Fy_VM *vm, uint8_t reg) {
    switch (reg) {
    case 0: // Ax
        return &vm->reg_ax;
    case 1:
        return &vm->reg_bx;
    default:
        return NULL;
    }
}

// uint16_t Fy_VM_getReg16(Fy_VM *vm, uint8_t reg) {
//     uint16_t *ptr = Fy_VM_getReg16Ptr(vm, reg);
//     if (!ptr) {
//         // TODO: Error here
//         // Fy_VM_runtimeError(vm, Fy_RuntimeError_RegNotFound);
//         FY_UNREACHABLE();
//     }
//     return *ptr;
// }

// void Fy_VM_setReg16(Fy_VM *vm, uint8_t reg, uint16_t value) {
//     uint16_t *ptr = Fy_VM_getReg16Ptr(vm, reg);
//     if (!ptr) {

//     }
// }

static void Fy_VM_runInstruction(Fy_VM *vm) {
    uint8_t opcode = vm->mem_space_bottom[vm->reg_ip];
    for (size_t i = 0; i < sizeof(Fy_instructionTypes) / sizeof(Fy_InstructionType*); ++i) {
        Fy_InstructionType *type = Fy_instructionTypes[i];
        if (opcode == type->opcode) {
            assert(type->run_func);
            type->run_func(vm);
            vm->reg_ip += 1 + type->additional_size;
            break;
        }
    }
}

void Fy_VM_runAll(Fy_VM *vm) {
    while (vm->running) {
        Fy_VM_runInstruction(vm);
    }
}
