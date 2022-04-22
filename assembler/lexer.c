#include "fy.h"

static inline bool is_keyword_start_char(char c) {
    return isalpha(c) || c == '_';
}

static inline bool is_keyword_char(char c) {
    return is_keyword_start_char(c) || isdigit(c);
}

static inline bool is_bin_char(char c) {
    return c == '0' || c == '1';
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
        char c = lexer->stream[0];
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

    if (lexer->stream[i] == '0' && lexer->stream[i + 1] == 'x') {
        i += 2;
        if (!isxdigit(lexer->stream[i]))
            return false;
        do {
            ++i;
        } while (isxdigit(lexer->stream[i]));

        // If ended with a character
        if (is_keyword_char(lexer->stream[i]) && !isxdigit(lexer->stream[i]))
            return false;
    } else if (lexer->stream[i] == '0' && lexer->stream[i + 1] == 'b') {
        i += 2;
        if (!is_bin_char(lexer->stream[i]))
            return false;
        do {
            ++i;
        } while (is_bin_char(lexer->stream[i]));

        // If ended with a character
        if (is_keyword_char(lexer->stream[i]) && !is_bin_char(lexer->stream[i]))
            return false;
    } else {
        if (!isdigit(lexer->stream[i]))
            return false;
        do {
            ++i;
        } while (isdigit(lexer->stream[i]));

        // If ended with a character
        if (is_keyword_char(lexer->stream[i]) && !isdigit(lexer->stream[i]))
            return false;
    }

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
        ++lexer->column;
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
    do {
        ++lexer->stream;
        ++lexer->line;
        lexer->column = 1;
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

    if (lexer->stream[0] == '[') {
        lexer->token.type = Fy_TokenType_LeftBracket;
        lexer->token.start = lexer->stream;
        lexer->token.length = 1;
        ++lexer->stream;
        return true;
    }

    if (lexer->stream[0] == ']') {
        lexer->token.type = Fy_TokenType_RightBracket;
        lexer->token.start = lexer->stream;
        lexer->token.length = 1;
        ++lexer->stream;
        return true;
    }

    if (lexer->stream[0] == '-') {
        // If this is actually a constant
        if (Fy_Lexer_lexConst(lexer))
            return true;

        lexer->token.type = Fy_TokenType_Minus;
        lexer->token.start = lexer->stream;
        lexer->token.length = 1;
        ++lexer->stream;
        return true;
    }

    if (lexer->stream[0] == '+') {
        lexer->token.type = Fy_TokenType_Plus;
        lexer->token.start = lexer->stream;
        lexer->token.length = 1;
        ++lexer->stream;
        return true;
    }

    if (Fy_Lexer_lexNewline(lexer))
        return true;

    if (Fy_Lexer_matchKeyword(lexer, "nop", Fy_TokenType_Nop))
        return true;
    if (Fy_Lexer_matchKeyword(lexer, "debug", Fy_TokenType_Debug))
        return true;
    if (Fy_Lexer_matchKeyword(lexer, "debugstack", Fy_TokenType_DebugStack))
        return true;
    if (Fy_Lexer_matchKeyword(lexer, "jmp", Fy_TokenType_Jmp))
        return true;
    if (Fy_Lexer_matchKeyword(lexer, "je", Fy_TokenType_Je))
        return true;
    if (Fy_Lexer_matchKeyword(lexer, "jl", Fy_TokenType_Jl))
        return true;
    if (Fy_Lexer_matchKeyword(lexer, "jg", Fy_TokenType_Jg))
        return true;
    if (Fy_Lexer_matchKeyword(lexer, "call", Fy_TokenType_Call))
        return true;
    if (Fy_Lexer_matchKeyword(lexer, "ret", Fy_TokenType_Ret))
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
    if (Fy_Lexer_matchKeyword(lexer, "push", Fy_TokenType_Push))
        return true;
    if (Fy_Lexer_matchKeyword(lexer, "pop", Fy_TokenType_Pop))
        return true;
    if (Fy_Lexer_matchKeyword(lexer, "ax", Fy_TokenType_Ax))
        return true;
    if (Fy_Lexer_matchKeyword(lexer, "bx", Fy_TokenType_Bx))
        return true;
    if (Fy_Lexer_matchKeyword(lexer, "cx", Fy_TokenType_Cx))
        return true;
    if (Fy_Lexer_matchKeyword(lexer, "dx", Fy_TokenType_Dx))
        return true;
    if (Fy_Lexer_matchKeyword(lexer, "ah", Fy_TokenType_Ah))
        return true;
    if (Fy_Lexer_matchKeyword(lexer, "al", Fy_TokenType_Al))
        return true;
    if (Fy_Lexer_matchKeyword(lexer, "bh", Fy_TokenType_Bh))
        return true;
    if (Fy_Lexer_matchKeyword(lexer, "bl", Fy_TokenType_Bl))
        return true;
    if (Fy_Lexer_matchKeyword(lexer, "ch", Fy_TokenType_Ch))
        return true;
    if (Fy_Lexer_matchKeyword(lexer, "cl", Fy_TokenType_Cl))
        return true;
    if (Fy_Lexer_matchKeyword(lexer, "dh", Fy_TokenType_Dh))
        return true;
    if (Fy_Lexer_matchKeyword(lexer, "dl", Fy_TokenType_Dl))
        return true;
    if (Fy_Lexer_matchKeyword(lexer, "proc", Fy_TokenType_Proc))
        return true;
    if (Fy_Lexer_matchKeyword(lexer, "endp", Fy_TokenType_Endp))
        return true;

    if (Fy_Lexer_lexConst(lexer))
        return true;

    if (Fy_Lexer_lexLabel(lexer))
        return true;

    Fy_Lexer_error(lexer, Fy_LexerError_Syntax);
    return false; // Unreachable
}
