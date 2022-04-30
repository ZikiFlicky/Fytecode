#include "fy.h"

/* Returns a new instruction initialized with the given type */
Fy_Instruction *Fy_Instruction_New(Fy_InstructionType *type, size_t size) {
    Fy_Instruction *instruction = malloc(size);
    instruction->type = type;
    return instruction;
}

/* Instruction type functions */
static void Fy_instructionTypeMovReg16Const_write(Fy_Generator *generator, Fy_Instruction_OpReg16Const *instruction) {
    Fy_Generator_addByte(generator, instruction->reg_id);
    Fy_Generator_addWord(generator, instruction->value);
}

static void Fy_instructionTypeMovReg16Const_run(Fy_VM *vm, uint16_t address) {
    uint8_t reg = Fy_VM_getMem8(vm, address + 0);
    uint16_t val = Fy_VM_getMem16(vm, address + 1);
    Fy_VM_setReg16(vm, reg, val);
}

static void Fy_instructionTypeMovReg16Reg16_write(Fy_Generator *generator, Fy_Instruction_OpReg16Reg16 *instruction) {
    Fy_Generator_addByte(generator, instruction->reg_id);
    Fy_Generator_addByte(generator, instruction->reg2_id);
}

static void Fy_instructionTypeMovReg16Reg16_run(Fy_VM *vm, uint16_t address) {
    uint8_t reg = Fy_VM_getMem8(vm, address + 0);
    uint8_t reg2 = Fy_VM_getMem8(vm, address + 1);
    uint16_t value;
    if (!Fy_VM_getReg16(vm, reg2, &value))
        return;
    Fy_VM_setReg16(vm, reg, value);
}

static void Fy_instructionTypeDebug_run(Fy_VM *vm, uint16_t address) {
    (void)address;
    printf("DEBUG INFO:\n");
    printf("AX: (h)%.2X (l)%.2X\n", vm->reg_ax[1], vm->reg_ax[0]);
    printf("BX: (h)%.2X (l)%.2X\n", vm->reg_bx[1], vm->reg_bx[0]);
    printf("CX: (h)%.2X (l)%.2X\n", vm->reg_cx[1], vm->reg_cx[0]);
    printf("DX: (h)%.2X (l)%.2X\n", vm->reg_dx[1], vm->reg_dx[0]);
    printf("IP: %.4X\n", vm->reg_ip);
    printf("SP: %.4X\n", vm->reg_sp);
    printf("BP: %.4X\n", vm->reg_bp);
    printf("FLAG_ZERO: %d\n", vm->flags & FY_FLAGS_ZERO ? 1 : 0);
    printf("FLAG_SIGN: %d\n", vm->flags & FY_FLAGS_SIGN ? 1 : 0);
}

static void Fy_instructionTypeDebugStack_run(Fy_VM *vm, uint16_t address) {
    (void)address;
    // if in range
    if (vm->reg_sp <= vm->stack_offset && vm->reg_sp >= (vm->stack_offset - vm->stack_size)) {
        printf("STACK INFO:\n");
        printf("%d items, %d bytes\n", (vm->stack_offset - vm->reg_sp) / 2, vm->stack_offset - vm->reg_sp);

        for (uint16_t addr = vm->stack_offset; addr > vm->reg_sp;) {
            addr -= 2;
            printf("%.4x: %.4X\n", addr, Fy_VM_getMem16(vm, addr));
        }
    } else {
        printf("NO STACK INFO\n");
    }

}

static void Fy_instructionTypeEndProgram_run(Fy_VM *vm, uint16_t address) {
    (void)address;
    vm->running = false;
}

static void Fy_instructionTypeAddReg16Const_write(Fy_Generator *generator, Fy_Instruction_OpReg16Const *instruction) {
    Fy_Generator_addByte(generator, instruction->reg_id);
    Fy_Generator_addWord(generator, instruction->value);
}

static void Fy_instructionTypeAddReg16Const_run(Fy_VM *vm, uint16_t address) {
    uint8_t reg = Fy_VM_getMem8(vm, address + 0);
    uint16_t base_value;
    uint16_t added_value = Fy_VM_getMem16(vm, address + 1);
    if (!Fy_VM_getReg16(vm, reg, &base_value))
        return;
    Fy_VM_setReg16(vm, reg, base_value + added_value);
}

static void Fy_instructionTypeAddReg16Reg16_write(Fy_Generator *generator, Fy_Instruction_OpReg16Reg16 *instruction) {
    Fy_Generator_addByte(generator, instruction->reg_id);
    Fy_Generator_addByte(generator, instruction->reg2_id);
}

static void Fy_instructionTypeAddReg16Reg16_run(Fy_VM *vm, uint16_t address) {
    uint8_t reg = Fy_VM_getMem8(vm, address + 0);
    uint8_t reg2 = Fy_VM_getMem8(vm, address + 1);
    uint16_t reg_value;
    uint16_t reg2_value;

    if (!Fy_VM_getReg16(vm, reg, &reg_value))
        return;
    if (!Fy_VM_getReg16(vm, reg2, &reg2_value))
        return;
    Fy_VM_setReg16(vm, reg, reg_value + reg2_value);
}

static void Fy_instructionTypeSubReg16Const_write(Fy_Generator *generator, Fy_Instruction_OpReg16Const *instruction) {
    Fy_Generator_addByte(generator, instruction->reg_id);
    Fy_Generator_addWord(generator, instruction->value);
}

static void Fy_instructionTypeSubReg16Const_run(Fy_VM *vm, uint16_t address) {
    uint8_t reg = Fy_VM_getMem8(vm, address + 0);
    uint16_t base_value;
    uint16_t sub_value = Fy_VM_getMem16(vm, address + 1);
    if (!Fy_VM_getReg16(vm, reg, &base_value))
        return;
    Fy_VM_setReg16(vm, reg, base_value - sub_value);
}

static void Fy_instructionTypeSubReg16Reg16_write(Fy_Generator *generator, Fy_Instruction_OpReg16Reg16 *instruction) {
    Fy_Generator_addByte(generator, instruction->reg_id);
    Fy_Generator_addByte(generator, instruction->reg2_id);
}

static void Fy_instructionTypeSubReg16Reg16_run(Fy_VM *vm, uint16_t address) {
    uint8_t reg = Fy_VM_getMem8(vm, address + 0);
    uint8_t reg2 = Fy_VM_getMem8(vm, address + 1);
    uint16_t reg_value;
    uint16_t reg2_value;

    if (!Fy_VM_getReg16(vm, reg, &reg_value))
        return;
    if (!Fy_VM_getReg16(vm, reg2, &reg2_value))
        return;
    Fy_VM_setReg16(vm, reg, reg_value - reg2_value);
}

static void Fy_instructionTypeCmpReg16Const_write(Fy_Generator *generator, Fy_Instruction_OpReg16Const *instruction) {
    Fy_Generator_addByte(generator, instruction->reg_id);
    Fy_Generator_addWord(generator, instruction->value);
}

static void Fy_instructionTypeCmpReg16Const_run(Fy_VM *vm, uint16_t address) {
    uint8_t reg_id = Fy_VM_getMem8(vm, address + 0);
    uint16_t value = Fy_VM_getMem16(vm, address + 1);
    uint16_t reg_value;
    uint16_t res;

    if (!Fy_VM_getReg16(vm, reg_id, &reg_value))
        return;
    res = reg_value - value;
    Fy_VM_setResult16InFlags(vm, *((int16_t*)&res));
}

static void Fy_instructionTypeCmpReg16Reg16_write(Fy_Generator *generator, Fy_Instruction_OpReg16Reg16 *instruction) {
    Fy_Generator_addByte(generator, instruction->reg_id);
    Fy_Generator_addByte(generator, instruction->reg2_id);
}

void Fy_instructionTypeCmpReg16Reg16_run(Fy_VM *vm, uint16_t address) {
    uint8_t reg = Fy_VM_getMem8(vm, address + 0);
    uint8_t reg2 = Fy_VM_getMem8(vm, address + 1);
    uint16_t reg_value;
    uint16_t reg2_value;

    if (!Fy_VM_getReg16(vm, reg, &reg_value))
        return;
    if (!Fy_VM_getReg16(vm, reg2, &reg2_value))
        return;
    Fy_VM_setResult16InFlags(vm, reg_value - reg2_value);
}

static void Fy_instructionTypeJmp_write(Fy_Generator *generator, Fy_Instruction_OpLabel *instruction) {
    Fy_Generator_addWord(generator, instruction->address);
}

static void Fy_instructionTypeJmp_run(Fy_VM *vm, uint16_t address) {
    Fy_VM_setIpToRelAddress(vm, Fy_VM_getMem16(vm, address + 0));
}

static void Fy_instructionTypeJe_write(Fy_Generator *generator, Fy_Instruction_OpLabel *instruction) {
    Fy_Generator_addWord(generator, instruction->address);
}

static void Fy_instructionTypeJe_run(Fy_VM *vm, uint16_t address) {
    // If the zero flag is on
    if (vm->flags & FY_FLAGS_ZERO)
        Fy_VM_setIpToRelAddress(vm, Fy_VM_getMem16(vm, address + 0));
}

static void Fy_instructionTypeJl_write(Fy_Generator *generator, Fy_Instruction_OpLabel *instruction) {
    Fy_Generator_addWord(generator, instruction->address);
}

static void Fy_instructionTypeJl_run(Fy_VM *vm, uint16_t address) {
    // If we have the sign it means the result was negative, thus the lhs was smaller than the rhs
    if (vm->flags & FY_FLAGS_SIGN)
        Fy_VM_setIpToRelAddress(vm, Fy_VM_getMem16(vm, address + 0));
}

static void Fy_instructionTypeJg_write(Fy_Generator *generator, Fy_Instruction_OpLabel *instruction) {
    Fy_Generator_addWord(generator, instruction->address);
}

static void Fy_instructionTypeJg_run(Fy_VM *vm, uint16_t address) {
    // If we don't have the sign and don't equal 0 it means the result was positive, thus the lhs was bigger than than the rhs
    if (!(vm->flags & FY_FLAGS_SIGN) && !(vm->flags & FY_FLAGS_ZERO))
        Fy_VM_setIpToRelAddress(vm, Fy_VM_getMem16(vm, address + 0));
}

static void Fy_instructionTypePushConst_write(Fy_Generator *generator, Fy_Instruction_OpConst16 *instruction) {
    Fy_Generator_addWord(generator, instruction->value);
}

static void Fy_instructionTypePushConst_run(Fy_VM *vm, uint16_t address) {
    uint16_t value = Fy_VM_getMem16(vm, address + 0);
    Fy_VM_pushToStack(vm, value);
}

static void Fy_instructionTypePushReg16_write(Fy_Generator *generator, Fy_Instruction_OpReg16 *instruction) {
    Fy_Generator_addByte(generator, instruction->reg_id);
}

static void Fy_instructionTypePushReg16_run(Fy_VM *vm, uint16_t address) {
    uint8_t reg = Fy_VM_getMem8(vm, address + 0);
    uint16_t reg_value;

    if (!Fy_VM_getReg16(vm, reg, &reg_value))
        return;
    Fy_VM_pushToStack(vm, reg_value);
}

static void Fy_instructionTypePop_write(Fy_Generator *generator, Fy_Instruction_OpReg16 *instruction) {
    Fy_Generator_addByte(generator, instruction->reg_id);
}

static void Fy_instructionTypePop_run(Fy_VM *vm, uint16_t address) {
    uint8_t reg = Fy_VM_getMem8(vm, address + 0);
    uint16_t popped;

    if (!Fy_VM_isWritableReg16(vm, reg)) {
        Fy_VM_runtimeError(vm, Fy_RuntimeError_WritableReg16NotFound, "'%X'", reg);
        return;
    }

    popped = Fy_VM_popFromStack(vm);
    Fy_VM_setReg16(vm, reg, popped);
}

static void Fy_instructionTypeMovReg8Const_write(Fy_Generator *generator, Fy_Instruction_OpReg8Const *instruction) {
    Fy_Generator_addByte(generator, instruction->reg_id);
    Fy_Generator_addByte(generator, instruction->value);
}

static void Fy_instructionTypeMovReg8Const_run(Fy_VM *vm, uint16_t address) {
    uint8_t reg = Fy_VM_getMem8(vm, address + 0);
    uint8_t val = Fy_VM_getMem8(vm, address + 1);

    Fy_VM_setReg8(vm, reg, val);
}

static void Fy_instructionTypeMovReg8Reg8_write(Fy_Generator *generator, Fy_Instruction_OpReg8Reg8 *instruction) {
    Fy_Generator_addByte(generator, instruction->reg_id);
    Fy_Generator_addByte(generator, instruction->reg2_id);
}

static void Fy_instructionTypeMovReg8Reg8_run(Fy_VM *vm, uint16_t address) {
    uint8_t reg = Fy_VM_getMem8(vm, address + 0);
    uint8_t reg2 = Fy_VM_getMem8(vm, address + 1);
    uint8_t value;

    if (!Fy_VM_getReg8(vm, reg2, &value))
        return;

    Fy_VM_setReg8(vm, reg, value);
}

static void Fy_instructionTypeCall_write(Fy_Generator *generator, Fy_Instruction_OpLabel *instruction) {
    Fy_Generator_addWord(generator, instruction->address);
}

static void Fy_instructionTypeCall_run(Fy_VM *vm, uint16_t address) {
    uint16_t rel_addr = Fy_VM_getMem16(vm, address + 0);
    // Push address of next instruction
    Fy_VM_pushToStack(vm, vm->reg_ip);
    Fy_VM_setIpToRelAddress(vm, rel_addr);
}


static void Fy_instructionTypeRetConst16_write(Fy_Generator *generator, Fy_Instruction_OpConst16 *instruction) {
    Fy_Generator_addWord(generator, instruction->value);
}

static void Fy_instructionTypeRet_run(Fy_VM *vm, uint16_t address) {
    uint16_t addr = Fy_VM_popFromStack(vm);
    (void)address;
    vm->reg_ip = addr;
}

static void Fy_instructionTypeRetConst16_run(Fy_VM *vm, uint16_t address) {
    uint16_t addr = Fy_VM_popFromStack(vm);
    uint16_t to_change = Fy_VM_getMem16(vm, address + 0);
    vm->reg_ip = addr;
    // FIXME: Check if this overflows the stack
    vm->reg_sp += to_change;
}

static uint16_t Fy_instructionTypeMovReg16Mem_getsize(Fy_Instruction_OpReg16Mem *instruction) {
    return 1 + Fy_InlineValue_getMapping(&instruction->value, NULL);
}

static void Fy_instructionTypeMovReg16Mem_write(Fy_Generator *generator, Fy_Instruction_OpReg16Mem *instruction) {
    Fy_Generator_addByte(generator, instruction->reg_id);
    Fy_Generator_addMemory(generator, &instruction->value);
}

static void Fy_instructionTypeMovReg16Mem_run(Fy_VM *vm, uint16_t address) {
    uint8_t reg_id = Fy_VM_getMem8(vm, address + 0);
    uint16_t addr;
    uint16_t memparam_size = Fy_VM_readMemoryParam(vm, address + 1, &addr);
    uint16_t value;

    value = Fy_VM_getMem16(vm, addr);
    if (!Fy_VM_setReg16(vm, reg_id, value))
        return;
    vm->reg_ip += 1 + 1 + memparam_size;
}

static uint16_t Fy_instructionTypeLea_getsize(Fy_Instruction_OpReg16Mem *instruction) {
    return 1 + Fy_InlineValue_getMapping(&instruction->value, NULL);
}

static void Fy_instructionTypeLea_write(Fy_Generator *generator, Fy_Instruction_OpReg16Mem *instruction) {
    Fy_Generator_addByte(generator, instruction->reg_id);
    Fy_Generator_addMemory(generator, &instruction->value);
}

static void Fy_instructionTypeLea_run(Fy_VM *vm, uint16_t address) {
    uint8_t reg_id = Fy_VM_getMem8(vm, address + 0);
    uint16_t addr;
    uint16_t memparam_size = Fy_VM_readMemoryParam(vm, address + 1, &addr);

    if (!Fy_VM_setReg16(vm, reg_id, addr))
        return;
    vm->reg_ip += 1 + 1 + memparam_size;
}

static uint16_t Fy_instructionTypeMovMemReg16_getsize(Fy_Instruction_OpMemReg16 *instruction) {
    return Fy_InlineValue_getMapping(&instruction->value, NULL) + 1;
}

static void Fy_instructionTypeMovMemReg16_write(Fy_Generator *generator, Fy_Instruction_OpMemReg16 *instruction) {
    Fy_Generator_addMemory(generator, &instruction->value);
    Fy_Generator_addByte(generator, instruction->reg_id);
}

static void Fy_instructionTypeMovMemReg16_run(Fy_VM *vm, uint16_t address) {
    uint16_t addr, value;
    uint16_t memparam_size = Fy_VM_readMemoryParam(vm, address + 0, &addr);
    uint8_t reg_id = Fy_VM_getMem8(vm, address + memparam_size);

    if (!Fy_VM_getReg16(vm, reg_id, &value))
        return;
    Fy_VM_setMem16(vm, addr, value);
    vm->reg_ip += 1 + memparam_size + 1;
}

/* Type definitions */
Fy_InstructionType Fy_instructionTypeNop = {
    .opcode = 0,
    .variable_size = false,
    .additional_size = 0,
    .write_func = NULL,
    .run_func = NULL
};
Fy_InstructionType Fy_instructionTypeMovReg16Const = {
    .opcode = 1,
    .variable_size = false,
    .additional_size = 3,
    .write_func = (Fy_InstructionWriteFunc)Fy_instructionTypeMovReg16Const_write,
    .run_func = Fy_instructionTypeMovReg16Const_run
};
Fy_InstructionType Fy_instructionTypeMovReg16Reg16 = {
    .opcode = 2,
    .variable_size = false,
    .additional_size = 2,
    .write_func = (Fy_InstructionWriteFunc)Fy_instructionTypeMovReg16Reg16_write,
    .run_func = Fy_instructionTypeMovReg16Reg16_run
};
Fy_InstructionType Fy_instructionTypeEndProgram = {
    .opcode = 3,
    .variable_size = false,
    .additional_size = 0,
    .write_func = NULL,
    .run_func = Fy_instructionTypeEndProgram_run
};
Fy_InstructionType Fy_instructionTypeAddReg16Const = {
    .opcode = 4,
    .variable_size = false,
    .additional_size = 3,
    .write_func = (Fy_InstructionWriteFunc)Fy_instructionTypeAddReg16Const_write,
    .run_func = Fy_instructionTypeAddReg16Const_run
};
Fy_InstructionType Fy_instructionTypeAddReg16Reg16 = {
    .opcode = 5,
    .variable_size = false,
    .additional_size = 2,
    .write_func = (Fy_InstructionWriteFunc)Fy_instructionTypeAddReg16Reg16_write,
    .run_func = Fy_instructionTypeAddReg16Reg16_run
};
Fy_InstructionType Fy_instructionTypeSubReg16Const = {
    .opcode = 6,
    .variable_size = false,
    .additional_size = 3,
    .write_func = (Fy_InstructionWriteFunc)Fy_instructionTypeSubReg16Const_write,
    .run_func = Fy_instructionTypeSubReg16Const_run
};
Fy_InstructionType Fy_instructionTypeSubReg16Reg16 = {
    .opcode = 7,
    .variable_size = false,
    .additional_size = 2,
    .write_func = (Fy_InstructionWriteFunc)Fy_instructionTypeSubReg16Reg16_write,
    .run_func = Fy_instructionTypeSubReg16Reg16_run
};
Fy_InstructionType Fy_instructionTypeCmpReg16Const = {
    .opcode = 8,
    .variable_size = false,
    .additional_size = 3,
    .write_func = (Fy_InstructionWriteFunc)Fy_instructionTypeCmpReg16Const_write,
    .run_func = Fy_instructionTypeCmpReg16Const_run
};
Fy_InstructionType Fy_instructionTypeCmpReg16Reg16 = {
    .opcode = 9,
    .variable_size = false,
    .additional_size = 2,
    .write_func = (Fy_InstructionWriteFunc)Fy_instructionTypeCmpReg16Reg16_write,
    .run_func = Fy_instructionTypeCmpReg16Reg16_run
};
Fy_InstructionType Fy_instructionTypeJmp = {
    .opcode = 10,
    .variable_size = false,
    .additional_size = 2,
    .write_func = (Fy_InstructionWriteFunc)Fy_instructionTypeJmp_write,
    .run_func = Fy_instructionTypeJmp_run
};
Fy_InstructionType Fy_instructionTypeJe = {
    .opcode = 11,
    .variable_size = false,
    .additional_size = 2,
    .write_func = (Fy_InstructionWriteFunc)Fy_instructionTypeJe_write,
    .run_func = Fy_instructionTypeJe_run
};
Fy_InstructionType Fy_instructionTypeJl = {
    .opcode = 12,
    .variable_size = false,
    .additional_size = 2,
    .write_func = (Fy_InstructionWriteFunc)Fy_instructionTypeJl_write,
    .run_func = Fy_instructionTypeJl_run
};
Fy_InstructionType Fy_instructionTypeJg = {
    .opcode = 13,
    .variable_size = false,
    .additional_size = 2,
    .write_func = (Fy_InstructionWriteFunc)Fy_instructionTypeJg_write,
    .run_func = Fy_instructionTypeJg_run
};
Fy_InstructionType Fy_instructionTypePushConst = {
    .opcode = 14,
    .variable_size = false,
    .additional_size = 2,
    .write_func = (Fy_InstructionWriteFunc)Fy_instructionTypePushConst_write,
    .run_func = Fy_instructionTypePushConst_run
};
Fy_InstructionType Fy_instructionTypePushReg16 = {
    .opcode = 15,
    .variable_size = false,
    .additional_size = 1,
    .write_func = (Fy_InstructionWriteFunc)Fy_instructionTypePushReg16_write,
    .run_func = Fy_instructionTypePushReg16_run
};
Fy_InstructionType Fy_instructionTypePop = {
    .opcode = 16,
    .variable_size = false,
    .additional_size = 1,
    .write_func = (Fy_InstructionWriteFunc)Fy_instructionTypePop_write,
    .run_func = Fy_instructionTypePop_run
};
Fy_InstructionType Fy_instructionTypeMovReg8Const = {
    .opcode = 17,
    .variable_size = false,
    .additional_size = 2,
    .write_func = (Fy_InstructionWriteFunc)Fy_instructionTypeMovReg8Const_write,
    .run_func = Fy_instructionTypeMovReg8Const_run
};
Fy_InstructionType Fy_instructionTypeMovReg8Reg8 = {
    .opcode = 18,
    .variable_size = false,
    .additional_size = 2,
    .write_func = (Fy_InstructionWriteFunc)Fy_instructionTypeMovReg8Reg8_write,
    .run_func = Fy_instructionTypeMovReg8Reg8_run
};
Fy_InstructionType Fy_instructionTypeCall = {
    .opcode = 19,
    .variable_size = false,
    .additional_size = 2,
    .write_func = (Fy_InstructionWriteFunc)Fy_instructionTypeCall_write,
    .run_func = Fy_instructionTypeCall_run
};
Fy_InstructionType Fy_instructionTypeRet = {
    .opcode = 20,
    .variable_size = false,
    .additional_size = 0,
    .write_func = NULL,
    .run_func = Fy_instructionTypeRet_run
};
Fy_InstructionType Fy_instructionTypeRetConst16 = {
    .opcode = 21,
    .variable_size = false,
    .additional_size = 2,
    .write_func = (Fy_InstructionWriteFunc)Fy_instructionTypeRetConst16_write,
    .run_func = Fy_instructionTypeRetConst16_run
};
Fy_InstructionType Fy_instructionTypeDebug = {
    .opcode = 22,
    .variable_size = false,
    .additional_size = 0,
    .write_func = NULL,
    .run_func = Fy_instructionTypeDebug_run
};
Fy_InstructionType Fy_instructionTypeDebugStack = {
    .opcode = 23,
    .variable_size = false,
    .additional_size = 0,
    .write_func = NULL,
    .run_func = Fy_instructionTypeDebugStack_run
};
Fy_InstructionType Fy_instructionTypeMovReg16Mem = {
    .opcode = 24,
    .variable_size = true,
    .getsize_func = (Fy_InstructionGetSizeFunc)Fy_instructionTypeMovReg16Mem_getsize,
    .write_func = (Fy_InstructionWriteFunc)Fy_instructionTypeMovReg16Mem_write,
    .run_func = Fy_instructionTypeMovReg16Mem_run
};
Fy_InstructionType Fy_instructionTypeLea = {
    .opcode = 25,
    .variable_size = true,
    .getsize_func = (Fy_InstructionGetSizeFunc)Fy_instructionTypeLea_getsize,
    .write_func = (Fy_InstructionWriteFunc)Fy_instructionTypeLea_write,
    .run_func = Fy_instructionTypeLea_run
};
Fy_InstructionType Fy_instructionTypeMovMemReg16 = {
    .opcode = 26,
    .variable_size = true,
    .getsize_func = (Fy_InstructionGetSizeFunc)Fy_instructionTypeMovMemReg16_getsize,
    .write_func = (Fy_InstructionWriteFunc)Fy_instructionTypeMovMemReg16_write,
    .run_func = Fy_instructionTypeMovMemReg16_run
};

Fy_InstructionType *Fy_instructionTypes[] = {
    &Fy_instructionTypeNop,
    &Fy_instructionTypeMovReg16Const,
    &Fy_instructionTypeMovReg16Reg16,
    &Fy_instructionTypeEndProgram,
    &Fy_instructionTypeAddReg16Const,
    &Fy_instructionTypeAddReg16Reg16,
    &Fy_instructionTypeSubReg16Const,
    &Fy_instructionTypeSubReg16Reg16,
    &Fy_instructionTypeCmpReg16Const,
    &Fy_instructionTypeCmpReg16Reg16,
    &Fy_instructionTypeJmp,
    &Fy_instructionTypeJe,
    &Fy_instructionTypeJl,
    &Fy_instructionTypeJg,
    &Fy_instructionTypePushConst,
    &Fy_instructionTypePushReg16,
    &Fy_instructionTypePop,
    &Fy_instructionTypeMovReg8Const,
    &Fy_instructionTypeMovReg8Reg8,
    &Fy_instructionTypeCall,
    &Fy_instructionTypeRet,
    &Fy_instructionTypeRetConst16,
    &Fy_instructionTypeDebug,
    &Fy_instructionTypeDebugStack,
    &Fy_instructionTypeMovReg16Mem,
    &Fy_instructionTypeLea,
    &Fy_instructionTypeMovMemReg16
};
