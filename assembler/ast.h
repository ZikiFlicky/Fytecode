#ifndef FY_AST_H
#define FY_AST_H

#include "parser.h"

#include <stdbool.h>
#include <inttypes.h>

typedef enum Fy_ASTParserType Fy_ASTParserType;
typedef struct Fy_ASTParser Fy_ASTParser;
typedef struct Fy_AST Fy_AST;
typedef struct Fy_InlineValue Fy_InlineValue;

#define FY_INLINEVAL_MAPPING_HASVAR (1 << 0)
#define FY_INLINEVAL_MAPPING_HASBP (1 << 1)
#define FY_INLINEVAL_MAPPING_HASBX (1 << 2)
#define FY_INLINEVAL_MAPPING_HASNUM (1 << 3)

typedef enum Fy_ASTType {
    Fy_ASTType_Number = 1,
    Fy_ASTType_Variable,
    Fy_ASTType_Bx,
    Fy_ASTType_Bp,
    Fy_ASTType_Add,
    Fy_ASTType_Sub,
    Fy_ASTType_Mul,
    Fy_ASTType_Div,
    Fy_ASTType_Neg
} Fy_ASTType;

struct Fy_AST {
    Fy_ASTType type;
    Fy_ParserState state;
    union {
        struct {
            Fy_AST *lhs;
            Fy_AST *rhs;
        };
        Fy_AST *as_neg;
        uint16_t as_number;
        char *as_variable;
    };
};

enum Fy_ASTParserType {
    Fy_ASTParserType_Memory = 1,
    Fy_ASTParserType_Number
};

struct Fy_ASTParser {
    Fy_ASTParserType type;
    Fy_Parser *regular_parser;
};

struct Fy_InlineValue {
    bool has_variable;
    uint16_t variable_offset;
    int16_t numeric;
    int16_t times_bx;
    int16_t times_bp;
};

Fy_AST *Fy_Parser_parseMemExpr(Fy_Parser *parser, Fy_InstructionArgType *out);
bool Fy_Parser_getConst16(Fy_Parser *parser, uint16_t *out);
bool Fy_Parser_getConst8(Fy_Parser *parser, uint8_t *out);
Fy_AST *Fy_AST_New(Fy_ASTType type);
void Fy_AST_Delete(Fy_AST *ast);
void Fy_AST_eval(Fy_AST *ast, Fy_Parser *parser, Fy_InlineValue *out);

uint16_t Fy_InlineValue_getMapping(Fy_InlineValue *inline_value, uint8_t *mapping);

#endif /* FY_AST_H */
