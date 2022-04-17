#include "fy.h"

Fy_TokenType Fy_reg16Tokens[] = {
    Fy_TokenType_Ax,
    Fy_TokenType_Bx,
    Fy_TokenType_Cx,
    Fy_TokenType_Dx
};

bool Fy_TokenType_isReg8(Fy_TokenType type) {
    switch (type) {
    default:
        return false;
    }
}

bool Fy_TokenType_isReg16(Fy_TokenType type) {
    switch (type) {
    case Fy_TokenType_Ax:
    case Fy_TokenType_Bx:
    case Fy_TokenType_Cx:
    case Fy_TokenType_Dx:
        return true;
    default:
        return false;
    }
}

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

/* Convert Fy_Token with type Fy_TokenType_Const to 16-bit integer */
int16_t Fy_Token_toConst16(Fy_Token *token, Fy_Parser *parser) {
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

        if (positive >= (1 << 15))
            Fy_Parser_error(parser, Fy_ParserError_ConstTooBig, NULL);

        ++i;
    } while (i < token->length);

    return (int16_t)((is_neg ? -1 : 1) * positive);
}

/* Convert Fy_Token with type Fy_TokenType_Const to 8-bit integer */
int8_t Fy_Token_toConst8(Fy_Token *token, Fy_Parser *parser) {
    bool is_neg;
    size_t i = 0;
    size_t positive = 0;

    assert(token->type == Fy_TokenType_Const);

    if (token->start[0] == '-') {
        is_neg = true;
        ++i;
    }

    do {
        positive *= 10;
        positive += token->start[i] - '0';

        if (positive >= (1 << 7))
            Fy_Parser_error(parser, Fy_ParserError_ConstTooBig, "%zu", positive);

        ++i;
    } while (i < token->length);

    return (int8_t)((is_neg ? -1 : 1) * positive);
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
        FY_UNREACHABLE(); // FIXME: Not implemented yet
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
