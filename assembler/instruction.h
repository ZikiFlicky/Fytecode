#ifndef FY_INSTRUCTION_H
#define FY_INSTRUCTION_H

#include "generator.h"
#include "parser.h"

#include <inttypes.h>
#include <stdbool.h>

#define FY_INSTRUCTION_BASE Fy_Instruction _base
#define FY_INSTRUCTION_NEW(c_type, type) ((c_type*)Fy_Instruction_New(&type, sizeof(c_type)))

typedef struct Fy_VM Fy_VM;
typedef struct Fy_Parser Fy_Parser;
typedef struct Fy_ParserParseRule Fy_ParserParseRule;
typedef struct Fy_Instruction Fy_Instruction;
typedef struct Fy_InstructionType Fy_InstructionType;
typedef struct Fy_Instruction_OpReg8Const Fy_Instruction_OpReg8Const;
typedef struct Fy_Instruction_OpReg8Reg8 Fy_Instruction_OpReg8Reg8;
typedef struct Fy_Instruction_OpReg16Const Fy_Instruction_OpReg16Const;
typedef struct Fy_Instruction_OpReg16Reg16 Fy_Instruction_OpReg16Reg16;
typedef struct Fy_Instruction_OpLabel Fy_Instruction_OpLabel;
typedef struct Fy_Instruction_PushConst Fy_Instruction_PushConst;
typedef struct Fy_Instruction_OpReg16 Fy_Instruction_OpReg16;
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
    Fy_ParserState start_state;
};

/* Inheriting instructions */
struct Fy_Instruction_OpReg8Const {
    FY_INSTRUCTION_BASE;
    uint8_t reg_id;
    uint8_t value;
};

struct Fy_Instruction_OpReg8Reg8 {
    FY_INSTRUCTION_BASE;
    uint8_t reg_id;
    uint8_t reg2_id;
};

struct Fy_Instruction_OpReg16Const {
    FY_INSTRUCTION_BASE;
    uint8_t reg_id;
    uint16_t value;
};

struct Fy_Instruction_OpReg16Reg16 {
    FY_INSTRUCTION_BASE;
    uint8_t reg_id;
    uint8_t reg2_id;
};

struct Fy_Instruction_OpLabel {
    FY_INSTRUCTION_BASE;
    char *name;
    uint16_t address;
};

struct Fy_Instruction_PushConst {
    FY_INSTRUCTION_BASE;
    uint16_t value;
};

struct Fy_Instruction_OpReg16 {
    FY_INSTRUCTION_BASE;
    uint8_t reg_id;
};

/* Instruction types */
extern Fy_InstructionType Fy_InstructionType_MovReg16Const;
extern Fy_InstructionType Fy_InstructionType_MovReg16Reg16;
extern Fy_InstructionType Fy_InstructionType_Debug;
extern Fy_InstructionType Fy_InstructionType_EndProgram;
extern Fy_InstructionType Fy_InstructionType_AddReg16Const;
extern Fy_InstructionType Fy_InstructionType_AddReg16Reg16;
extern Fy_InstructionType Fy_InstructionType_SubReg16Const;
extern Fy_InstructionType Fy_InstructionType_SubReg16Reg16;
extern Fy_InstructionType Fy_InstructionType_CmpReg16Const;
extern Fy_InstructionType Fy_InstructionType_CmpReg16Reg16;
extern Fy_InstructionType Fy_InstructionType_Jmp;
extern Fy_InstructionType Fy_InstructionType_Je;
extern Fy_InstructionType Fy_InstructionType_Jl;
extern Fy_InstructionType Fy_InstructionType_Jg;
extern Fy_InstructionType Fy_InstructionType_PushConst;
extern Fy_InstructionType Fy_InstructionType_PushReg16;
extern Fy_InstructionType Fy_InstructionType_Pop;
extern Fy_InstructionType Fy_InstructionType_MovReg8Const;
extern Fy_InstructionType Fy_InstructionType_MovReg8Reg8;
extern Fy_InstructionType Fy_InstructionType_Call;

extern Fy_InstructionType *Fy_instructionTypes[20];

/* Instruction methods/functions */
Fy_Instruction *Fy_Instruction_New(Fy_InstructionType *type, size_t size);

#endif /* FY_INSTRUCTION_H */
