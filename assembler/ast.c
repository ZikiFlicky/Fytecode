#include "fy.h"

Fy_AST *Fy_AST_New(Fy_ASTType type) {
    Fy_AST *ast = malloc(sizeof(Fy_AST));
    ast->type = type;
    return ast;
}

void Fy_AST_eval(Fy_AST *ast, Fy_Parser *parser, Fy_InlineValue *out) {
    switch (ast->type) {
    case Fy_ASTType_Number:
        out->has_variable = false;
        out->numeric = ast->as_number;
        out->times_bp = 0;
        out->times_bx = 0;
        break;
    case Fy_ASTType_Label: {
        Fy_BucketNode *entry = Fy_Labelmap_getEntry(&parser->labelmap, ast->as_label);
        if (entry->type != Fy_MapEntryType_Variable)
            Fy_Parser_error(parser, Fy_ParserError_LabelNotVariable, &ast->state, "%s", ast->as_label);
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
    default:
        FY_UNREACHABLE();
    }
}

void Fy_AST_delete(Fy_AST *ast) {
    switch (ast->type) {
    case Fy_ASTType_Number:
    case Fy_ASTType_Bx:
    case Fy_ASTType_Bp:
        break;
    case Fy_ASTType_Label:
        free(ast->as_label);
        break;
    case Fy_ASTType_Add:
    case Fy_ASTType_Sub:
        Fy_AST_delete(ast->lhs);
        Fy_AST_delete(ast->rhs);
        break;
    }
    free(ast);
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
