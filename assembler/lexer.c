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
    case Fy_LexerError_EOLReached:
        return "Reached EOL/EOF";
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

/* Show lexer error message and exit */
void Fy_Lexer_error(Fy_Lexer *lexer, Fy_LexerError error) {
    printf("LexerError[%zu,%zu]: %s\n",
            lexer->line, lexer->column,
            Fy_LexerError_toString(error));
    exit(1);
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
size_t Fy_Lexer_removeWhitespace(Fy_Lexer *lexer) {
    bool removing = true;
    size_t amount_removed = 0;
    while (removing) {
        char c = lexer->stream[0];
        if (c == ' ' || c == '\t') {
            ++lexer->stream;
            ++lexer->column;
            ++amount_removed;
        } else if (c == ';') {
            ++amount_removed;
            do {
                ++amount_removed;
                ++lexer->stream;
                ++lexer->column;
                c = lexer->stream[0];
            } while (c != '\n' && c != '\0');
        } else {
            removing = false;
        }
    }
    return amount_removed;
}

/* Lex a constant value */
bool Fy_Lexer_lexConst(Fy_Lexer *lexer) {
    size_t i = 0;

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

bool Fy_Lexer_lexSymbol(Fy_Lexer *lexer) {
    if (!is_keyword_start_char(lexer->stream[0]))
        return false;
    lexer->token.type = Fy_TokenType_Symbol;
    lexer->token.start = lexer->stream;
    lexer->token.length = 0;
    do {
        ++lexer->stream;
        ++lexer->token.length;
        ++lexer->column;
    } while (is_keyword_char(lexer->stream[0]));

    return true;
}

bool Fy_Lexer_lexString(Fy_Lexer *lexer) {
    if (lexer->stream[0] != '"')
        return false;

    ++lexer->stream;
    ++lexer->column;

    lexer->token.type = Fy_TokenType_String;
    lexer->token.start = lexer->stream;
    lexer->token.length = 0;

    while (lexer->stream[0] != '"') {
        if (lexer->stream[0] == '\n' || lexer->stream[0] == '\0')
            Fy_Lexer_error(lexer, Fy_LexerError_EOLReached);
        ++lexer->stream;
        ++lexer->column;
        ++lexer->token.length;
    }

    ++lexer->stream;
    ++lexer->column;

    return true;
}

static bool Fy_Lexer_lexChar(Fy_Lexer *lexer) {
    if (lexer->stream[0] == '\'') {
        if (lexer->stream[1] == '\0' || lexer->stream[1] == '\n')
            Fy_Lexer_error(lexer, Fy_LexerError_EOLReached);
        if (lexer->stream[1] == '\'')
            Fy_Lexer_error(lexer, Fy_LexerError_Syntax);
        if (lexer->stream[2] != '\'')
            Fy_Lexer_error(lexer, Fy_LexerError_Syntax);
        lexer->token.type = Fy_TokenType_Char;
        lexer->token.length = 1;
        lexer->token.start = &lexer->stream[1];
        lexer->stream += 3;
        lexer->column += 3;
        return true;
    } else {
        return false;
    }
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
        lexer->token.length += Fy_Lexer_removeWhitespace(lexer);
    } while (lexer->stream[0] == '\n');
    return true;
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
        ++lexer->column;
        return true;
    }

    if (lexer->stream[0] == '[') {
        lexer->token.type = Fy_TokenType_LeftBracket;
        lexer->token.start = lexer->stream;
        lexer->token.length = 1;
        ++lexer->stream;
        ++lexer->column;
        return true;
    }

    if (lexer->stream[0] == ']') {
        lexer->token.type = Fy_TokenType_RightBracket;
        lexer->token.start = lexer->stream;
        lexer->token.length = 1;
        ++lexer->stream;
        ++lexer->column;
        return true;
    }

    if (lexer->stream[0] == '(') {
        lexer->token.type = Fy_TokenType_LeftParen;
        lexer->token.start = lexer->stream;
        lexer->token.length = 1;
        ++lexer->stream;
        ++lexer->column;
        return true;
    }

    if (lexer->stream[0] == ')') {
        lexer->token.type = Fy_TokenType_RightParen;
        lexer->token.start = lexer->stream;
        lexer->token.length = 1;
        ++lexer->stream;
        ++lexer->column;
        return true;
    }

    if (lexer->stream[0] == '-') {
        lexer->token.type = Fy_TokenType_Minus;
        lexer->token.start = lexer->stream;
        lexer->token.length = 1;
        ++lexer->column;
        ++lexer->stream;
        return true;
    }

    if (lexer->stream[0] == '+') {
        lexer->token.type = Fy_TokenType_Plus;
        lexer->token.start = lexer->stream;
        lexer->token.length = 1;
        ++lexer->column;
        ++lexer->stream;
        return true;
    }

    if (lexer->stream[0] == ',') {
        lexer->token.type = Fy_TokenType_Comma;
        lexer->token.start = lexer->stream;
        lexer->token.length = 1;
        ++lexer->column;
        ++lexer->stream;
        return true;
    }

    if (lexer->stream[0] == '=') {
        lexer->token.type = Fy_TokenType_EqualSign;
        lexer->token.start = lexer->stream;
        lexer->token.length = 1;
        ++lexer->column;
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
    if (Fy_Lexer_matchKeyword(lexer, "lea", Fy_TokenType_Lea))
        return true;
    if (Fy_Lexer_matchKeyword(lexer, "add", Fy_TokenType_Add))
        return true;
    if (Fy_Lexer_matchKeyword(lexer, "sub", Fy_TokenType_Sub))
        return true;
    if (Fy_Lexer_matchKeyword(lexer, "and", Fy_TokenType_And))
        return true;
    if (Fy_Lexer_matchKeyword(lexer, "or", Fy_TokenType_Or))
        return true;
    if (Fy_Lexer_matchKeyword(lexer, "xor", Fy_TokenType_Xor))
        return true;
    if (Fy_Lexer_matchKeyword(lexer, "cmp", Fy_TokenType_Cmp))
        return true;
    if (Fy_Lexer_matchKeyword(lexer, "push", Fy_TokenType_Push))
        return true;
    if (Fy_Lexer_matchKeyword(lexer, "pop", Fy_TokenType_Pop))
        return true;
    if (Fy_Lexer_matchKeyword(lexer, "int", Fy_TokenType_Int))
        return true;
    if (Fy_Lexer_matchKeyword(lexer, "ax", Fy_TokenType_Ax))
        return true;
    if (Fy_Lexer_matchKeyword(lexer, "bx", Fy_TokenType_Bx))
        return true;
    if (Fy_Lexer_matchKeyword(lexer, "cx", Fy_TokenType_Cx))
        return true;
    if (Fy_Lexer_matchKeyword(lexer, "dx", Fy_TokenType_Dx))
        return true;
    if (Fy_Lexer_matchKeyword(lexer, "sp", Fy_TokenType_Sp))
        return true;
    if (Fy_Lexer_matchKeyword(lexer, "bp", Fy_TokenType_Bp))
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
    if (Fy_Lexer_matchKeyword(lexer, "data", Fy_TokenType_Data))
        return true;
    if (Fy_Lexer_matchKeyword(lexer, "eb", Fy_TokenType_Eb))
        return true;
    if (Fy_Lexer_matchKeyword(lexer, "ew", Fy_TokenType_Ew))
        return true;
    if (Fy_Lexer_matchKeyword(lexer, "code", Fy_TokenType_Code))
        return true;
    if (Fy_Lexer_matchKeyword(lexer, "byte", Fy_TokenType_Byte))
        return true;
    if (Fy_Lexer_matchKeyword(lexer, "word", Fy_TokenType_Word))
        return true;
    if (Fy_Lexer_matchKeyword(lexer, "dup", Fy_TokenType_Dup))
        return true;

    if (Fy_Lexer_lexConst(lexer))
        return true;

    if (Fy_Lexer_lexSymbol(lexer))
        return true;

    if (Fy_Lexer_lexString(lexer))
        return true;

    if (Fy_Lexer_lexChar(lexer))
        return true;

    Fy_Lexer_error(lexer, Fy_LexerError_Syntax);
    return false; // Unreachable
}
