#include "fy.h"

// FIXME: Temporary
static inline uint16_t Fy_MemoryGet16(uint8_t *mem) {
    return mem[0] + (mem[1] << 8);
}

/* Returns a new instruction initialized with the given type */
Fy_Instruction *Fy_Instruction_New(Fy_InstructionType *type, size_t size) {
    Fy_Instruction *instruction = malloc(size);
    instruction->type = type;
    return instruction;
}

/* Instruction type functions */

static void Fy_InstructionType_MovReg16Const_write(Fy_Generator *generator, Fy_Instruction_OpReg16Const *instruction) {
    Fy_Generator_addByte(generator, instruction->reg_id);
    Fy_Generator_addConst16(generator, instruction->value);
}

static void Fy_InstructionType_MovReg16Const_run(Fy_VM *vm) {
    uint8_t *base = &vm->mem_space_bottom[vm->reg_ip];
    uint8_t reg = base[1];
    uint16_t val = Fy_MemoryGet16(&base[2]);
    uint16_t *reg_ptr = Fy_VM_getReg16Ptr(vm, reg);
    // Register not found
    if (!reg_ptr)
        Fy_VM_runtimeErrorAdditionalText(vm, Fy_RuntimeError_RegNotFound, "%d", reg);
    *reg_ptr = val;
}

static void Fy_InstructionType_MovReg16Reg16_write(Fy_Generator *generator, Fy_Instruction_OpReg16Reg16 *instruction) {
    Fy_Generator_addByte(generator, instruction->reg_id);
    Fy_Generator_addByte(generator, instruction->reg2_id);
}

static void Fy_InstructionType_MovReg16Reg16_run(Fy_VM *vm) {
    uint8_t *base = &vm->mem_space_bottom[vm->reg_ip];
    uint8_t reg = base[1];
    uint8_t reg2 = base[2];
    uint16_t *reg_ptr = Fy_VM_getReg16Ptr(vm, reg);
    uint16_t *reg2_ptr = Fy_VM_getReg16Ptr(vm, reg2);
    // Register not found
    if (!reg_ptr || !reg2_ptr) {
        Fy_VM_runtimeError(vm, Fy_RuntimeError_RegNotFound);
    }
    *reg_ptr = *reg2_ptr;
}

static void Fy_InstructionType_Debug_run(Fy_VM *vm) {
    (void)vm;
    printf("DEBUG INFO:\n");
    printf("AX: %.2X %.2X\n", vm->reg_ax & ((1 << 8) - 1), vm->reg_ax >> 8);
    printf("BX: %.2X %.2X\n", vm->reg_bx & ((1 << 8) - 1), vm->reg_bx >> 8);
    printf("IP: %.4X\n", vm->reg_ip);
    printf("FLAG_ZERO: %d\n", vm->flags & FY_FLAGS_ZERO ? 1 : 0);
    printf("FLAG_SIGN: %d\n", vm->flags & FY_FLAGS_SIGN ? 1 : 0);
}

static void Fy_InstructionType_EndProgram_run(Fy_VM *vm) {
    vm->running = false;
}

static void Fy_InstructionType_Jmp_write(Fy_Generator *generator, Fy_Instruction_Jmp *instruction) {
    Fy_Generator_addConst16(generator, instruction->address);
}

static void Fy_InstructionType_Jmp_run(Fy_VM *vm) {
    // FIXME: This should actually be relative to the code and not to the whole memory
    vm->reg_ip = Fy_MemoryGet16(&vm->mem_space_bottom[vm->reg_ip + 1]);
}

static void Fy_InstructionType_AddReg16Const_write(Fy_Generator *generator, Fy_Instruction_OpReg16Const *instruction) {
    Fy_Generator_addByte(generator, instruction->reg_id);
    Fy_Generator_addConst16(generator, instruction->value);
}

static void Fy_InstructionType_AddReg16Const_run(Fy_VM *vm) {
    uint8_t *base = &vm->mem_space_bottom[vm->reg_ip];
    uint8_t reg_id = base[1];
    uint16_t value = Fy_MemoryGet16(&base[2]);
    uint16_t *reg_ptr = Fy_VM_getReg16Ptr(vm, reg_id);
    if (!reg_ptr) {
        FY_UNREACHABLE();
    }
    *reg_ptr += value;
}

static void Fy_InstructionType_AddReg16Reg16_write(Fy_Generator *generator, Fy_Instruction_OpReg16Reg16 *instruction) {
    Fy_Generator_addByte(generator, instruction->reg_id);
    Fy_Generator_addByte(generator, instruction->reg2_id);
}

static void Fy_InstructionType_AddReg16Reg16_run(Fy_VM *vm) {
    uint8_t *base = &vm->mem_space_bottom[vm->reg_ip];
    uint8_t reg = base[1];
    uint8_t reg2 = base[2];
    uint16_t *reg_ptr = Fy_VM_getReg16Ptr(vm, reg);
    uint16_t *reg2_ptr = Fy_VM_getReg16Ptr(vm, reg2);
    // Register not found
    if (!reg_ptr || !reg2_ptr) {
        Fy_VM_runtimeError(vm, Fy_RuntimeError_RegNotFound);
    }
    *reg_ptr += *reg2_ptr;
}

static void Fy_InstructionType_SubReg16Const_write(Fy_Generator *generator, Fy_Instruction_OpReg16Const *instruction) {
    Fy_Generator_addByte(generator, instruction->reg_id);
    Fy_Generator_addConst16(generator, instruction->value);
}

static void Fy_InstructionType_SubReg16Const_run(Fy_VM *vm) {
    uint8_t *base = &vm->mem_space_bottom[vm->reg_ip];
    uint8_t reg_id = base[1];
    uint16_t value = Fy_MemoryGet16(&base[2]);
    uint16_t *reg_ptr = Fy_VM_getReg16Ptr(vm, reg_id);
    if (!reg_ptr) {
        FY_UNREACHABLE();
    }
    *reg_ptr -= value;
}

static void Fy_InstructionType_SubReg16Reg16_write(Fy_Generator *generator, Fy_Instruction_OpReg16Reg16 *instruction) {
    Fy_Generator_addByte(generator, instruction->reg_id);
    Fy_Generator_addByte(generator, instruction->reg2_id);
}

static void Fy_InstructionType_SubReg16Reg16_run(Fy_VM *vm) {
    uint8_t *base = &vm->mem_space_bottom[vm->reg_ip];
    uint8_t reg = base[1];
    uint8_t reg2 = base[2];
    uint16_t *reg_ptr = Fy_VM_getReg16Ptr(vm, reg);
    uint16_t *reg2_ptr = Fy_VM_getReg16Ptr(vm, reg2);
    // Register not found
    if (!reg_ptr || !reg2_ptr) {
        Fy_VM_runtimeError(vm, Fy_RuntimeError_RegNotFound);
    }
    *reg_ptr -= *reg2_ptr;
}

static void Fy_InstructionType_CmpReg16Const_write(Fy_Generator *generator, Fy_Instruction_OpReg16Const *instruction) {
    Fy_Generator_addByte(generator, instruction->reg_id);
    Fy_Generator_addConst16(generator, instruction->value);
}

static void Fy_InstructionType_CmpReg16Const_run(Fy_VM *vm) {
    uint8_t *base = &vm->mem_space_bottom[vm->reg_ip];
    uint8_t reg_id = base[1];
    uint16_t value = Fy_MemoryGet16(&base[2]);
    uint16_t *reg_ptr = Fy_VM_getReg16Ptr(vm, reg_id);
    uint16_t res;
    if (!reg_ptr)
        FY_UNREACHABLE();
    res = *reg_ptr - value;
    Fy_VM_setResult16InFlags(vm, *((int16_t*)&res));
}

static void Fy_InstructionType_CmpReg16Reg16_write(Fy_Generator *generator, Fy_Instruction_OpReg16Reg16 *instruction) {
    Fy_Generator_addByte(generator, instruction->reg_id);
    Fy_Generator_addByte(generator, instruction->reg2_id);
}

void Fy_InstructionType_CmpReg16Reg16_run(Fy_VM *vm) {
    uint8_t *base = &vm->mem_space_bottom[vm->reg_ip];
    uint8_t reg = base[1];
    uint8_t reg2 = base[2];
    uint16_t *reg_ptr = Fy_VM_getReg16Ptr(vm, reg);
    uint16_t *reg2_ptr = Fy_VM_getReg16Ptr(vm, reg2);
    // Register not found
    if (!reg_ptr || !reg2_ptr) {
        Fy_VM_runtimeError(vm, Fy_RuntimeError_RegNotFound);
    }
    Fy_VM_setResult16InFlags(vm, *reg_ptr - *reg2_ptr);
}

/* Type definitions */
Fy_InstructionType Fy_InstructionType_MovReg16Const = {
    .opcode = 0,
    .additional_size = 3,
    .write_func = (Fy_InstructionWriteFunc)Fy_InstructionType_MovReg16Const_write,
    .run_func = Fy_InstructionType_MovReg16Const_run,
    .advance_after_run = true
};
Fy_InstructionType Fy_InstructionType_MovReg16Reg16 = {
    .opcode = 1,
    .additional_size = 2,
    .write_func = (Fy_InstructionWriteFunc)Fy_InstructionType_MovReg16Reg16_write,
    .run_func = Fy_InstructionType_MovReg16Reg16_run,
    .advance_after_run = true
};
Fy_InstructionType Fy_InstructionType_Debug = {
    .opcode = 2,
    .additional_size = 0,
    .write_func = NULL,
    .run_func = Fy_InstructionType_Debug_run,
    .advance_after_run = true
};
Fy_InstructionType Fy_InstructionType_EndProgram = {
    .opcode = 3,
    .additional_size = 0,
    .write_func = NULL,
    .run_func = Fy_InstructionType_EndProgram_run,
    .advance_after_run = true
};
Fy_InstructionType Fy_InstructionType_Jmp = {
    .opcode = 4,
    .additional_size = 2,
    .write_func = (Fy_InstructionWriteFunc)Fy_InstructionType_Jmp_write,
    .run_func = Fy_InstructionType_Jmp_run,
    .advance_after_run = false
};
Fy_InstructionType Fy_InstructionType_AddReg16Const = {
    .opcode = 5,
    .additional_size = 3,
    .write_func = (Fy_InstructionWriteFunc)Fy_InstructionType_AddReg16Const_write,
    .run_func = Fy_InstructionType_AddReg16Const_run,
    .advance_after_run = true
};
Fy_InstructionType Fy_InstructionType_AddReg16Reg16 = {
    .opcode = 6,
    .additional_size = 2,
    .write_func = (Fy_InstructionWriteFunc)Fy_InstructionType_AddReg16Reg16_write,
    .run_func = Fy_InstructionType_AddReg16Reg16_run,
    .advance_after_run = true
};
Fy_InstructionType Fy_InstructionType_SubReg16Const = {
    .opcode = 7,
    .additional_size = 3,
    .write_func = (Fy_InstructionWriteFunc)Fy_InstructionType_SubReg16Const_write,
    .run_func = Fy_InstructionType_SubReg16Const_run,
    .advance_after_run = true
};
Fy_InstructionType Fy_InstructionType_SubReg16Reg16 = {
    .opcode = 8,
    .additional_size = 2,
    .write_func = (Fy_InstructionWriteFunc)Fy_InstructionType_SubReg16Reg16_write,
    .run_func = Fy_InstructionType_SubReg16Reg16_run,
    .advance_after_run = true
};
Fy_InstructionType Fy_InstructionType_CmpReg16Const = {
    .opcode = 9,
    .additional_size = 3,
    .write_func = (Fy_InstructionWriteFunc)Fy_InstructionType_CmpReg16Const_write,
    .run_func = Fy_InstructionType_CmpReg16Const_run,
    .advance_after_run = true
};
Fy_InstructionType Fy_InstructionType_CmpReg16Reg16 = {
    .opcode = 10,
    .additional_size = 2,
    .write_func = (Fy_InstructionWriteFunc)Fy_InstructionType_CmpReg16Reg16_write,
    .run_func = Fy_InstructionType_CmpReg16Reg16_run,
    .advance_after_run = true
};

Fy_InstructionType *Fy_instructionTypes[] = {
    &Fy_InstructionType_MovReg16Const,
    &Fy_InstructionType_MovReg16Reg16,
    &Fy_InstructionType_Debug,
    &Fy_InstructionType_EndProgram,
    &Fy_InstructionType_Jmp,
    &Fy_InstructionType_AddReg16Const,
    &Fy_InstructionType_AddReg16Reg16,
    &Fy_InstructionType_SubReg16Const,
    &Fy_InstructionType_SubReg16Reg16,
    &Fy_InstructionType_CmpReg16Const,
    &Fy_InstructionType_CmpReg16Reg16
};
