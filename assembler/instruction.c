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

static void Fy_InstructionType_MovReg16Const_write(Fy_Generator *generator, Fy_Instruction_MovReg16Const *instruction) {
    Fy_Generator_addByte(generator, instruction->reg_id);
    Fy_Generator_addConst16(generator, instruction->val);
}

static void Fy_InstructionType_MovReg16Const_run(Fy_VM *vm) {
    uint8_t *base = &vm->mem_space_bottom[vm->reg_ip];
    uint8_t reg = base[1];
    uint16_t val = Fy_MemoryGet16(&base[2]);
    uint16_t *reg_ptr = Fy_VM_getReg16Ptr(vm, reg);
    // Register not found
    if (!reg_ptr) {
        Fy_VM_runtimeError(vm, Fy_RuntimeError_RegNotFound);
    }
    *reg_ptr = val;
}

static void Fy_InstructionType_MovReg16Reg16_write(Fy_Generator *generator, Fy_Instruction_MovReg16Reg16 *instruction) {
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
    printf("DEBUG!\n");
}

static void Fy_InstructionType_EndProgram_run(Fy_VM *vm) {
    vm->running = false;
}

/* Type definitions */

Fy_InstructionType Fy_InstructionType_MovReg16Const = {
    .opcode = 0,
    .additional_size = 3,
    .write_func = (Fy_InstructionWriteFunc)Fy_InstructionType_MovReg16Const_write,
    .run_func = Fy_InstructionType_MovReg16Const_run
};

Fy_InstructionType Fy_InstructionType_MovReg16Reg16 = {
    .opcode = 1,
    .additional_size = 2,
    .write_func = (Fy_InstructionWriteFunc)Fy_InstructionType_MovReg16Reg16_write,
    .run_func = Fy_InstructionType_MovReg16Reg16_run
};

Fy_InstructionType Fy_InstructionType_Debug = {
    .opcode = 2,
    .additional_size = 0,
    .write_func = NULL,
    .run_func = Fy_InstructionType_Debug_run
};

Fy_InstructionType Fy_InstructionType_EndProgram = {
    .opcode = 3,
    .additional_size = 0,
    .write_func = NULL,
    .run_func = Fy_InstructionType_EndProgram_run
};

Fy_InstructionType *Fy_instructionTypes[] = {
    &Fy_InstructionType_MovReg16Const,
    &Fy_InstructionType_MovReg16Reg16,
    &Fy_InstructionType_Debug,
    &Fy_InstructionType_EndProgram
};
