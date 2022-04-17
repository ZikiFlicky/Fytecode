#ifndef FY_INSTRUCTION_H
#define FY_INSTRUCTION_H

#include "generator.h"

#include <inttypes.h>
#include <stdbool.h>

#define FY_INSTRUCTION_BASE Fy_Instruction _base
#define FY_INSTRUCTION_NEW(c_type, type) ((c_type*)Fy_Instruction_New(&type, sizeof(c_type)))

typedef struct Fy_VM Fy_VM;
typedef struct Fy_Parser Fy_Parser;
typedef struct Fy_ParserParseRule Fy_ParserParseRule;
typedef struct Fy_Instruction Fy_Instruction;
typedef struct Fy_InstructionType Fy_InstructionType;
typedef struct Fy_Instruction_MovReg16Const Fy_Instruction_MovReg16Const;
typedef struct Fy_Instruction_MovReg16Reg16 Fy_Instruction_MovReg16Reg16;
typedef struct Fy_Instruction_Jmp Fy_Instruction_Jmp;
typedef struct Fy_Instruction_AddReg16Const Fy_Instruction_AddReg16Const;
typedef void (*Fy_InstructionWriteFunc)(Fy_Generator*, Fy_Instruction*);
typedef void (*Fy_InstructionRunFunc)(Fy_VM *vm);

/* Stores information about an instruction */
struct Fy_InstructionType {
    uint8_t opcode;
    uint16_t additional_size;
    Fy_InstructionWriteFunc write_func;
    /* Parses and runs instruction from memory pointer on virtual machine */
    Fy_InstructionRunFunc run_func;
    bool advance_after_run;
};

/* Base instruction */
struct Fy_Instruction {
    Fy_InstructionType *type;
    Fy_ParserParseRule *parse_rule;
};

/* Inheriting instructions */
struct Fy_Instruction_MovReg16Const {
    FY_INSTRUCTION_BASE;
    uint8_t reg_id;
    uint16_t value;
};

struct Fy_Instruction_MovReg16Reg16 {
    FY_INSTRUCTION_BASE;
    uint8_t reg_id;
    uint8_t reg2_id;
};

struct Fy_Instruction_Jmp {
    FY_INSTRUCTION_BASE;
    char *name;
    uint16_t address;
};

struct Fy_Instruction_AddReg16Const {
    FY_INSTRUCTION_BASE;
    uint8_t reg_id;
    uint16_t value;
};

/* Instruction types */
extern Fy_InstructionType Fy_InstructionType_MovReg16Const;
extern Fy_InstructionType Fy_InstructionType_MovReg16Reg16;
extern Fy_InstructionType Fy_InstructionType_Debug;
extern Fy_InstructionType Fy_InstructionType_EndProgram;
extern Fy_InstructionType Fy_InstructionType_Jmp;
extern Fy_InstructionType Fy_InstructionType_AddReg16Const;

extern Fy_InstructionType *Fy_instructionTypes[6];

/* Instruction methods/functions */
Fy_Instruction *Fy_Instruction_New(Fy_InstructionType *type, size_t size);

#endif /* FY_INSTRUCTION_H */
