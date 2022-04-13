#include "fy.h"

/* Convert Fy_LexerError to string */
char *Fy_LexerError_toString(Fy_LexerError error) {
    switch (error) {
    case Fy_LexerError_Syntax:
        return "Syntax error";
    default:
        FY_UNREACHABLE();
    }
}

/* Initialize a lexer */
void Fy_Lexer_Init(Fy_Lexer *lexer, char *stream) {
    lexer->stream_base = stream;
    lexer->stream = stream;
    lexer->line = 1;
    lexer->column = 1;
}

/*
 * Returns true if matched a keyword, and puts the keyword in the lexer's token.
 * Parameter `keyword` must be lowercase.
 */
bool Fy_Lexer_matchKeyword(Fy_Lexer *lexer, char *keyword, Fy_TokenType type) {
    size_t i;

    for (i = 0; keyword[i] != '\0'; ++i) {
        if (lexer->stream[i] == '\0')
            return false;
        if (keyword[i] != tolower(lexer->stream[i]))
            return false;
    }

    // Set the token member
    lexer->token.type = type;
    lexer->token.start = lexer->stream;
    lexer->token.length = i;

    lexer->stream += i;
    lexer->column += i;

    return true;
}

/* Removes space and tab (indent characters) */
void Fy_Lexer_removeWhitespace(Fy_Lexer *lexer) {
    int8_t c;
    while ((c = lexer->stream[0]) == ' ' || c == '\t') {
        ++lexer->stream;
        ++lexer->column;
    }
}

/* Lex a constant value */
bool Fy_Lexer_lexConst(Fy_Lexer *lexer) {
    size_t i = 0;

    if (lexer->stream[0] == '-')
        ++i;
    if (!isdigit(lexer->stream[i]))
        return false;
    do {
        ++i;
    } while (isdigit(lexer->stream[i]));

    lexer->token.type = Fy_TokenType_Const;
    lexer->token.start = lexer->stream;
    lexer->token.length = i;

    lexer->stream += i;
    lexer->column += i;

    return true;
}

/*
 * Lex a newline.
 * Returns false on failure.
 */
bool Fy_Lexer_lexNewline(Fy_Lexer *lexer) {
    if (lexer->stream[0] != '\n')
        return false;
    lexer->token.type = Fy_TokenType_Newline;
    lexer->stream = lexer->stream;
    lexer->token.length = 0;
    lexer->column = 1;
    do {
        ++lexer->stream;
        ++lexer->line;
        ++lexer->token.length;
        Fy_Lexer_removeWhitespace(lexer);
    } while (lexer->stream[0] == '\n');
    return true;
}

/* Show lexer error message and exit */
void Fy_Lexer_error(Fy_Lexer *lexer, Fy_LexerError error) {
    printf("LexerError[%zu,%zu]: %s\n",
            lexer->line, lexer->column,
            Fy_LexerError_toString(error));
    exit(1);
}

/*
 * Lex a token into the `token` member.
 * If unable to tokenize, returns false, otherwise returns true.
 */
bool Fy_Lexer_lex(Fy_Lexer *lexer) {
    Fy_Lexer_removeWhitespace(lexer);

    // Can't lex anymore
    if (lexer->stream[0] == '\0')
        return false;

    if (Fy_Lexer_lexNewline(lexer))
        return true;

    if (Fy_Lexer_matchKeyword(lexer, "mov", Fy_TokenType_Mov))
        return true;
    if (Fy_Lexer_matchKeyword(lexer, "ax", Fy_TokenType_Ax))
        return true;
    if (Fy_Lexer_matchKeyword(lexer, "bx", Fy_TokenType_Bx))
        return true;
    if (Fy_Lexer_lexConst(lexer))
        return true;

    Fy_Lexer_error(lexer, Fy_LexerError_Syntax);
    return false; // Unreachable
}
