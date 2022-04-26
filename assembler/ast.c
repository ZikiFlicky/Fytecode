#include "fy.h"

Fy_AST *Fy_AST_New(Fy_ASTType type) {
    Fy_AST *ast = malloc(sizeof(Fy_AST));
    ast->type = type;
    return ast;
}

void Fy_AST_eval(Fy_AST *ast, Fy_InlineValue *out) {
    switch (ast->type) {
    case Fy_ASTType_Number:
        out->numeric = ast->as_number;
        out->times_bx = 0;
        break;
    case Fy_ASTType_Bx:
        out->numeric = 0;
        out->times_bx = 1;
        out->times_bp = 0;
        break;
    case Fy_ASTType_Bp:
        out->numeric = 0;
        out->times_bx = 0;
        out->times_bp = 1;
        break;
    case Fy_ASTType_Add: {
        Fy_InlineValue lhs;
        Fy_InlineValue rhs;
        Fy_AST_eval(ast->lhs, &lhs);
        Fy_AST_eval(ast->rhs, &rhs);
        out->numeric = lhs.numeric + rhs.numeric;
        out->times_bx = lhs.times_bx + rhs.times_bx;
        out->times_bp = lhs.times_bp + rhs.times_bp;
        break;
    }
    case Fy_ASTType_Sub: {
        Fy_InlineValue lhs;
        Fy_InlineValue rhs;
        Fy_AST_eval(ast->lhs, &lhs);
        Fy_AST_eval(ast->rhs, &rhs);
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
    case Fy_ASTType_Add:
    case Fy_ASTType_Sub:
        Fy_AST_delete(ast->lhs);
        Fy_AST_delete(ast->rhs);
        break;
    }
    free(ast);
}
