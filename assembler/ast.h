#ifndef FY_AST_H
#define FY_AST_H

#include "parser.h"

#include <stdbool.h>
#include <inttypes.h>

typedef struct Fy_AST Fy_AST;
typedef struct Fy_InlineValue Fy_InlineValue;

typedef enum Fy_ASTType {
    Fy_ASTType_Number = 1,
    Fy_ASTType_Label,
    Fy_ASTType_Bx,
    Fy_ASTType_Bp,
    Fy_ASTType_Add,
    Fy_ASTType_Sub
} Fy_ASTType;

struct Fy_AST {
    Fy_ASTType type;
    Fy_ParserState state;
    union {
        struct {
            Fy_AST *lhs;
            Fy_AST *rhs;
        };
        uint16_t as_number;
        char *as_label;
    };
};

struct Fy_InlineValue {
    bool has_variable;
    uint16_t variable_offset;
    int16_t numeric;
    int16_t times_bx;
    int16_t times_bp;
};

Fy_AST *Fy_AST_New(Fy_ASTType type);
void Fy_AST_delete(Fy_AST *ast);
void Fy_AST_eval(Fy_AST *ast, Fy_Parser *parser, Fy_InlineValue *out);

#endif /* FY_AST_H */
