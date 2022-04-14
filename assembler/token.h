#ifndef FY_TOKEN_H
#define FY_TOKEN_H

#include <stddef.h>
#include <inttypes.h>
#include <stdbool.h>

typedef struct Fy_Parser Fy_Parser;

typedef enum Fy_ParserReg16 Fy_ParserReg16;
typedef enum Fy_ParserArgType Fy_ParserArgType;

typedef enum Fy_TokenType {
    Fy_TokenType_Mov = 1,
    Fy_TokenType_Ax,
    Fy_TokenType_Bx,
    Fy_TokenType_Const,
    Fy_TokenType_Newline
} Fy_TokenType;

typedef struct Fy_Token {
    Fy_TokenType type;
    char *start;
    size_t length;
} Fy_Token;

extern Fy_TokenType Fy_reg16Tokens[];

int16_t Fy_Token_toConst16(Fy_Token *token, Fy_Parser *parser);
Fy_ParserReg16 Fy_TokenType_toReg16(Fy_TokenType type);
bool Fy_TokenType_isPossibleArg(Fy_TokenType token_type, Fy_ParserArgType arg_type);

#endif /* FY_TOKEN_H */
