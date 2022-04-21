#ifndef FY_AST_H
#define FY_AST_H

#include <inttypes.h>

typedef struct Fy_AST Fy_AST;
typedef struct Fy_InlineValue Fy_InlineValue;

typedef enum Fy_ASTType {
    Fy_ASTType_Number = 1,
    Fy_ASTType_Add,
    Fy_ASTType_Sub
} Fy_ASTType;

struct Fy_AST {
    Fy_ASTType type;
    union {
        struct {
            Fy_AST *lhs;
            Fy_AST *rhs;
        };
        uint16_t as_number;
    };
};

struct Fy_InlineValue {
    int16_t numeric;
};

Fy_AST *Fy_AST_New(Fy_ASTType type);
void Fy_AST_delete(Fy_AST *ast);
void Fy_AST_eval(Fy_AST *ast, Fy_InlineValue *out);

#endif /* FY_AST_H */
