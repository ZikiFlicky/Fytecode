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
        break;
    case Fy_ASTType_Add: {
        Fy_InlineValue lhs;
        Fy_InlineValue rhs;
        Fy_AST_eval(ast->lhs, &lhs);
        Fy_AST_eval(ast->rhs, &rhs);
        out->numeric = lhs.numeric + rhs.numeric;
        break;
    }
    case Fy_ASTType_Sub: {
        Fy_InlineValue lhs;
        Fy_InlineValue rhs;
        Fy_AST_eval(ast->lhs, &lhs);
        Fy_AST_eval(ast->rhs, &rhs);
        out->numeric = lhs.numeric + rhs.numeric;
        break;
    }
    default:
        FY_UNREACHABLE();
    }
}

void Fy_AST_delete(Fy_AST *ast) {
    switch (ast->type) {
    case Fy_ASTType_Number:
        break;
    case Fy_ASTType_Add:
    case Fy_ASTType_Sub:
        Fy_AST_delete(ast->lhs);
        Fy_AST_delete(ast->rhs);
        break;
    }
    free(ast);
}
