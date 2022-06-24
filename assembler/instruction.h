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
typedef enum Fy_BinaryOperatorArgsType Fy_BinaryOperatorArgsType;
typedef enum Fy_UnaryOperatorArgsType Fy_UnaryOperatorArgsType;
typedef enum Fy_BinaryOperator Fy_BinaryOperator;
typedef struct Fy_InstructionType Fy_InstructionType;
typedef struct Fy_Instruction_OpLabel Fy_Instruction_OpLabel;
typedef struct Fy_Instruction_OpConst8 Fy_Instruction_OpConst8;
typedef struct Fy_Instruction_OpConst16 Fy_Instruction_OpConst16;
typedef struct Fy_Instruction_OpReg8 Fy_Instruction_OpReg8;
typedef struct Fy_Instruction_OpReg16 Fy_Instruction_OpReg16;
typedef struct Fy_Instruction_OpMem Fy_Instruction_OpMem;
typedef struct Fy_Instruction_OpReg16Mem Fy_Instruction_OpReg16Mem;
typedef struct Fy_Instruction_BinaryOperator Fy_Instruction_BinaryOperator;
typedef struct Fy_Instruction_UnaryOperator Fy_Instruction_UnaryOperator;
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
    const Fy_InstructionType *type;
    const Fy_ParserParseRule *parse_rule;
    Fy_ParserState start_state;
    /* Stores information needed for code labels */
    uint16_t code_offset, size;
};

enum Fy_BinaryOperatorArgsType {
    Fy_BinaryOperatorArgsType_Reg16Const = 1,
    Fy_BinaryOperatorArgsType_Reg16Reg16,
    Fy_BinaryOperatorArgsType_Reg16Memory16,
    Fy_BinaryOperatorArgsType_Reg8Const,
    Fy_BinaryOperatorArgsType_Reg8Reg8,
    Fy_BinaryOperatorArgsType_Reg8Memory8,
    Fy_BinaryOperatorArgsType_Memory16Const,
    Fy_BinaryOperatorArgsType_Memory16Reg16,
    Fy_BinaryOperatorArgsType_Memory8Const,
    Fy_BinaryOperatorArgsType_Memory8Reg8
};

enum Fy_UnaryOperatorArgsType {
    Fy_UnaryOperatorArgsType_Reg16 = 1,
    Fy_UnaryOperatorArgsType_Mem16,
    Fy_UnaryOperatorArgsType_Reg8,
    Fy_UnaryOperatorArgsType_Mem8
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

struct Fy_Instruction_OpConst8 {
    FY_INSTRUCTION_BASE;
    uint8_t value;
};

struct Fy_Instruction_OpConst16 {
    FY_INSTRUCTION_BASE;
    uint16_t value;
};

struct Fy_Instruction_OpReg8 {
    FY_INSTRUCTION_BASE;
    uint8_t reg_id;
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
    Fy_AST *address_ast;
    Fy_InlineValue value;
    uint8_t reg_id;
};

struct Fy_Instruction_OpMem8Reg8 {
    FY_INSTRUCTION_BASE;
    Fy_AST *address_ast;
    Fy_InlineValue value;
    uint8_t reg_id;
};

struct Fy_Instruction_BinaryOperator {
    FY_INSTRUCTION_BASE;
    Fy_BinaryOperatorArgsType type;
    Fy_BinaryOperator operator;
    union {
        struct {
            uint8_t reg_id;
            uint16_t value;
        } as_reg16const;
        struct {
            uint8_t reg_id;
            uint8_t reg2_id;
        } as_reg16reg16;
        struct {
            uint8_t reg_id;
            Fy_AST *ast;
            Fy_InlineValue address;
        } as_reg16mem16;

        struct {
            uint8_t reg_id;
            uint8_t value;
        } as_reg8const;
        struct {
            uint8_t reg_id;
            uint8_t reg2_id;
        } as_reg8reg8;
        struct {
            uint8_t reg_id;
            Fy_AST *ast;
            Fy_InlineValue address;
        } as_reg8mem8;

        struct {
            Fy_AST *ast;
            Fy_InlineValue address;
            uint16_t value;
        } as_mem16const;
        struct {
            Fy_AST *ast;
            Fy_InlineValue address;
            uint8_t reg_id;
        } as_mem16reg16;

        struct {
            Fy_AST *ast;
            Fy_InlineValue address;
            uint8_t value;
        } as_mem8const;
        struct {
            Fy_AST *ast;
            Fy_InlineValue address;
            uint8_t reg_id;
        } as_mem8reg8;
    };
};

struct Fy_Instruction_UnaryOperator {
    FY_INSTRUCTION_BASE;
    Fy_UnaryOperatorArgsType type;
    Fy_UnaryOperator operator;
    union {
        uint8_t as_reg8;
        uint8_t as_reg16;
        struct {
            Fy_AST *ast;
            Fy_InlineValue address;
        } as_mem8;
        struct {
            Fy_AST *ast;
            Fy_InlineValue address;
        } as_mem16;
    };
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
extern Fy_InstructionType Fy_instructionTypeJne;
extern Fy_InstructionType Fy_instructionTypeJb;
extern Fy_InstructionType Fy_instructionTypeJbe;
extern Fy_InstructionType Fy_instructionTypeJa;
extern Fy_InstructionType Fy_instructionTypeJae;
extern Fy_InstructionType Fy_instructionTypeJl;
extern Fy_InstructionType Fy_instructionTypeJle;
extern Fy_InstructionType Fy_instructionTypeJg;
extern Fy_InstructionType Fy_instructionTypeJge;
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
extern Fy_InstructionType Fy_instructionTypeMovMem8Reg8;
extern Fy_InstructionType Fy_instructionTypeInt;
extern Fy_InstructionType Fy_instructionTypeMulReg16;
extern Fy_InstructionType Fy_instructionTypeMulReg8;
extern Fy_InstructionType Fy_instructionTypeImulReg16;
extern Fy_InstructionType Fy_instructionTypeImulReg8;
extern Fy_InstructionType Fy_instructionTypeDivReg16;
extern Fy_InstructionType Fy_instructionTypeDivReg8;
extern Fy_InstructionType Fy_instructionTypeIdivReg16;
extern Fy_InstructionType Fy_instructionTypeIdivReg8;
extern Fy_InstructionType Fy_instructionTypeCmpReg8Const;
extern Fy_InstructionType Fy_instructionTypeCmpReg8Reg8;
extern Fy_InstructionType Fy_instructionTypeAddReg8Const;
extern Fy_InstructionType Fy_instructionTypeAddReg8Reg8;
extern Fy_InstructionType Fy_instructionTypeBinaryOperator;
extern Fy_InstructionType Fy_instructionTypeUnaryOperator;
extern Fy_InstructionType Fy_instructionTypeCbw;

extern Fy_InstructionType* const Fy_instructionTypes[34];

/* Instruction methods/functions */
Fy_Instruction *Fy_Instruction_New(const Fy_InstructionType *type, size_t size);
void Fy_Instruction_Delete(Fy_Instruction *instruction);

#endif /* FY_INSTRUCTION_H */
