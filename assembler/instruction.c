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
    Fy_Generator_addWord(generator, instruction->value);
}

static void Fy_InstructionType_MovReg16Const_run(Fy_VM *vm) {
    uint8_t *base = &vm->mem_space_bottom[vm->reg_ip];
    uint8_t reg = base[1];
    uint16_t val = Fy_MemoryGet16(&base[2]);
    Fy_VM_setReg16(vm, reg, val);
}

static void Fy_InstructionType_MovReg16Reg16_write(Fy_Generator *generator, Fy_Instruction_OpReg16Reg16 *instruction) {
    Fy_Generator_addByte(generator, instruction->reg_id);
    Fy_Generator_addByte(generator, instruction->reg2_id);
}

static void Fy_InstructionType_MovReg16Reg16_run(Fy_VM *vm) {
    uint8_t *base = &vm->mem_space_bottom[vm->reg_ip];
    uint8_t reg = base[1];
    uint8_t reg2 = base[2];
    uint16_t value;
    value = Fy_VM_getReg16(vm, reg2);
    Fy_VM_setReg16(vm, reg, value);
}

static void Fy_InstructionType_Debug_run(Fy_VM *vm) {
    (void)vm;
    printf("DEBUG INFO:\n");
    printf("AX: %.2X %.2X\n", vm->reg_ax[0], vm->reg_ax[1]);
    printf("BX: %.2X %.2X\n", vm->reg_bx[0], vm->reg_bx[1]);
    printf("CX: %.2X %.2X\n", vm->reg_cx[0], vm->reg_cx[1]);
    printf("DX: %.2X %.2X\n", vm->reg_dx[0], vm->reg_dx[1]);
    printf("IP: %.4X\n", vm->reg_ip);
    printf("SP: %.4X\n", vm->reg_sp);
    printf("FLAG_ZERO: %d\n", vm->flags & FY_FLAGS_ZERO ? 1 : 0);
    printf("FLAG_SIGN: %d\n", vm->flags & FY_FLAGS_SIGN ? 1 : 0);
}

static void Fy_InstructionType_EndProgram_run(Fy_VM *vm) {
    vm->running = false;
}

static void Fy_InstructionType_AddReg16Const_write(Fy_Generator *generator, Fy_Instruction_OpReg16Const *instruction) {
    Fy_Generator_addByte(generator, instruction->reg_id);
    Fy_Generator_addWord(generator, instruction->value);
}

static void Fy_InstructionType_AddReg16Const_run(Fy_VM *vm) {
    uint8_t *base = &vm->mem_space_bottom[vm->reg_ip];
    uint8_t reg = base[1];
    uint16_t base_value;
    uint16_t added_value = Fy_MemoryGet16(&base[2]);
    base_value = Fy_VM_getReg16(vm, reg);
    Fy_VM_setReg16(vm, reg, base_value + added_value);
}

static void Fy_InstructionType_AddReg16Reg16_write(Fy_Generator *generator, Fy_Instruction_OpReg16Reg16 *instruction) {
    Fy_Generator_addByte(generator, instruction->reg_id);
    Fy_Generator_addByte(generator, instruction->reg2_id);
}

static void Fy_InstructionType_AddReg16Reg16_run(Fy_VM *vm) {
    uint8_t *base = &vm->mem_space_bottom[vm->reg_ip];
    uint8_t reg = base[1];
    uint8_t reg2 = base[2];
    uint16_t reg_value;
    uint16_t reg2_value;

    reg_value = Fy_VM_getReg16(vm, reg);
    reg2_value = Fy_VM_getReg16(vm, reg2);
    Fy_VM_setReg16(vm, reg, reg_value + reg2_value);
}

static void Fy_InstructionType_SubReg16Const_write(Fy_Generator *generator, Fy_Instruction_OpReg16Const *instruction) {
    Fy_Generator_addByte(generator, instruction->reg_id);
    Fy_Generator_addWord(generator, instruction->value);
}

static void Fy_InstructionType_SubReg16Const_run(Fy_VM *vm) {
    uint8_t *base = &vm->mem_space_bottom[vm->reg_ip];
    uint8_t reg = base[1];
    uint16_t base_value;
    uint16_t sub_value = Fy_MemoryGet16(&base[2]);
    base_value = Fy_VM_getReg16(vm, reg);
    Fy_VM_setReg16(vm, reg, base_value - sub_value);
}

static void Fy_InstructionType_SubReg16Reg16_write(Fy_Generator *generator, Fy_Instruction_OpReg16Reg16 *instruction) {
    Fy_Generator_addByte(generator, instruction->reg_id);
    Fy_Generator_addByte(generator, instruction->reg2_id);
}

static void Fy_InstructionType_SubReg16Reg16_run(Fy_VM *vm) {
    uint8_t *base = &vm->mem_space_bottom[vm->reg_ip];
    uint8_t reg = base[1];
    uint8_t reg2 = base[2];
    uint16_t reg_value;
    uint16_t reg2_value;

    reg_value = Fy_VM_getReg16(vm, reg);
    reg2_value = Fy_VM_getReg16(vm, reg2);
    Fy_VM_setReg16(vm, reg, reg_value - reg2_value);
}

static void Fy_InstructionType_CmpReg16Const_write(Fy_Generator *generator, Fy_Instruction_OpReg16Const *instruction) {
    Fy_Generator_addByte(generator, instruction->reg_id);
    Fy_Generator_addWord(generator, instruction->value);
}

static void Fy_InstructionType_CmpReg16Const_run(Fy_VM *vm) {
    uint8_t *base = &vm->mem_space_bottom[vm->reg_ip];
    uint8_t reg_id = base[1];
    uint16_t value = Fy_MemoryGet16(&base[2]);
    uint16_t reg_value;
    uint16_t res;

    reg_value = Fy_VM_getReg16(vm, reg_id);
    res = reg_value - value;
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
    uint16_t reg_value;
    uint16_t reg2_value;

    reg_value = Fy_VM_getReg16(vm, reg);
    reg2_value = Fy_VM_getReg16(vm, reg2);
    Fy_VM_setResult16InFlags(vm, reg_value - reg2_value);
}

static void Fy_InstructionType_Jmp_write(Fy_Generator *generator, Fy_Instruction_OpLabel *instruction) {
    Fy_Generator_addWord(generator, instruction->address);
}

static void Fy_InstructionType_Jmp_run(Fy_VM *vm) {
    Fy_VM_setIpToRelAddress(vm, Fy_MemoryGet16(&vm->mem_space_bottom[vm->reg_ip + 1]));
}

static void Fy_InstructionType_Je_write(Fy_Generator *generator, Fy_Instruction_OpLabel *instruction) {
    Fy_Generator_addWord(generator, instruction->address);
}

static void Fy_InstructionType_Je_run(Fy_VM *vm) {
    // If the zero flag is on
    if (vm->flags & FY_FLAGS_ZERO)
        Fy_VM_setIpToRelAddress(vm, Fy_MemoryGet16(&vm->mem_space_bottom[vm->reg_ip + 1]));
    else // FIXME: Find a better way to handle no-jumps
        vm->reg_ip += 1 + 2; // Advance otherwise
}

static void Fy_InstructionType_Jl_write(Fy_Generator *generator, Fy_Instruction_OpLabel *instruction) {
    Fy_Generator_addWord(generator, instruction->address);
}

static void Fy_InstructionType_Jl_run(Fy_VM *vm) {
    // If we have the sign it means the result was negative, thus the lhs was smaller than the rhs
    if (vm->flags & FY_FLAGS_SIGN)
        Fy_VM_setIpToRelAddress(vm, Fy_MemoryGet16(&vm->mem_space_bottom[vm->reg_ip + 1]));
    else // FIXME: Find a better way to handle no-jumps
        vm->reg_ip += 1 + 2; // Advance otherwise
}

static void Fy_InstructionType_Jg_write(Fy_Generator *generator, Fy_Instruction_OpLabel *instruction) {
    Fy_Generator_addWord(generator, instruction->address);
}

static void Fy_InstructionType_Jg_run(Fy_VM *vm) {
    // If we don't have the sign and don't equal 0 it means the result was positive, thus the lhs was bigger than than the rhs
    if (!(vm->flags & FY_FLAGS_SIGN) && !(vm->flags & FY_FLAGS_ZERO))
        Fy_VM_setIpToRelAddress(vm, Fy_MemoryGet16(&vm->mem_space_bottom[vm->reg_ip + 1]));
    else // FIXME: Find a better way to handle no-jumps
        vm->reg_ip += 1 + 2; // Advance otherwise
}

static void Fy_InstructionType_PushConst_write(Fy_Generator *generator, Fy_Instruction_PushConst *instruction) {
    Fy_Generator_addWord(generator, instruction->value);
}

static void Fy_InstructionType_PushConst_run(Fy_VM *vm) {
    uint16_t value = Fy_MemoryGet16(&vm->mem_space_bottom[vm->reg_ip + 1]);
    Fy_VM_pushToStack(vm, value);
}

static void Fy_InstructionType_PushReg16_write(Fy_Generator *generator, Fy_Instruction_OpReg16 *instruction) {
    Fy_Generator_addByte(generator, instruction->reg_id);
}

static void Fy_InstructionType_PushReg16_run(Fy_VM *vm) {
    uint8_t reg = vm->mem_space_bottom[vm->reg_ip + 1];
    uint16_t reg_value;

    reg_value = Fy_VM_getReg16(vm, reg);
    Fy_VM_pushToStack(vm, reg_value);
}

static void Fy_InstructionType_Pop_write(Fy_Generator *generator, Fy_Instruction_OpReg16 *instruction) {
    Fy_Generator_addByte(generator, instruction->reg_id);
}

static void Fy_InstructionType_Pop_run(Fy_VM *vm) {
    uint8_t reg = vm->mem_space_bottom[vm->reg_ip + 1];
    uint16_t popped = Fy_VM_popFromStack(vm); // FIXME: This should happen only after the register was verified to be valid

    Fy_VM_setReg16(vm, reg, popped);
}

static void Fy_InstructionType_MovReg8Const_write(Fy_Generator *generator, Fy_Instruction_OpReg8Const *instruction) {
    Fy_Generator_addByte(generator, instruction->reg_id);
    Fy_Generator_addByte(generator, instruction->value);
}

static void Fy_InstructionType_MovReg8Const_run(Fy_VM *vm) {
    uint8_t *base = &vm->mem_space_bottom[vm->reg_ip];
    uint8_t reg = base[1];
    uint8_t val = base[2];

    Fy_VM_setReg8(vm, reg, val);
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
Fy_InstructionType Fy_InstructionType_AddReg16Const = {
    .opcode = 4,
    .additional_size = 3,
    .write_func = (Fy_InstructionWriteFunc)Fy_InstructionType_AddReg16Const_write,
    .run_func = Fy_InstructionType_AddReg16Const_run,
    .advance_after_run = true
};
Fy_InstructionType Fy_InstructionType_AddReg16Reg16 = {
    .opcode = 5,
    .additional_size = 2,
    .write_func = (Fy_InstructionWriteFunc)Fy_InstructionType_AddReg16Reg16_write,
    .run_func = Fy_InstructionType_AddReg16Reg16_run,
    .advance_after_run = true
};
Fy_InstructionType Fy_InstructionType_SubReg16Const = {
    .opcode = 6,
    .additional_size = 3,
    .write_func = (Fy_InstructionWriteFunc)Fy_InstructionType_SubReg16Const_write,
    .run_func = Fy_InstructionType_SubReg16Const_run,
    .advance_after_run = true
};
Fy_InstructionType Fy_InstructionType_SubReg16Reg16 = {
    .opcode = 7,
    .additional_size = 2,
    .write_func = (Fy_InstructionWriteFunc)Fy_InstructionType_SubReg16Reg16_write,
    .run_func = Fy_InstructionType_SubReg16Reg16_run,
    .advance_after_run = true
};
Fy_InstructionType Fy_InstructionType_CmpReg16Const = {
    .opcode = 8,
    .additional_size = 3,
    .write_func = (Fy_InstructionWriteFunc)Fy_InstructionType_CmpReg16Const_write,
    .run_func = Fy_InstructionType_CmpReg16Const_run,
    .advance_after_run = true
};
Fy_InstructionType Fy_InstructionType_CmpReg16Reg16 = {
    .opcode = 9,
    .additional_size = 2,
    .write_func = (Fy_InstructionWriteFunc)Fy_InstructionType_CmpReg16Reg16_write,
    .run_func = Fy_InstructionType_CmpReg16Reg16_run,
    .advance_after_run = true
};
Fy_InstructionType Fy_InstructionType_Jmp = {
    .opcode = 10,
    .additional_size = 2,
    .write_func = (Fy_InstructionWriteFunc)Fy_InstructionType_Jmp_write,
    .run_func = Fy_InstructionType_Jmp_run,
    .advance_after_run = false
};
Fy_InstructionType Fy_InstructionType_Je = {
    .opcode = 11,
    .additional_size = 2,
    .write_func = (Fy_InstructionWriteFunc)Fy_InstructionType_Je_write,
    .run_func = Fy_InstructionType_Je_run,
    .advance_after_run = false
};
Fy_InstructionType Fy_InstructionType_Jl = {
    .opcode = 12,
    .additional_size = 2,
    .write_func = (Fy_InstructionWriteFunc)Fy_InstructionType_Jl_write,
    .run_func = Fy_InstructionType_Jl_run,
    .advance_after_run = false
};
Fy_InstructionType Fy_InstructionType_Jg = {
    .opcode = 13,
    .additional_size = 2,
    .write_func = (Fy_InstructionWriteFunc)Fy_InstructionType_Jg_write,
    .run_func = Fy_InstructionType_Jg_run,
    .advance_after_run = false
};
Fy_InstructionType Fy_InstructionType_PushConst = {
    .opcode = 14,
    .additional_size = 2,
    .write_func = (Fy_InstructionWriteFunc)Fy_InstructionType_PushConst_write,
    .run_func = Fy_InstructionType_PushConst_run,
    .advance_after_run = true
};
Fy_InstructionType Fy_InstructionType_PushReg16 = {
    .opcode = 15,
    .additional_size = 1,
    .write_func = (Fy_InstructionWriteFunc)Fy_InstructionType_PushReg16_write,
    .run_func = Fy_InstructionType_PushReg16_run,
    .advance_after_run = true
};
Fy_InstructionType Fy_InstructionType_Pop = {
    .opcode = 16,
    .additional_size = 1,
    .write_func = (Fy_InstructionWriteFunc)Fy_InstructionType_Pop_write,
    .run_func = Fy_InstructionType_Pop_run,
    .advance_after_run = true
};
Fy_InstructionType Fy_InstructionType_MovReg8Const = {
    .opcode = 17,
    .additional_size = 2,
    .write_func = (Fy_InstructionWriteFunc)Fy_InstructionType_MovReg8Const_write,
    .run_func = Fy_InstructionType_MovReg8Const_run,
    .advance_after_run = true
};

Fy_InstructionType *Fy_instructionTypes[] = {
    &Fy_InstructionType_MovReg16Const,
    &Fy_InstructionType_MovReg16Reg16,
    &Fy_InstructionType_Debug,
    &Fy_InstructionType_EndProgram,
    &Fy_InstructionType_AddReg16Const,
    &Fy_InstructionType_AddReg16Reg16,
    &Fy_InstructionType_SubReg16Const,
    &Fy_InstructionType_SubReg16Reg16,
    &Fy_InstructionType_CmpReg16Const,
    &Fy_InstructionType_CmpReg16Reg16,
    &Fy_InstructionType_Jmp,
    &Fy_InstructionType_Je,
    &Fy_InstructionType_Jl,
    &Fy_InstructionType_Jg,
    &Fy_InstructionType_PushConst,
    &Fy_InstructionType_PushReg16,
    &Fy_InstructionType_Pop,
    &Fy_InstructionType_MovReg8Const
};
