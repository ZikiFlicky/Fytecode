#ifndef FY_PARSER_H
#define FY_PARSER_H

#include "lexer.h"
#include "generator.h"
#include "symbolmap.h"
#include "ast.h"

#include <stddef.h>
#include <inttypes.h>

#define FY_MACRO_DEPTH 128

typedef struct Fy_ParserState Fy_ParserState;
typedef enum Fy_ParserError Fy_ParserError;
typedef struct Fy_Parser Fy_Parser;
typedef enum Fy_InstructionArgType Fy_InstructionArgType;
typedef struct Fy_InstructionArg Fy_InstructionArg;
typedef enum Fy_ParserParseRuleType Fy_ParserParseRuleType;
typedef struct Fy_AST Fy_AST;
typedef struct Fy_ParserParseRule Fy_ParserParseRule;
typedef void (*Fy_InstructionProcessFunc)(Fy_Parser*, Fy_Instruction*);
typedef void (*Fy_InstructionProcessLabelFunc)(Fy_Instruction*, Fy_Parser*);

struct Fy_ParserState {
    char *stream;
    size_t line, column;
    // Store where the token is stored
    Fy_Macro macros[FY_MACRO_DEPTH];
    size_t amount_macros;
};

enum Fy_ParserError {
    Fy_ParserError_UnexpectedToken = 1,
    Fy_ParserError_UnexpectedEof,
    Fy_ParserError_ExpectedReg,
    Fy_ParserError_ConstTooBig,
    Fy_ParserError_ExpectedNewline,
    Fy_ParserError_InvalidInstruction,
    Fy_ParserError_SyntaxError,
    Fy_ParserError_CannotOpenFileForWrite,
    Fy_ParserError_SymbolNotFound,
    Fy_ParserError_UnexpectedSymbol,
    Fy_ParserError_ExpectedDifferentToken,
    Fy_ParserError_SymbolNotCode,
    Fy_ParserError_SymbolNotVariable,
    Fy_ParserError_SymbolAlreadyDefined,
    Fy_ParserError_RecursiveMacro,
    Fy_ParserError_MaxMacroDepthReached
};

struct Fy_Parser {
    Fy_Lexer *lexer;
    Fy_Token token;
    uint8_t *data_part;
    uint16_t data_allocated, data_size;
    uint16_t code_size;

    size_t amount_used, amount_allocated;
    Fy_Instruction **instructions;

    Fy_Symbolmap symmap;

    Fy_MacroEvalInstance macros[FY_MACRO_DEPTH];
    size_t amount_macros;
};

enum Fy_InstructionArgType {
    Fy_InstructionArgType_Reg16 = 1,
    Fy_InstructionArgType_Reg8,
    Fy_InstructionArgType_Constant,
    Fy_InstructionArgType_Label,
    Fy_InstructionArgType_Memory
};

struct Fy_InstructionArg {
    Fy_InstructionArgType type;
    union {
        uint8_t as_reg16;
        uint8_t as_reg8;
        uint16_t as_const;
        char *as_label;
        Fy_AST *as_memory;
    };
};

enum Fy_ParserParseRuleType {
    Fy_ParserParseRuleType_NoParams = 1,
    Fy_ParserParseRuleType_OneParam,
    Fy_ParserParseRuleType_TwoParams
};

struct Fy_ParserParseRule {
    Fy_ParserParseRuleType type;
    Fy_TokenType start_token; /* Type of token to be expected at start */
    Fy_InstructionArgType arg1_type, arg2_type;
    /* A function that always returns a valid ParserInstruction based on the given token */
    union {
        Fy_Instruction *(*func_no_params)(Fy_Parser*);
        Fy_Instruction *(*func_one_param)(Fy_Parser*, Fy_InstructionArg*);
        Fy_Instruction *(*func_two_params)(Fy_Parser*, Fy_InstructionArg*, Fy_InstructionArg*);
    };
    /* Process instruction after full file parsing */
    Fy_InstructionProcessFunc process_func;
    Fy_InstructionProcessLabelFunc process_label_func;
};

extern Fy_ParserParseRule *Fy_parserRules[];

void Fy_Parser_Init(Fy_Lexer *lexer, Fy_Parser *out);
void Fy_Parser_Destruct(Fy_Parser *parser);
void Fy_Parser_parseAll(Fy_Parser *parser);
void Fy_Parser_error(Fy_Parser *parser, Fy_ParserError error, Fy_ParserState *state, char *additional, ...);
void Fy_Parser_generateBytecode(Fy_Parser *parser, Fy_Generator *out);
void Fy_Parser_generateToFile(Fy_Parser *parser, char *filename);
void Fy_Parser_logParsed(Fy_Parser *parser); /* NOTE: Debug function */
uint16_t Fy_Parser_getCodeOffsetByInstructionIndex(Fy_Parser *parser, size_t index);

#endif /* FY_PARSER_H */
