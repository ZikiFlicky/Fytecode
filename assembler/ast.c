#include "fy.h"

static Fy_AST *Fy_ASTParser_parseSumExpr(Fy_ASTParser *ast_parser);
static inline bool Fy_InlineValue_isFullyNumeric(Fy_InlineValue *inline_value);

Fy_AST *Fy_AST_New(Fy_ASTType type) {
    Fy_AST *ast = malloc(sizeof(Fy_AST));
    ast->type = type;
    return ast;
}

void Fy_AST_Delete(Fy_AST *ast) {
    switch (ast->type) {
    case Fy_ASTType_Number:
    case Fy_ASTType_Bx:
    case Fy_ASTType_Bp:
        break;
    case Fy_ASTType_Variable:
        free(ast->as_variable);
        break;
    case Fy_ASTType_Add:
    case Fy_ASTType_Sub:
    case Fy_ASTType_Mul:
    case Fy_ASTType_Div:
        Fy_AST_Delete(ast->lhs);
        Fy_AST_Delete(ast->rhs);
        break;
    case Fy_ASTType_Neg:
        Fy_AST_Delete(ast->as_neg);
        break;
    default:
        FY_UNREACHABLE();
    }
    free(ast);
}

static Fy_AST *Fy_ASTParser_parseLiteralExpr(Fy_ASTParser *ast_parser) {
    Fy_Parser *parser;
    Fy_AST *expr = NULL;
    Fy_ParserState backtrack;

    parser = ast_parser->regular_parser;
    Fy_Parser_dumpState(parser, &backtrack);

    if (Fy_Parser_match(parser, Fy_TokenType_Const, true)) {
        uint16_t literal = Fy_Token_toConst16(&parser->token, parser);
        expr = Fy_AST_New(Fy_ASTType_Number);
        expr->as_number = literal;
        return expr;
    }

    if (Fy_Parser_match(parser, Fy_TokenType_Char, true)) {
        uint16_t char_value = parser->token.start[0];
        expr = Fy_AST_New(Fy_ASTType_Number);
        expr->as_number = char_value;
        return expr;
    }

    if (ast_parser->type == Fy_ASTParserType_Memory) {
        if (!Fy_Parser_lex(parser, true))
            return NULL;

        switch (parser->token.type) {
        case Fy_TokenType_Symbol: {
            expr = Fy_AST_New(Fy_ASTType_Variable);
            expr->as_variable = Fy_Token_toLowercaseCStr(&parser->token);
            break;
        }
        case Fy_TokenType_Bx:
            expr = Fy_AST_New(Fy_ASTType_Bx);
            break;
        case Fy_TokenType_Bp:
            expr = Fy_AST_New(Fy_ASTType_Bp);
            break;
        default:
            Fy_Parser_loadState(parser, &backtrack);
        }
    }

    if (expr)
        expr->state = backtrack;
    return expr;
}

static Fy_AST *Fy_ASTParser_parseSingle(Fy_ASTParser *ast_parser) {
    Fy_Parser *parser = ast_parser->regular_parser;
    Fy_AST *ast;

    if (Fy_Parser_match(parser, Fy_TokenType_LeftParen, true)) { // Paren expression
        if (!(ast = Fy_ASTParser_parseSumExpr(ast_parser)))
           Fy_Parser_error(parser, Fy_ParserError_SyntaxError, NULL, NULL);
        if (!Fy_Parser_match(parser, Fy_TokenType_RightParen, true))
            Fy_Parser_error(parser, Fy_ParserError_ExpectedDifferentToken, NULL, "')'");
    } else if (Fy_Parser_match(parser, Fy_TokenType_Minus, true)) {
        Fy_AST *inner;
        if (!(inner = Fy_ASTParser_parseSingle(ast_parser)))
           Fy_Parser_error(parser, Fy_ParserError_SyntaxError, NULL, NULL);
        ast = Fy_AST_New(Fy_ASTType_Neg);
        ast->as_neg = inner;
    } else if (!(ast = Fy_ASTParser_parseLiteralExpr(ast_parser))) {
        ast = NULL;
    }

    return ast;
}

static Fy_AST *Fy_ASTParser_parseProductExpr(Fy_ASTParser *ast_parser) {
    Fy_Parser *parser = ast_parser->regular_parser;
    Fy_AST *expr;

    if (!(expr = Fy_ASTParser_parseSingle(ast_parser)))
        return NULL;

    for (;;) {
        Fy_ParserState backtrack;
        Fy_AST *new_expr, *rhs;
        Fy_ASTType type;

        Fy_Parser_dumpState(parser, &backtrack);

        if (!Fy_Parser_lex(parser, true))
            break;

        switch (parser->token.type) {
        case Fy_TokenType_Star:
            type = Fy_ASTType_Mul;
            break;
        case Fy_TokenType_Slash:
            type = Fy_ASTType_Div;
            break;
        default:
            Fy_Parser_loadState(parser, &backtrack);
            return expr;
        }

        if (!(rhs = Fy_ASTParser_parseSingle(ast_parser)))
            Fy_Parser_error(parser, Fy_ParserError_SyntaxError, NULL, NULL);

        new_expr = Fy_AST_New(type);
        new_expr->lhs = expr;
        new_expr->rhs = rhs;
        new_expr->state = backtrack;
        expr = new_expr;
    }

    return expr;
}

static Fy_AST *Fy_ASTParser_parseSumExpr(Fy_ASTParser *ast_parser) {
    Fy_Parser *parser = ast_parser->regular_parser;
    Fy_AST *expr;

    if (!(expr = Fy_ASTParser_parseProductExpr(ast_parser)))
        return NULL;

    for (;;) {
        Fy_ParserState backtrack;
        Fy_AST *new_expr, *rhs;
        Fy_ASTType type;

        Fy_Parser_dumpState(parser, &backtrack);

        if (!Fy_Parser_lex(parser, true))
            break;

        switch (parser->token.type) {
        case Fy_TokenType_Plus:
            type = Fy_ASTType_Add;
            break;
        case Fy_TokenType_Minus:
            type = Fy_ASTType_Sub;
            break;
        default:
            Fy_Parser_loadState(parser, &backtrack);
            return expr;
        }

        if (!(rhs = Fy_ASTParser_parseProductExpr(ast_parser)))
            Fy_Parser_error(parser, Fy_ParserError_SyntaxError, NULL, NULL);

        new_expr = Fy_AST_New(type);
        new_expr->lhs = expr;
        new_expr->rhs = rhs;
        new_expr->state = backtrack;
        expr = new_expr;
    }

    return expr;
}

Fy_AST *Fy_Parser_parseMemExpr(Fy_Parser *parser, Fy_InstructionArgType *out) {
    Fy_ASTParser ast_parser;
    Fy_InstructionArgType type;
    Fy_AST *ast;

    ast_parser.type = Fy_ASTParserType_Memory;
    ast_parser.regular_parser = parser;

    if (!Fy_Parser_match(parser, Fy_TokenType_LeftBracket, true))
        return NULL;

    // Decide which memory type this is
    if (Fy_Parser_match(parser, Fy_TokenType_Byte, true))
        type = Fy_InstructionArgType_Memory8;
    else if (Fy_Parser_match(parser, Fy_TokenType_Word, true))
        type = Fy_InstructionArgType_Memory16;
    else
        type = Fy_InstructionArgType_MemoryUnknownSize;

    ast = Fy_ASTParser_parseSumExpr(&ast_parser);
    if (!ast)
        Fy_Parser_error(parser, Fy_ParserError_SyntaxError, NULL, "Expected address");

    if (!Fy_Parser_match(parser, Fy_TokenType_RightBracket, true))
        Fy_Parser_error(parser, Fy_ParserError_ExpectedDifferentToken, NULL, "']'");

    *out = type;
    return ast;
}

static Fy_AST *Fy_Parser_parseNumericLiteral(Fy_Parser *parser) {
    Fy_ASTParser ast_parser;
    Fy_AST *ast;

    ast_parser.type = Fy_ASTParserType_Number;
    ast_parser.regular_parser = parser;

    ast = Fy_ASTParser_parseSumExpr(&ast_parser);
    return ast;
}

bool Fy_Parser_getConst16(Fy_Parser *parser, uint16_t *out) {
    Fy_InlineValue inline_value;
    Fy_AST *ast = Fy_Parser_parseNumericLiteral(parser);

    if (!ast)
        return false;

    Fy_AST_eval(ast, parser, &inline_value);
    Fy_AST_Delete(ast);

    *out = *(uint16_t*)&inline_value.numeric;
    return true;
}

bool Fy_Parser_getConst8(Fy_Parser *parser, uint8_t *out) {
    Fy_InlineValue inline_value;
    Fy_AST *ast = Fy_Parser_parseNumericLiteral(parser);

    if (!ast)
        return false;

    Fy_AST_eval(ast, parser, &inline_value);
    Fy_AST_Delete(ast);

    if (inline_value.numeric > 0xff || inline_value.numeric < -0x80)
        Fy_Parser_error(parser, Fy_ParserError_ConstTooBig, NULL, "%d", inline_value.numeric); // FIXME: Error out of range

    *out = (uint8_t)(int8_t)inline_value.numeric;
    return true;
}

void Fy_AST_eval(Fy_AST *ast, Fy_Parser *parser, Fy_InlineValue *out) {
    switch (ast->type) {
    case Fy_ASTType_Number:
        out->has_variable = false;
        out->numeric = ast->as_number;
        out->times_bp = 0;
        out->times_bx = 0;
        break;
    case Fy_ASTType_Variable: {
        Fy_BucketNode *entry = Fy_Symbolmap_getEntry(&parser->symmap, ast->as_variable);
        if (!entry)
            Fy_Parser_error(parser, Fy_ParserError_SymbolNotFound, &ast->state, "%s", ast->as_variable);
        if (entry->type != Fy_MapEntryType_Variable)
            Fy_Parser_error(parser, Fy_ParserError_SymbolNotVariable, &ast->state, "%s", ast->as_variable);
        out->has_variable = true;
        out->variable_offset = entry->data_offset;
        out->numeric = 0;
        out->times_bx = 0;
        out->times_bp = 0;
        break;
    }
    case Fy_ASTType_Bx:
        out->has_variable = false;
        out->numeric = 0;
        out->times_bx = 1;
        out->times_bp = 0;
        break;
    case Fy_ASTType_Bp:
        out->has_variable = false;
        out->numeric = 0;
        out->times_bx = 0;
        out->times_bp = 1;
        break;
    case Fy_ASTType_Add: {
        Fy_InlineValue lhs;
        Fy_InlineValue rhs;
        Fy_AST_eval(ast->lhs, parser, &lhs);
        Fy_AST_eval(ast->rhs, parser, &rhs);
        if (lhs.has_variable && rhs.has_variable)
            Fy_Parser_error(parser, Fy_ParserError_UnexpectedToken, &ast->rhs->state, NULL);
        if (lhs.has_variable) {
            out->has_variable = true;
            out->variable_offset = lhs.variable_offset;
        } else if (rhs.has_variable) {
            out->has_variable = true;
            out->variable_offset = rhs.variable_offset;
        } else {
            out->has_variable = false;
        }
        out->numeric = lhs.numeric + rhs.numeric;
        out->times_bx = lhs.times_bx + rhs.times_bx;
        out->times_bp = lhs.times_bp + rhs.times_bp;
        break;
    }
    case Fy_ASTType_Sub: {
        Fy_InlineValue lhs;
        Fy_InlineValue rhs;
        Fy_AST_eval(ast->lhs, parser, &lhs);
        Fy_AST_eval(ast->rhs, parser, &rhs);
        if (rhs.has_variable)
            Fy_Parser_error(parser, Fy_ParserError_UnexpectedToken, &ast->rhs->state, NULL);
        if (lhs.has_variable) {
            out->has_variable = true;
            out->variable_offset = lhs.variable_offset;
        } else {
            out->has_variable = false;
        }
        out->numeric = lhs.numeric - rhs.numeric;
        out->times_bx = lhs.times_bx - rhs.times_bx;
        out->times_bp = lhs.times_bp - rhs.times_bp;
        break;
    }
    case Fy_ASTType_Mul: {
        Fy_InlineValue lhs;
        Fy_InlineValue rhs;
        int16_t multiplier;

        Fy_AST_eval(ast->lhs, parser, &lhs);
        Fy_AST_eval(ast->rhs, parser, &rhs);

        if (Fy_InlineValue_isFullyNumeric(&lhs)) {
            if (rhs.has_variable)
                Fy_Parser_error(parser, Fy_ParserError_InvalidOperation, &ast->rhs->state, "Variables in addresses cannot be multiplied");
            multiplier = (int16_t)lhs.numeric;
            out->numeric = multiplier * rhs.numeric;
            out->times_bp = multiplier * rhs.times_bp;
            out->times_bx = multiplier * rhs.times_bx;
        } else if (Fy_InlineValue_isFullyNumeric(&rhs)) {
            if (lhs.has_variable)
                Fy_Parser_error(parser, Fy_ParserError_InvalidOperation, &ast->lhs->state, "Variables in addresses cannot be multiplied");
            multiplier = (int16_t)rhs.numeric;
            out->numeric = multiplier * lhs.numeric;
            out->times_bp = multiplier * lhs.times_bp;
            out->times_bx = multiplier * lhs.times_bx;
        } else {
            Fy_Parser_error(parser, Fy_ParserError_InvalidOperation, &ast->state, NULL);
        }
        out->has_variable = false;
        break;
    }
    case Fy_ASTType_Div: {
        Fy_InlineValue lhs;
        Fy_InlineValue rhs;
        int16_t divider;

        Fy_AST_eval(ast->lhs, parser, &lhs);
        Fy_AST_eval(ast->rhs, parser, &rhs);

        if (!Fy_InlineValue_isFullyNumeric(&rhs))
            Fy_Parser_error(parser, Fy_ParserError_InvalidOperation, &ast->rhs->state, "Expected to divide in a number");

        if (lhs.has_variable)
            Fy_Parser_error(parser, Fy_ParserError_InvalidOperation, &ast->lhs->state, "Variables in addresses cannot be divided");

        divider = (int16_t)rhs.numeric;

        out->has_variable = false;
        out->numeric = lhs.numeric / divider;
        out->times_bp = lhs.times_bp / divider;
        out->times_bx = lhs.times_bx / divider;
        break;
    }
    case Fy_ASTType_Neg: {
        Fy_InlineValue value;
        Fy_AST_eval(ast->as_neg, parser, &value);
        if (value.has_variable)
            Fy_Parser_error(parser, Fy_ParserError_InvalidInlineValue, &ast->as_neg->state, "has variable");
        out->has_variable = false;
        out->numeric = -value.numeric;
        out->times_bp = -value.times_bp;
        out->times_bx = -value.times_bx;
        break;
    }
    default:
        FY_UNREACHABLE();
    }
}

static inline bool Fy_InlineValue_isFullyNumeric(Fy_InlineValue *inline_value) {
    return !inline_value->has_variable && !inline_value->times_bp && !inline_value->times_bx;
}

uint16_t Fy_InlineValue_getMapping(Fy_InlineValue *inline_value, uint8_t *mapping) {
    uint16_t size = 1; // Initialized to one because the mapping is one byte
    uint16_t map = 0;
    if (inline_value->has_variable) {
        map |= FY_INLINEVAL_MAPPING_HASVAR;
        size += 2;
    }
    if (inline_value->numeric) {
        map |= FY_INLINEVAL_MAPPING_HASNUM;
        size += 2;
    }
    if (inline_value->times_bp) {
        map |= FY_INLINEVAL_MAPPING_HASBP;
        size += 2;
    }
    if (inline_value->times_bx) {
        map |= FY_INLINEVAL_MAPPING_HASBX;
        size += 2;
    }
    if (mapping)
        *mapping = map;
    return size;
}
