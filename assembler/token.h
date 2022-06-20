#ifndef FY_TOKEN_H
#define FY_TOKEN_H

#include "../vm/registers.h"

#include <stddef.h>
#include <inttypes.h>
#include <stdbool.h>

typedef struct Fy_Parser Fy_Parser;

typedef enum Fy_InstructionArgType Fy_InstructionArgType;

typedef enum Fy_TokenType {
    Fy_TokenType_Nop = 1,
    Fy_TokenType_Mov,
    Fy_TokenType_Lea,
    Fy_TokenType_Add,
    Fy_TokenType_Sub,
    Fy_TokenType_And,
    Fy_TokenType_Or,
    Fy_TokenType_Xor,
    Fy_TokenType_Cmp,
    Fy_TokenType_Debug,
    Fy_TokenType_DebugStack,
    Fy_TokenType_Jmp,
    Fy_TokenType_Je,
    Fy_TokenType_Jne,
    Fy_TokenType_Jz,
    Fy_TokenType_Jnz,
    Fy_TokenType_Jb,
    Fy_TokenType_Jbe,
    Fy_TokenType_Ja,
    Fy_TokenType_Jae,
    Fy_TokenType_Jl,
    Fy_TokenType_Jle,
    Fy_TokenType_Jg,
    Fy_TokenType_Jge,
    Fy_TokenType_Call,
    Fy_TokenType_Ret,
    Fy_TokenType_Push,
    Fy_TokenType_Pop,
    Fy_TokenType_Int,
    Fy_TokenType_End,
    Fy_TokenType_Ax,
    Fy_TokenType_Bx,
    Fy_TokenType_Cx,
    Fy_TokenType_Dx,
    Fy_TokenType_Sp,
    Fy_TokenType_Bp,
    Fy_TokenType_Ah,
    Fy_TokenType_Al,
    Fy_TokenType_Bh,
    Fy_TokenType_Bl,
    Fy_TokenType_Ch,
    Fy_TokenType_Cl,
    Fy_TokenType_Dh,
    Fy_TokenType_Dl,
    Fy_TokenType_Proc,
    Fy_TokenType_Endp,
    Fy_TokenType_Symbol,
    Fy_TokenType_Const,
    Fy_TokenType_Newline,
    Fy_TokenType_Colon,
    Fy_TokenType_LeftBracket,
    Fy_TokenType_RightBracket,
    Fy_TokenType_LeftParen,
    Fy_TokenType_RightParen,
    Fy_TokenType_Byte,
    Fy_TokenType_Word,
    Fy_TokenType_Plus,
    Fy_TokenType_Minus,
    Fy_TokenType_Star,
    Fy_TokenType_Slash,
    Fy_TokenType_EqualSign,
    Fy_TokenType_Comma,
    Fy_TokenType_Data,
    Fy_TokenType_Code,
    Fy_TokenType_Eb,
    Fy_TokenType_Ew,
    Fy_TokenType_Dup,
    Fy_TokenType_String,
    Fy_TokenType_Char
} Fy_TokenType;

typedef struct Fy_Token {
    Fy_TokenType type;
    char *start;
    size_t length;
} Fy_Token;

int8_t Fy_Token_toConst8(Fy_Token *token, Fy_Parser *parser);
int16_t Fy_Token_toConst16(Fy_Token *token, Fy_Parser *parser);
bool Fy_TokenType_isReg8(Fy_TokenType type);
bool Fy_TokenType_isReg16(Fy_TokenType type);
Fy_Reg8 Fy_TokenType_toReg8(Fy_TokenType type);
Fy_Reg16 Fy_TokenType_toReg16(Fy_TokenType type);
char *Fy_Token_toLowercaseCStr(Fy_Token *token);

#endif /* FY_TOKEN_H */
