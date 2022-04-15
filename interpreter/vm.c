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

static char *Fy_RuntimeError_toString(Fy_RuntimeError error) {
    switch (error) {
    case Fy_RuntimeError_RegNotFound:
        return "Register not found";
    case Fy_RuntimeError_InvalidOpcode:
        return "Invalid opcode";
    default:
        FY_UNREACHABLE();
    }
}

void Fy_VM_runtimeError(Fy_VM *vm, Fy_RuntimeError err) {
    (void)vm;
    printf("RuntimeError: %s", Fy_RuntimeError_toString(err));
    exit(1);
}

void Fy_VM_runtimeErrorAdditionalText(Fy_VM *vm, Fy_RuntimeError err, char *additional, ...) {
    va_list va;
    (void)vm;
    printf("RuntimeError: %s: ", Fy_RuntimeError_toString(err));
    va_start(va, additional);
    vprintf(additional, va);
    va_end(va);
    exit(1);
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

static void Fy_VM_runInstruction(Fy_VM *vm) {
    uint8_t opcode = vm->mem_space_bottom[vm->reg_ip];
    for (size_t i = 0; i < sizeof(Fy_instructionTypes) / sizeof(Fy_InstructionType*); ++i) {
        Fy_InstructionType *type = Fy_instructionTypes[i];
        if (opcode == type->opcode) {
            assert(type->run_func);
            type->run_func(vm);
            vm->reg_ip += 1 + type->additional_size;
            return;
        }
    }
    // If we got here, we didn't match any opcode and this is an invalid instruction
    Fy_VM_runtimeErrorAdditionalText(vm, Fy_RuntimeError_InvalidOpcode, "'%.2x'", opcode);
}

void Fy_VM_runAll(Fy_VM *vm) {
    while (vm->running) {
        Fy_VM_runInstruction(vm);
    }
}
