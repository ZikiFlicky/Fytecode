#include "fy.h"

static inline bool is_keyword_start_char(char c) {
    return isalpha(c) || c == '_';
}

static inline bool is_keyword_char(char c) {
    return is_keyword_start_char(c) || isdigit(c);
}

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
void Fy_Lexer_Init(char *stream, Fy_Lexer *out) {
    out->stream_base = stream;
    out->stream = stream;
    out->line = 1;
    out->column = 1;
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

    // The keyword cannot end with a keyword character (number, alphabetic etc)
    if (is_keyword_char(lexer->stream[i]))
        return false;

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
    bool removing = true;
    while (removing) {
        int8_t c = lexer->stream[0];
        if (c == ' ' || c == '\t') {
            ++lexer->stream;
            ++lexer->column;
        } else if (c == ';') {
            do {
                ++lexer->stream;
                ++lexer->column;
                c = lexer->stream[0];
            } while (c != '\n' && c != '\0');
        } else {
            removing = false;
        }
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

bool Fy_Lexer_lexLabel(Fy_Lexer *lexer) {
    if (!is_keyword_start_char(lexer->stream[0]))
        return false;
    lexer->token.type = Fy_TokenType_Label;
    lexer->token.start = lexer->stream;
    lexer->token.length = 0;
    do {
        ++lexer->stream;
        ++lexer->token.length;
    } while (is_keyword_char(lexer->stream[0]));

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
 * If end of file, returns false, otherwise returns true.
 */
bool Fy_Lexer_lex(Fy_Lexer *lexer) {
    Fy_Lexer_removeWhitespace(lexer);

    // Can't lex anymore
    if (lexer->stream[0] == '\0')
        return false;

    if (lexer->stream[0] == ':') {
        lexer->token.type = Fy_TokenType_Colon;
        lexer->token.start = lexer->stream;
        lexer->token.length = 1;
        ++lexer->stream;
        return true;
    }

    if (Fy_Lexer_lexNewline(lexer))
        return true;

    if (Fy_Lexer_matchKeyword(lexer, "debug", Fy_TokenType_Debug))
        return true;
    if (Fy_Lexer_matchKeyword(lexer, "jmp", Fy_TokenType_Jmp))
        return true;
    if (Fy_Lexer_matchKeyword(lexer, "end", Fy_TokenType_End))
        return true;
    if (Fy_Lexer_matchKeyword(lexer, "mov", Fy_TokenType_Mov))
        return true;
    if (Fy_Lexer_matchKeyword(lexer, "add", Fy_TokenType_Add))
        return true;
    if (Fy_Lexer_matchKeyword(lexer, "sub", Fy_TokenType_Sub))
        return true;
    if (Fy_Lexer_matchKeyword(lexer, "cmp", Fy_TokenType_Cmp))
        return true;
    if (Fy_Lexer_matchKeyword(lexer, "ax", Fy_TokenType_Ax))
        return true;
    if (Fy_Lexer_matchKeyword(lexer, "bx", Fy_TokenType_Bx))
        return true;

    if (Fy_Lexer_lexConst(lexer))
        return true;

    if (Fy_Lexer_lexLabel(lexer))
        return true;

    Fy_Lexer_error(lexer, Fy_LexerError_Syntax);
    return false; // Unreachable
}
