#ifndef FY_LEXER_H
#define FY_LEXER_H

#include "token.h"

#include <stdbool.h>

typedef struct Fy_Lexer {
    char *stream_base, *stream;
    Fy_Token token;
    size_t line, column;
} Fy_Lexer;

typedef enum Fy_LexerError {
    Fy_LexerError_Syntax = 1,
} Fy_LexerError;

void Fy_Lexer_Init(char *stream, Fy_Lexer *out);
bool Fy_Lexer_lex(Fy_Lexer *lexer);

#endif /* FY_LEXER_H */
