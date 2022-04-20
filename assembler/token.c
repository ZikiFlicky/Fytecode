#include "fy.h"

Fy_TokenType Fy_reg16Tokens[] = {
    Fy_TokenType_Ax,
    Fy_TokenType_Bx,
    Fy_TokenType_Cx,
    Fy_TokenType_Dx
};

Fy_TokenType Fy_reg8Tokens[] = {
    Fy_TokenType_Ah,
    Fy_TokenType_Al,
    Fy_TokenType_Bh,
    Fy_TokenType_Bl,
    Fy_TokenType_Ch,
    Fy_TokenType_Cl,
    Fy_TokenType_Dh,
    Fy_TokenType_Dl
};

Fy_Reg16 Fy_TokenType_toReg16(Fy_TokenType type) {
    switch (type) {
    case Fy_TokenType_Ax:
        return Fy_Reg16_Ax;
    case Fy_TokenType_Bx:
        return Fy_Reg16_Bx;
    case Fy_TokenType_Cx:
        return Fy_Reg16_Cx;
    case Fy_TokenType_Dx:
        return Fy_Reg16_Dx;
    default:
        FY_UNREACHABLE();
    }
}

Fy_Reg8 Fy_TokenType_toReg8(Fy_TokenType type) {
    switch (type) {
    case Fy_TokenType_Ah:
        return Fy_Reg8_Ah;
    case Fy_TokenType_Al:
        return Fy_Reg8_Al;
    case Fy_TokenType_Bh:
        return Fy_Reg8_Bh;
    case Fy_TokenType_Bl:
        return Fy_Reg8_Bl;
    case Fy_TokenType_Ch:
        return Fy_Reg8_Ch;
    case Fy_TokenType_Cl:
        return Fy_Reg8_Cl;
    case Fy_TokenType_Dh:
        return Fy_Reg8_Dh;
    case Fy_TokenType_Dl:
        return Fy_Reg8_Dl;
    default:
        FY_UNREACHABLE();
    }
}

static int16_t Fy_Token_toConst(Fy_Token *token, uint8_t width, Fy_Parser *parser) {
    bool is_neg;
    size_t i = 0;
    size_t positive = 0;

    assert(token->type == Fy_TokenType_Const);

    if (token->start[0] == '-') {
        is_neg = true;
        ++i;
    } else {
        is_neg = false;
    }

    do {
        positive *= 10;
        positive += token->start[i] - '0';

        if (positive >= (size_t)(1 << (is_neg ? width - 1 : width)))
            Fy_Parser_error(parser, Fy_ParserError_ConstTooBig, NULL);

        ++i;
    } while (i < token->length);

    return (int16_t)((is_neg ? -1 : 1) * positive);
}

/* Convert Fy_Token with type Fy_TokenType_Const to 16-bit integer */
int16_t Fy_Token_toConst16(Fy_Token *token, Fy_Parser *parser) {
    return Fy_Token_toConst(token, 16, parser);
}

/* Convert Fy_Token with type Fy_TokenType_Const to 8-bit integer */
int8_t Fy_Token_toConst8(Fy_Token *token, Fy_Parser *parser) {
    return (int8_t)Fy_Token_toConst(token, 8, parser);
}

/* Returns whether the token is of type `arg_type` given */
bool Fy_TokenType_isPossibleArg(Fy_TokenType token_type, Fy_ParserArgType arg_type) {
    switch (arg_type) {
    case Fy_ParserArgType_Reg16:
        for (size_t i = 0; i < sizeof(Fy_reg16Tokens) / sizeof(Fy_TokenType); ++i) {
            if (token_type == Fy_reg16Tokens[i]) {
                return true;
            }
        }
        return false;
    case Fy_ParserArgType_Constant:
        if (token_type == Fy_TokenType_Const)
            return true;
        else
            return false;
    case Fy_ParserArgType_Reg8:
        for (size_t i = 0; i < sizeof(Fy_reg8Tokens) / sizeof(Fy_TokenType); ++i) {
            if (token_type == Fy_reg8Tokens[i]) {
                return true;
            }
        }
        return false;
    case Fy_ParserArgType_Label:
        if (token_type == Fy_TokenType_Label)
            return true;
        else
            return false;
    default:
        FY_UNREACHABLE();
    }
}

char *Fy_Token_toLowercaseCStr(Fy_Token *token) {
    char *cstr = malloc((token->length + 1) * sizeof(char));
    for (size_t i = 0; i < token->length; ++i)
        cstr[i] = tolower(token->start[i]);
    cstr[token->length] = '\0';
    return cstr;
}
