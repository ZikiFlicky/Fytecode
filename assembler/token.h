#ifndef FY_TOKEN_H
#define FY_TOKEN_H

#include "../interpreter/registers.h"

#include <stddef.h>
#include <inttypes.h>
#include <stdbool.h>

typedef struct Fy_Parser Fy_Parser;

typedef enum Fy_ParserArgType Fy_ParserArgType;

typedef enum Fy_TokenType {
    Fy_TokenType_Mov = 1,
    Fy_TokenType_Add,
    Fy_TokenType_Sub,
    Fy_TokenType_Cmp,
    Fy_TokenType_Debug,
    Fy_TokenType_Jmp,
    Fy_TokenType_Je,
    Fy_TokenType_Jl,
    Fy_TokenType_Jg,
    Fy_TokenType_Push,
    Fy_TokenType_Pop,
    Fy_TokenType_End,
    Fy_TokenType_Ax,
    Fy_TokenType_Bx,
    Fy_TokenType_Cx,
    Fy_TokenType_Dx,
    Fy_TokenType_Label,
    Fy_TokenType_Const,
    Fy_TokenType_Newline,
    Fy_TokenType_Colon
} Fy_TokenType;

typedef struct Fy_Token {
    Fy_TokenType type;
    char *start;
    size_t length;
} Fy_Token;

extern Fy_TokenType Fy_reg16Tokens[];

int16_t Fy_Token_toConst16(Fy_Token *token, Fy_Parser *parser);
Fy_Reg16 Fy_TokenType_toReg16(Fy_TokenType type);
char *Fy_Token_toLowercaseCStr(Fy_Token *token);
bool Fy_TokenType_isPossibleArg(Fy_TokenType token_type, Fy_ParserArgType arg_type);

#endif /* FY_TOKEN_H */
