#include "fy.h"

/* Returns a new instruction initialized with the given type */
Fy_Instruction *Fy_Instruction_New(Fy_InstructionType *type, size_t size) {
    Fy_Instruction *instruction = malloc(size);
    instruction->type = type;
    return instruction;
}


static void Fy_InstructionType_MovReg16Const_write(Fy_Generator *generator, Fy_Instruction_MovReg16Const *instruction) {
    Fy_Generator_addByte(generator, instruction->reg_id);
    Fy_Generator_addConst16(generator, instruction->val);
}


static void Fy_InstructionType_MovReg16Reg16_write(Fy_Generator *generator, Fy_Instruction_MovReg16Reg16 *instruction) {
    Fy_Generator_addByte(generator, instruction->reg_id);
    Fy_Generator_addByte(generator, instruction->reg2_id);
}

Fy_InstructionType Fy_InstructionType_MovReg16Const = {
    .opcode = 0,
    .write_func = (Fy_InstructionWriteFunc)Fy_InstructionType_MovReg16Const_write
};

Fy_InstructionType Fy_InstructionType_MovReg16Reg16 = {
    .opcode = 1,
    .write_func = (Fy_InstructionWriteFunc)Fy_InstructionType_MovReg16Reg16_write
};

Fy_InstructionType Fy_InstructionType_Debug = {
    .opcode = 2,
    .write_func = NULL
};