#ifndef FY_PARSER_H
#define FY_PARSER_H

#include "lexer.h"

#include <stddef.h>
#include <inttypes.h>

typedef struct Fy_ParserInstruction Fy_ParserInstruction;
typedef struct Fy_Parser Fy_Parser;

/* Parse function declarations */
Fy_ParserInstruction *Fy_ParseMovReg16Const(Fy_Parser *parser, Fy_Token *token_arg1, Fy_Token *token_arg2);
Fy_ParserInstruction *Fy_ParseMovReg16Reg16(Fy_Parser *parser, Fy_Token *token_arg1, Fy_Token *token_arg2);

typedef struct Fy_ParserState {
    char *stream;
    size_t line, column;
} Fy_ParserState;

typedef enum Fy_ParserInstructionType {
    Fy_ParserInstructionType_MovReg8Const = 1,
    Fy_ParserInstructionType_MovReg8Mem,
    Fy_ParserInstructionType_MovReg16Const,
    Fy_ParserInstructionType_MovReg16Mem,
    Fy_ParserInstructionType_MovReg16Reg16
} Fy_ParserInstructionType;

typedef enum Fy_ParserReg16 {
    Fy_ParserReg16_Ax = 1,
    Fy_ParserReg16_Bx
} Fy_ParserReg16;

typedef struct Fy_ParserInstruction {
    Fy_ParserInstructionType type;
    union {
        struct {
            Fy_ParserReg16 reg_id;
            uint8_t const8;
        } mov_reg8_const;
        struct {
            Fy_ParserReg16 reg_id;
            uint16_t const16;
        } mov_reg16_const;
        struct {
            Fy_ParserReg16 reg_id;
            Fy_ParserReg16 reg2_id;
        } mov_reg16_reg16;
    };
} Fy_ParserInstruction;

typedef enum Fy_ParserError {
    Fy_ParserError_UnexpectedToken = 1,
    Fy_ParserError_UnexpectedEof,
    Fy_ParserError_ExpectedReg,
    Fy_ParserError_ConstTooBig,
    Fy_ParserError_ExpectedNewline,
    Fy_ParserError_InvalidInstructionParams
} Fy_ParserError;

typedef struct Fy_Parser {
    Fy_Lexer *lexer;
    Fy_Token token;

    size_t amount_used, amount_allocated;
    Fy_ParserInstruction **instructions;
} Fy_Parser;


typedef enum Fy_ParserArgType {
    Fy_ParserArgType_Reg16 = 1,
    Fy_ParserArgType_Reg8,
    Fy_ParserArgType_Constant
} Fy_ParserArgType;

// FIXME: Don't define here, just declare

// TODO: This currently only supports two parameters
typedef struct Fy_ParserParseRule {
    Fy_TokenType start_token; /* Type of token to be expected at start */
    struct {
        Fy_ParserArgType type;
        Fy_TokenType *possible_tokens; /* All of the possible tokens to be found after */
    } arg1, arg2;
    /* A function that always returns a valid ParserInstruction based on the given token */
    Fy_ParserInstruction *(*func)(Fy_Parser*, Fy_Token*, Fy_Token*);
} Fy_ParserParseRule;

extern Fy_ParserParseRule *Fy_parserRules[];

void Fy_Parser_Init(Fy_Lexer *lexer, Fy_Parser *out);
void Fy_Parser_parseAll(Fy_Parser *parser);
void Fy_Parser_error(Fy_Parser *parser, Fy_ParserError error);
void Fy_Parser_generateToFile(Fy_Parser *parser, char *filename);
void Fy_Parser_logParsed(Fy_Parser *parser); /* NOTE: Debug function */

#endif /* FY_PARSER_H */
