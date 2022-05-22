#include "fy.h"

/* Returns a new instruction initialized with the given type */
Fy_Instruction *Fy_Instruction_New(Fy_InstructionType *type, size_t size) {
    Fy_Instruction *instruction = malloc(size);
    instruction->type = type;
    return instruction;
}

/* Instruction type functions */
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

static void Fy_instructionTypeInt_write(Fy_Generator *generator, Fy_Instruction_OpConst8 *instruction) {
    Fy_Generator_addByte(generator, instruction->value);
}

static void Fy_instructionTypeInt_run(Fy_VM *vm, uint16_t address) {
    uint8_t opcode = Fy_VM_getMem8(vm, address + 0);
    Fy_InterruptDef *interrupt = Fy_findInterruptDefByOpcode(opcode);
    if (!interrupt) {
        Fy_VM_runtimeError(vm, Fy_RuntimeError_InterruptNotFound, "%d", opcode);
        return;
    }
    Fy_InterruptDef_run(interrupt, vm);
}

static uint16_t Fy_instructionTypeBinaryOperator_getsize(Fy_Instruction_BinaryOperator *instruction) {
    uint16_t size = 1; // Because we have one info byte

    switch (instruction->type) {
    case Fy_BinaryOperatorArgsType_Reg16Const:
        size += 1 + 2;
        break;
    case Fy_BinaryOperatorArgsType_Reg16Reg16:
        size += 1 + 1;
        break;
    case Fy_BinaryOperatorArgsType_Reg16Memory16:
        size += 1 + Fy_InlineValue_getMapping(&instruction->as_reg16mem16.address, NULL);
        break;
    case Fy_BinaryOperatorArgsType_Reg8Const:
        size += 1 + 1;
        break;
    case Fy_BinaryOperatorArgsType_Reg8Reg8:
        size += 1 + 1;
        break;
    case Fy_BinaryOperatorArgsType_Reg8Memory8:
        size += 1 + Fy_InlineValue_getMapping(&instruction->as_reg8mem8.address, NULL);
        break;
    case Fy_BinaryOperatorArgsType_Memory16Const:
        size += 2 + Fy_InlineValue_getMapping(&instruction->as_mem16const.address, NULL);
        break;
    case Fy_BinaryOperatorArgsType_Memory16Reg16:
        size += 1 + Fy_InlineValue_getMapping(&instruction->as_mem16reg16.address, NULL);
        break;
    case Fy_BinaryOperatorArgsType_Memory8Const:
        size += 1 + Fy_InlineValue_getMapping(&instruction->as_mem8const.address, NULL);
        break;
    case Fy_BinaryOperatorArgsType_Memory8Reg8:
        size += 1 + Fy_InlineValue_getMapping(&instruction->as_mem8reg8.address, NULL);
        break;
    default:
        FY_UNREACHABLE();
    }

    return size;
}

static void Fy_instructionTypeBinaryOperator_write(Fy_Generator *generator, Fy_Instruction_BinaryOperator *instruction) {
    Fy_Generator_addByte(generator, (instruction->type << 4) + instruction->operator);
    switch (instruction->type) {
    case Fy_BinaryOperatorArgsType_Reg16Const:
        Fy_Generator_addByte(generator, instruction->as_reg16const.reg_id);
        Fy_Generator_addWord(generator, instruction->as_reg16const.value);
        break;
    case Fy_BinaryOperatorArgsType_Reg16Reg16:
        Fy_Generator_addByte(generator, instruction->as_reg16reg16.reg_id);
        Fy_Generator_addByte(generator, instruction->as_reg16reg16.reg2_id);
        break;
    case Fy_BinaryOperatorArgsType_Reg16Memory16:
        Fy_Generator_addByte(generator, instruction->as_reg16mem16.reg_id);
        Fy_Generator_addMemory(generator, &instruction->as_reg16mem16.address);
        break;
    case Fy_BinaryOperatorArgsType_Reg8Const:
        Fy_Generator_addByte(generator, instruction->as_reg8const.reg_id);
        Fy_Generator_addByte(generator, instruction->as_reg8const.value);
        break;
    case Fy_BinaryOperatorArgsType_Reg8Reg8:
        Fy_Generator_addByte(generator, instruction->as_reg8reg8.reg_id);
        Fy_Generator_addByte(generator, instruction->as_reg8reg8.reg2_id);
        break;
    case Fy_BinaryOperatorArgsType_Reg8Memory8:
        Fy_Generator_addByte(generator, instruction->as_reg8mem8.reg_id);
        Fy_Generator_addMemory(generator, &instruction->as_reg8mem8.address);
        break;
    // We flip the arguments because the memory is variable-length (we put the value/reg-id before the memory)
    case Fy_BinaryOperatorArgsType_Memory16Const:
        Fy_Generator_addWord(generator, instruction->as_mem16const.value);
        Fy_Generator_addMemory(generator, &instruction->as_mem16const.address);
        break;
    case Fy_BinaryOperatorArgsType_Memory16Reg16:
        Fy_Generator_addByte(generator, instruction->as_mem16reg16.reg_id);
        Fy_Generator_addMemory(generator, &instruction->as_mem16reg16.address);
        break;
    case Fy_BinaryOperatorArgsType_Memory8Const:
        Fy_Generator_addByte(generator, instruction->as_mem8const.value);
        Fy_Generator_addMemory(generator, &instruction->as_mem8const.address);
        break;
    case Fy_BinaryOperatorArgsType_Memory8Reg8:
        Fy_Generator_addByte(generator, instruction->as_mem8reg8.reg_id);
        Fy_Generator_addMemory(generator, &instruction->as_mem8reg8.address);
        break;
    default:
        FY_UNREACHABLE();
    }
}

static void Fy_InstructionTypeBinaryOperator_run(Fy_VM *vm, uint16_t address) {
    uint8_t info_byte = Fy_VM_getMem8(vm, address + 0);
    uint8_t type = info_byte >> 4;
    uint8_t operator = info_byte & 0x0f;
    uint16_t instruction_size = 1 + 1; // How much we need to advance

    switch (type) {
    case Fy_BinaryOperatorArgsType_Reg16Const: {
        uint8_t reg_id = Fy_VM_getMem8(vm, address + 1);
        uint16_t value = Fy_VM_getMem16(vm, address + 2);
        Fy_VM_runOperatorOnReg16(vm, operator, reg_id, value);
        instruction_size += 1 + 2;
        break;
    }
    case Fy_BinaryOperatorArgsType_Reg16Reg16: {
        uint8_t reg_id = Fy_VM_getMem8(vm, address + 1);
        uint8_t reg2_id = Fy_VM_getMem8(vm, address + 2);
        uint16_t value;

        if (!Fy_VM_getReg16(vm, reg2_id, &value))
            return;

        Fy_VM_runOperatorOnReg16(vm, operator, reg_id, value);
        instruction_size += 1 + 1;
        break;
    }
    case Fy_BinaryOperatorArgsType_Reg16Memory16: {
        uint8_t reg_id = Fy_VM_getMem8(vm, address + 1);
        uint16_t value_address;
        uint16_t value;
        uint16_t memory_param_size;

        memory_param_size = Fy_VM_readMemoryParam(vm, address + 2, &value_address);
        value = Fy_VM_getMem16(vm, value_address);

        Fy_VM_runOperatorOnReg16(vm, operator, reg_id, value);
        instruction_size += 1 + memory_param_size;
        break;
    }
    case Fy_BinaryOperatorArgsType_Reg8Const: {
        uint8_t reg_id = Fy_VM_getMem8(vm, address + 1);
        uint8_t value = Fy_VM_getMem8(vm, address + 2);
        Fy_VM_runOperatorOnReg8(vm, operator, reg_id, value);
        instruction_size += 1 + 1;
        break;
    }
    case Fy_BinaryOperatorArgsType_Reg8Reg8: {
        uint8_t reg_id = Fy_VM_getMem8(vm, address + 1);
        uint8_t reg2_id = Fy_VM_getMem8(vm, address + 2);
        uint8_t value;

        if (!Fy_VM_getReg8(vm, reg2_id, &value))
            return;

        Fy_VM_runOperatorOnReg8(vm, operator, reg_id, value);
        instruction_size += 1 + 1;
        break;
    }
    case Fy_BinaryOperatorArgsType_Reg8Memory8: {
        uint8_t reg_id = Fy_VM_getMem8(vm, address + 1);
        uint16_t value_address;
        uint8_t value;
        uint16_t memory_param_size;

        memory_param_size = Fy_VM_readMemoryParam(vm, address + 2, &value_address);
        value = Fy_VM_getMem8(vm, value_address);

        Fy_VM_runOperatorOnReg8(vm, operator, reg_id, value);
        instruction_size += 1 + memory_param_size;
        break;
    }
    case Fy_BinaryOperatorArgsType_Memory16Const: {
        uint16_t value = Fy_VM_getMem16(vm, address + 1);
        uint16_t write_address;
        uint16_t memory_param_size;

        memory_param_size = Fy_VM_readMemoryParam(vm, address + 3, &write_address);

        Fy_VM_runOperatorOnMem16(vm, operator, write_address, value);
        instruction_size += 2 + memory_param_size;
        break;
    }
    case Fy_BinaryOperatorArgsType_Memory16Reg16: {
        uint8_t reg_id = Fy_VM_getMem8(vm, address + 1);
        uint16_t value;
        uint16_t write_address;
        uint16_t memory_param_size;

        memory_param_size = Fy_VM_readMemoryParam(vm, address + 2, &write_address);
        if (!Fy_VM_getReg16(vm, reg_id, &value))
            return;

        Fy_VM_runOperatorOnMem16(vm, operator, write_address, value);
        instruction_size += 1 + memory_param_size;
        break;
    }
    case Fy_BinaryOperatorArgsType_Memory8Const: {
        uint8_t value = Fy_VM_getMem16(vm, address + 1);
        uint16_t write_address;
        uint16_t memory_param_size;

        memory_param_size = Fy_VM_readMemoryParam(vm, address + 2, &write_address);

        Fy_VM_runOperatorOnMem8(vm, operator, write_address, value);
        instruction_size += 1 + memory_param_size;
        break;
    }
    case Fy_BinaryOperatorArgsType_Memory8Reg8: {
        uint8_t reg_id = Fy_VM_getMem8(vm, address + 1);
        uint8_t value;
        uint16_t write_address;
        uint16_t memory_param_size;

        memory_param_size = Fy_VM_readMemoryParam(vm, address + 2, &write_address);
        if (!Fy_VM_getReg8(vm, reg_id, &value))
            return;

        Fy_VM_runOperatorOnMem16(vm, operator, write_address, value);
        instruction_size += 1 + memory_param_size;
        break;
    }
    default:
        Fy_VM_runtimeError(vm, Fy_RuntimeError_InvalidOpcode, "Binary operator id '%d'", type);
        return;
    }

    vm->reg_ip += instruction_size;
}

/* Type definitions */
Fy_InstructionType Fy_instructionTypeNop = {
    .opcode = 0,
    .variable_size = false,
    .additional_size = 0,
    .write_func = NULL,
    .run_func = NULL
};

Fy_InstructionType Fy_instructionTypeEndProgram = {
    .opcode = 1,
    .variable_size = false,
    .additional_size = 0,
    .write_func = NULL,
    .run_func = Fy_instructionTypeEndProgram_run
};
Fy_InstructionType Fy_instructionTypeJmp = {
    .opcode = 2,
    .variable_size = false,
    .additional_size = 2,
    .write_func = (Fy_InstructionWriteFunc)Fy_instructionTypeJmp_write,
    .run_func = Fy_instructionTypeJmp_run
};
Fy_InstructionType Fy_instructionTypeJe = {
    .opcode = 3,
    .variable_size = false,
    .additional_size = 2,
    .write_func = (Fy_InstructionWriteFunc)Fy_instructionTypeJe_write,
    .run_func = Fy_instructionTypeJe_run
};
Fy_InstructionType Fy_instructionTypeJl = {
    .opcode = 3,
    .variable_size = false,
    .additional_size = 2,
    .write_func = (Fy_InstructionWriteFunc)Fy_instructionTypeJl_write,
    .run_func = Fy_instructionTypeJl_run
};
Fy_InstructionType Fy_instructionTypeJg = {
    .opcode = 4,
    .variable_size = false,
    .additional_size = 2,
    .write_func = (Fy_InstructionWriteFunc)Fy_instructionTypeJg_write,
    .run_func = Fy_instructionTypeJg_run
};
Fy_InstructionType Fy_instructionTypePushConst = {
    .opcode = 5,
    .variable_size = false,
    .additional_size = 2,
    .write_func = (Fy_InstructionWriteFunc)Fy_instructionTypePushConst_write,
    .run_func = Fy_instructionTypePushConst_run
};
Fy_InstructionType Fy_instructionTypePushReg16 = {
    .opcode = 6,
    .variable_size = false,
    .additional_size = 1,
    .write_func = (Fy_InstructionWriteFunc)Fy_instructionTypePushReg16_write,
    .run_func = Fy_instructionTypePushReg16_run
};
Fy_InstructionType Fy_instructionTypePop = {
    .opcode = 7,
    .variable_size = false,
    .additional_size = 1,
    .write_func = (Fy_InstructionWriteFunc)Fy_instructionTypePop_write,
    .run_func = Fy_instructionTypePop_run
};
Fy_InstructionType Fy_instructionTypeCall = {
    .opcode = 8,
    .variable_size = false,
    .additional_size = 2,
    .write_func = (Fy_InstructionWriteFunc)Fy_instructionTypeCall_write,
    .run_func = Fy_instructionTypeCall_run
};
Fy_InstructionType Fy_instructionTypeRet = {
    .opcode = 9,
    .variable_size = false,
    .additional_size = 0,
    .write_func = NULL,
    .run_func = Fy_instructionTypeRet_run
};
Fy_InstructionType Fy_instructionTypeRetConst16 = {
    .opcode = 10,
    .variable_size = false,
    .additional_size = 2,
    .write_func = (Fy_InstructionWriteFunc)Fy_instructionTypeRetConst16_write,
    .run_func = Fy_instructionTypeRetConst16_run
};
Fy_InstructionType Fy_instructionTypeDebug = {
    .opcode = 11,
    .variable_size = false,
    .additional_size = 0,
    .write_func = NULL,
    .run_func = Fy_instructionTypeDebug_run
};
Fy_InstructionType Fy_instructionTypeDebugStack = {
    .opcode = 12,
    .variable_size = false,
    .additional_size = 0,
    .write_func = NULL,
    .run_func = Fy_instructionTypeDebugStack_run
};
Fy_InstructionType Fy_instructionTypeLea = {
    .opcode = 13,
    .variable_size = true,
    .getsize_func = (Fy_InstructionGetSizeFunc)Fy_instructionTypeLea_getsize,
    .write_func = (Fy_InstructionWriteFunc)Fy_instructionTypeLea_write,
    .run_func = Fy_instructionTypeLea_run
};
Fy_InstructionType Fy_instructionTypeInt = {
    .opcode = 14,
    .variable_size = false,
    .additional_size = 1,
    .write_func = (Fy_InstructionWriteFunc)Fy_instructionTypeInt_write,
    .run_func = Fy_instructionTypeInt_run
};
Fy_InstructionType Fy_instructionTypeBinaryOperator = {
    .opcode = 15,
    .variable_size = true,
    .getsize_func = (Fy_InstructionGetSizeFunc)Fy_instructionTypeBinaryOperator_getsize,
    .write_func = (Fy_InstructionWriteFunc)Fy_instructionTypeBinaryOperator_write,
    .run_func = Fy_InstructionTypeBinaryOperator_run
};

Fy_InstructionType *Fy_instructionTypes[] = {
    &Fy_instructionTypeNop,
    &Fy_instructionTypeEndProgram,
    &Fy_instructionTypeJmp,
    &Fy_instructionTypeJe,
    &Fy_instructionTypeJl,
    &Fy_instructionTypeJg,
    &Fy_instructionTypePushConst,
    &Fy_instructionTypePushReg16,
    &Fy_instructionTypePop,
    &Fy_instructionTypeCall,
    &Fy_instructionTypeRet,
    &Fy_instructionTypeRetConst16,
    &Fy_instructionTypeDebug,
    &Fy_instructionTypeDebugStack,
    &Fy_instructionTypeLea,
    &Fy_instructionTypeInt,
    &Fy_instructionTypeBinaryOperator
};
