#ifndef FY_PARSER_H
#define FY_PARSER_H

#include "lexer.h"
#include "generator.h"
#include "instruction.h"

#include <stddef.h>
#include <inttypes.h>

typedef struct Fy_ParserState {
    char *stream;
    size_t line, column;
} Fy_ParserState;

typedef enum Fy_ParserReg16 {
    Fy_ParserReg16_Ax = 1,
    Fy_ParserReg16_Bx
} Fy_ParserReg16;

typedef enum Fy_ParserError {
    Fy_ParserError_UnexpectedToken = 1,
    Fy_ParserError_UnexpectedEof,
    Fy_ParserError_ExpectedReg,
    Fy_ParserError_ConstTooBig,
    Fy_ParserError_ExpectedNewline,
    Fy_ParserError_InvalidInstruction
} Fy_ParserError;

typedef struct Fy_Parser {
    Fy_Lexer *lexer;
    Fy_Token token;

    size_t amount_used, amount_allocated;
    Fy_Instruction **instructions;
} Fy_Parser;


typedef enum Fy_ParserArgType {
    Fy_ParserArgType_Reg16 = 1,
    Fy_ParserArgType_Reg8,
    Fy_ParserArgType_Constant
} Fy_ParserArgType;

typedef enum Fy_ParserParseRuleType {
    Fy_ParserParseRuleType_NoParams = 1,
    Fy_ParserParseRuleType_OneParam,
    Fy_ParserParseRuleType_TwoParams
} Fy_ParserParseRuleType;

// TODO: This currently only supports two parameters
typedef struct Fy_ParserParseRule {
    Fy_ParserParseRuleType type;
    Fy_TokenType start_token; /* Type of token to be expected at start */
    struct {
        Fy_ParserArgType type;
        Fy_TokenType *possible_tokens; /* All of the possible tokens to be found after */
    } arg1, arg2;
    /* A function that always returns a valid ParserInstruction based on the given token */
    union {
        Fy_Instruction *(*func_no_params)(Fy_Parser*);
        Fy_Instruction *(*func_one_param)(Fy_Parser*);
        Fy_Instruction *(*func_two_params)(Fy_Parser*, Fy_Token*, Fy_Token*);
    };
} Fy_ParserParseRule;

extern Fy_ParserParseRule *Fy_parserRules[];

void Fy_Parser_Init(Fy_Lexer *lexer, Fy_Parser *out);
void Fy_Parser_parseAll(Fy_Parser *parser);
void Fy_Parser_error(Fy_Parser *parser, Fy_ParserError error);
void Fy_Parser_generateBytecode(Fy_Parser *parser, Fy_Generator *out);
void Fy_Parser_generateToFile(Fy_Parser *parser, char *filename);
void Fy_Parser_logParsed(Fy_Parser *parser); /* NOTE: Debug function */

#endif /* FY_PARSER_H */
