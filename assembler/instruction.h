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
typedef struct Fy_Instruction_OpConst16 Fy_Instruction_OpConst16;
typedef struct Fy_Instruction_OpReg16 Fy_Instruction_OpReg16;
typedef struct Fy_Instruction_OpReg16Mem Fy_Instruction_OpReg16Mem;
typedef struct Fy_Instruction_OpMemReg16 Fy_Instruction_OpMemReg16;
typedef struct Fy_Instruction_OpMem Fy_Instruction_OpMem;
typedef struct Fy_Instruction_OpVarReg16 Fy_Instruction_OpVarReg16;
typedef struct Fy_Instruction_OpVarConst16 Fy_Instruction_OpVarConst16;
typedef void (*Fy_InstructionWriteFunc)(Fy_Generator*, Fy_Instruction*);
typedef uint16_t (*Fy_InstructionGetSizeFunc)(Fy_Instruction*);
typedef void (*Fy_InstructionRunFunc)(Fy_VM*, uint16_t);

/* Stores information about an instruction */
struct Fy_InstructionType {
    uint8_t opcode;
    bool variable_size;
    union {
        Fy_InstructionGetSizeFunc getsize_func;
        uint16_t additional_size;
    };
    Fy_InstructionWriteFunc write_func;
    /* Parses and runs instruction from memory pointer on virtual machine */
    Fy_InstructionRunFunc run_func;
};

/* Base instruction */
struct Fy_Instruction {
    Fy_InstructionType *type;
    Fy_ParserParseRule *parse_rule;
    Fy_ParserState start_state;
    /* Stores information needed for code labels */
    uint16_t code_offset, size;
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
    /* The next instruction offset */
    size_t instruction_offset;
    uint16_t address;
};

struct Fy_Instruction_OpConst16 {
    FY_INSTRUCTION_BASE;
    uint16_t value;
};

struct Fy_Instruction_OpReg16 {
    FY_INSTRUCTION_BASE;
    uint8_t reg_id;
};

struct Fy_Instruction_OpReg16Mem {
    FY_INSTRUCTION_BASE;
    uint8_t reg_id;
    Fy_AST *address_ast;
    Fy_InlineValue value;
};

struct Fy_Instruction_OpMemReg16 {
    FY_INSTRUCTION_BASE;
    uint8_t reg_id;
    Fy_AST *address_ast;
    Fy_InlineValue value;
};

/* Instruction types */
extern Fy_InstructionType Fy_instructionTypeNop;
extern Fy_InstructionType Fy_instructionTypeMovReg16Const;
extern Fy_InstructionType Fy_instructionTypeMovReg16Reg16;
extern Fy_InstructionType Fy_instructionTypeEndProgram;
extern Fy_InstructionType Fy_instructionTypeAddReg16Const;
extern Fy_InstructionType Fy_instructionTypeAddReg16Reg16;
extern Fy_InstructionType Fy_instructionTypeSubReg16Const;
extern Fy_InstructionType Fy_instructionTypeSubReg16Reg16;
extern Fy_InstructionType Fy_instructionTypeCmpReg16Const;
extern Fy_InstructionType Fy_instructionTypeCmpReg16Reg16;
extern Fy_InstructionType Fy_instructionTypeJmp;
extern Fy_InstructionType Fy_instructionTypeJe;
extern Fy_InstructionType Fy_instructionTypeJl;
extern Fy_InstructionType Fy_instructionTypeJg;
extern Fy_InstructionType Fy_instructionTypePushConst;
extern Fy_InstructionType Fy_instructionTypePushReg16;
extern Fy_InstructionType Fy_instructionTypePop;
extern Fy_InstructionType Fy_instructionTypeMovReg8Const;
extern Fy_InstructionType Fy_instructionTypeMovReg8Reg8;
extern Fy_InstructionType Fy_instructionTypeCall;
extern Fy_InstructionType Fy_instructionTypeRet;
extern Fy_InstructionType Fy_instructionTypeRetConst16;
extern Fy_InstructionType Fy_instructionTypeDebug;
extern Fy_InstructionType Fy_instructionTypeDebugStack;
extern Fy_InstructionType Fy_instructionTypeMovReg16Mem;
extern Fy_InstructionType Fy_instructionTypeLea;
extern Fy_InstructionType Fy_instructionTypeMovMemReg16;

extern Fy_InstructionType *Fy_instructionTypes[27];

/* Instruction methods/functions */
Fy_Instruction *Fy_Instruction_New(Fy_InstructionType *type, size_t size);

#endif /* FY_INSTRUCTION_H */
