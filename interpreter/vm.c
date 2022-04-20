#include "fy.h"

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

void Fy_VM_Init(uint8_t *generated, uint16_t length, uint16_t stack_size, Fy_VM *out) {
    out->mem_space_bottom = malloc((1 << 16) * sizeof(uint8_t));
    memcpy(out->mem_space_bottom, generated, length * sizeof(uint8_t));
    out->code_offset = 0;
    out->code_size = length;
    out->stack_offset = out->code_offset + out->code_size + 0x100 + stack_size; // The 0x100 is for padding
    out->stack_size = stack_size; // In bytes
    out->reg_ax[0] = out->reg_ax[1] = 0;
    out->reg_bx[0] = out->reg_bx[1] = 0;
    out->reg_cx[0] = out->reg_cx[1] = 0;
    out->reg_dx[0] = out->reg_dx[1] = 0;
    out->reg_ip = 0;
    out->reg_sp = out->stack_offset;
    out->running = true;
    out->flags = 0;
}

// TODO: Merge the two functions
void Fy_VM_runtimeError(Fy_VM *vm, Fy_RuntimeError err) {
    (void)vm;
    printf("RuntimeError: %s\n", Fy_RuntimeError_toString(err));
    exit(1);
}

void Fy_VM_runtimeErrorAdditionalText(Fy_VM *vm, Fy_RuntimeError err, char *additional, ...) {
    va_list va;
    (void)vm;
    printf("RuntimeError: %s: ", Fy_RuntimeError_toString(err));
    va_start(va, additional);
    vprintf(additional, va);
    va_end(va);
    printf("\n");
    exit(1);
}

static uint8_t *Fy_VM_getReg16Ptr(Fy_VM *vm, uint8_t reg) {
    uint8_t *reg_ptr;

    switch (reg) {
    case Fy_Reg16_Ax:
        reg_ptr = vm->reg_ax;
        break;
    case Fy_Reg16_Bx:
        reg_ptr = vm->reg_bx;
        break;
    case Fy_Reg16_Cx:
        reg_ptr = vm->reg_cx;
        break;
    case Fy_Reg16_Dx:
        reg_ptr = vm->reg_dx;
        break;
    default:
        Fy_VM_runtimeErrorAdditionalText(vm, Fy_RuntimeError_RegNotFound, "%d", reg);
        FY_UNREACHABLE();
    }

    return reg_ptr;
}

uint16_t Fy_VM_getReg16(Fy_VM *vm, uint8_t reg) {
    uint8_t *reg_ptr = Fy_VM_getReg16Ptr(vm, reg);

    // Little-endian
    return reg_ptr[0] + (reg_ptr[1] << 8);
}

void Fy_VM_setReg16(Fy_VM *vm, uint8_t reg, uint16_t value) {
    uint8_t *reg_ptr = Fy_VM_getReg16Ptr(vm, reg);

    // Little endian
    reg_ptr[0] = value & 0xFF;
    reg_ptr[1] = value >> 8;
}

static uint8_t *Fy_VM_getReg8Ptr(Fy_VM *vm, uint8_t reg) {
    switch (reg) {
    case Fy_Reg8_Ah:
        return &vm->reg_ax[1];
    case Fy_Reg8_Al:
        return &vm->reg_ax[0];
    case Fy_Reg8_Bh:
        return &vm->reg_bx[1];
    case Fy_Reg8_Bl:
        return &vm->reg_bx[0];
    case Fy_Reg8_Ch:
        return &vm->reg_cx[1];
    case Fy_Reg8_Cl:
        return &vm->reg_cx[0];
    case Fy_Reg8_Dh:
        return &vm->reg_dx[1];
    case Fy_Reg8_Dl:
        return &vm->reg_dx[0];
    default:
        Fy_VM_runtimeErrorAdditionalText(vm, Fy_RuntimeError_RegNotFound, "%d", reg);
        FY_UNREACHABLE();
    }
}

uint8_t Fy_VM_getReg8(Fy_VM *vm, uint8_t reg) {
    uint8_t *reg_ptr = Fy_VM_getReg8Ptr(vm, reg);
    return *reg_ptr;
}

void Fy_VM_setReg8(Fy_VM *vm, uint8_t reg, uint8_t value) {
    uint8_t *reg_ptr = Fy_VM_getReg8Ptr(vm, reg);
    *reg_ptr = value;
}

static void Fy_VM_runInstruction(Fy_VM *vm) {
    uint8_t opcode = vm->mem_space_bottom[vm->reg_ip];
    for (size_t i = 0; i < sizeof(Fy_instructionTypes) / sizeof(Fy_InstructionType*); ++i) {
        Fy_InstructionType *type = Fy_instructionTypes[i];
        if (opcode == type->opcode) {
            assert(type->run_func);
            type->run_func(vm);
            // If we are told to advance after running the instruction (disabled in jumps)
            if (type->advance_after_run)
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

void Fy_VM_setResult16InFlags(Fy_VM *vm, int16_t res) {
    if (res == 0)
        vm->flags |= FY_FLAGS_ZERO; // Enable
    else
        vm->flags &= ~FY_FLAGS_ZERO; // Disable

    if (res < 0)
        vm->flags |= FY_FLAGS_SIGN; // Enable
    else
        vm->flags &= ~FY_FLAGS_SIGN; // Disable
}

/* Set the ip register to the given address relative to the code's start point in memory */
void Fy_VM_setIpToRelAddress(Fy_VM *vm, uint16_t address) {
    vm->reg_ip = vm->code_offset + address;
}

void Fy_VM_pushToStack(Fy_VM *vm, uint16_t value) {
    // FIXME: Do some stack overflow error
    if (vm->stack_offset - vm->reg_sp >= vm->stack_size)
        FY_UNREACHABLE();

    vm->reg_sp -= 2;

    *(uint16_t*)&vm->mem_space_bottom[vm->reg_sp] = value;
}

uint16_t Fy_VM_popFromStack(Fy_VM *vm) {
    uint16_t value;

    // FIXME: Do some stack underflow error
    if (vm->reg_sp >= vm->stack_offset)
        FY_UNREACHABLE();

    value = *(uint16_t*)&vm->mem_space_bottom[vm->reg_sp];

    vm->reg_sp += 2;

    return value;
}
