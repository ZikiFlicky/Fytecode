#ifndef FY_INSTRUCTION_H
#define FY_INSTRUCTION_H

#include "generator.h"

#include <inttypes.h>

#define FY_INSTRUCTION_BASE Fy_Instruction _base
#define FY_INSTRUCTION_NEW(c_type, type) ((c_type*)Fy_Instruction_New(&type, sizeof(c_type)))

typedef struct Fy_Parser Fy_Parser;
typedef struct Fy_Instruction Fy_Instruction;
typedef struct Fy_InstructionType Fy_InstructionType;
typedef struct Fy_Instruction_MovReg16Const Fy_Instruction_MovReg16Const;
typedef struct Fy_Instruction_MovReg16Reg16 Fy_Instruction_MovReg16Reg16;
typedef void (*Fy_InstructionWriteFunc)(Fy_Generator*, Fy_Instruction*);

/* Stores information about an instruction */
struct Fy_InstructionType {
    uint8_t opcode;
    Fy_InstructionWriteFunc write_func;
};

/* Base instruction */
struct Fy_Instruction {
    Fy_InstructionType *type;
};

/* Inheriting instructions */
struct Fy_Instruction_MovReg16Const {
    FY_INSTRUCTION_BASE;
    uint8_t reg_id;
    uint16_t val;
};

struct Fy_Instruction_MovReg16Reg16 {
    FY_INSTRUCTION_BASE;
    uint8_t reg_id;
    uint8_t reg2_id;
};

/* Instruction types */
extern Fy_InstructionType Fy_InstructionType_MovReg16Const;
extern Fy_InstructionType Fy_InstructionType_MovReg16Reg16;
extern Fy_InstructionType Fy_InstructionType_Debug;

/* Instruction methods/functions */
Fy_Instruction *Fy_Instruction_New(Fy_InstructionType *type, size_t size);

#endif /* FY_INSTRUCTION_H */
