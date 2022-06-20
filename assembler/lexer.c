#include "fy.h"

/* Declare all static lex functions */
static bool Fy_Lexer_lexSingleByteToken(Fy_Lexer *lexer);
static bool Fy_Lexer_lexAnyKeyword(Fy_Lexer *lexer);
static bool Fy_Lexer_lexNewline(Fy_Lexer *lexer);
static bool Fy_Lexer_lexConst(Fy_Lexer *lexer);
static bool Fy_Lexer_lexSymbol(Fy_Lexer *lexer);
static bool Fy_Lexer_lexString(Fy_Lexer *lexer);
static bool Fy_Lexer_lexChar(Fy_Lexer *lexer);

/* Define a type for single byte token definitions */
typedef struct Fy_SingleByteTokenDef {
    char character;
    Fy_TokenType type;
} Fy_SingleByteTokenDef;

/* Define a type for keyword token definitions */
typedef struct Fy_KeywordTokenDef {
    char *match_string;
    Fy_TokenType type;
} Fy_KeywordTokenDef;

/* Define a function type for the lexer parsing functions */
typedef bool (*Fy_LexerLexFunc)(Fy_Lexer *lexer);

/* Define all single byte token definitions */
const Fy_SingleByteTokenDef Fy_singleByteTokens[] = {
    { ':', Fy_TokenType_Colon },
    { '[', Fy_TokenType_LeftBracket },
    { ']', Fy_TokenType_RightBracket },
    { '(', Fy_TokenType_LeftParen },
    { ')', Fy_TokenType_RightParen },
    { '-', Fy_TokenType_Minus },
    { '+', Fy_TokenType_Plus },
    { '*', Fy_TokenType_Star },
    { '/', Fy_TokenType_Slash },
    { ',', Fy_TokenType_Comma },
    { '=', Fy_TokenType_EqualSign }
};

/* Define all keyword token definitions */
const Fy_KeywordTokenDef Fy_keywordTokens[] = {
    { "nop", Fy_TokenType_Nop },
    { "debug", Fy_TokenType_Debug },
    { "debugstack", Fy_TokenType_DebugStack },
    { "jmp", Fy_TokenType_Jmp },
    { "je", Fy_TokenType_Je },
    { "jne", Fy_TokenType_Jne },
    { "jz", Fy_TokenType_Jz },
    { "jnz", Fy_TokenType_Jnz },
    { "jb", Fy_TokenType_Jb },
    { "jbe", Fy_TokenType_Jbe },
    { "ja", Fy_TokenType_Ja },
    { "jae", Fy_TokenType_Jae },
    { "jl", Fy_TokenType_Jl },
    { "jle", Fy_TokenType_Jle },
    { "jg", Fy_TokenType_Jg },
    { "jge", Fy_TokenType_Jge },
    { "call", Fy_TokenType_Call },
    { "ret", Fy_TokenType_Ret },
    { "end", Fy_TokenType_End },
    { "mov", Fy_TokenType_Mov },
    { "lea", Fy_TokenType_Lea },
    { "add", Fy_TokenType_Add },
    { "sub", Fy_TokenType_Sub },
    { "and", Fy_TokenType_And },
    { "or", Fy_TokenType_Or },
    { "xor", Fy_TokenType_Xor },
    { "cmp", Fy_TokenType_Cmp },
    { "push", Fy_TokenType_Push },
    { "pop", Fy_TokenType_Pop },
    { "int", Fy_TokenType_Int },
    { "ax", Fy_TokenType_Ax },
    { "bx", Fy_TokenType_Bx },
    { "cx", Fy_TokenType_Cx },
    { "dx", Fy_TokenType_Dx },
    { "sp", Fy_TokenType_Sp },
    { "bp", Fy_TokenType_Bp },
    { "ah", Fy_TokenType_Ah },
    { "al", Fy_TokenType_Al },
    { "bh", Fy_TokenType_Bh },
    { "bl", Fy_TokenType_Bl },
    { "ch", Fy_TokenType_Ch },
    { "cl", Fy_TokenType_Cl },
    { "dh", Fy_TokenType_Dh },
    { "dl", Fy_TokenType_Dl },
    { "proc", Fy_TokenType_Proc },
    { "endp", Fy_TokenType_Endp },
    { "data", Fy_TokenType_Data },
    { "eb", Fy_TokenType_Eb },
    { "ew", Fy_TokenType_Ew },
    { "code", Fy_TokenType_Code },
    { "byte", Fy_TokenType_Byte },
    { "word", Fy_TokenType_Word },
    { "dup", Fy_TokenType_Dup }
};

/* Store pointers to all lexing functions */
const Fy_LexerLexFunc Fy_lexFuncs[] = {
    Fy_Lexer_lexSingleByteToken,
    Fy_Lexer_lexAnyKeyword,
    Fy_Lexer_lexNewline,
    Fy_Lexer_lexConst,
    Fy_Lexer_lexSymbol,
    Fy_Lexer_lexString,
    Fy_Lexer_lexChar
};

/* Returns whether the byte can be an identifier's start char */
static inline bool is_keyword_start_char(char c) {
    return isalpha(c) || c == '_';
}

/* Returns whether the byte can be an identifier's char in any place other than the start */
static inline bool is_keyword_char(char c) {
    return is_keyword_start_char(c) || isdigit(c);
}

// Returns whether a character is a binary character (1 or 0)
static inline bool is_bin_char(char c) {
    return c == '0' || c == '1';
}

/* Convert Fy_LexerError to string */
static char *Fy_LexerError_toString(Fy_LexerError error) {
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
static void Fy_Lexer_error(Fy_Lexer *lexer, Fy_LexerError error) {
    printf("LexerError[%zu,%zu]: %s\n",
            lexer->line, lexer->column,
            Fy_LexerError_toString(error));
    exit(1);
}

/*
 * Returns true if matched a keyword, and puts the keyword in the lexer's token.
 * Parameter `keyword` must be lowercase.
 */
static bool Fy_Lexer_matchKeyword(Fy_Lexer *lexer, char *keyword, Fy_TokenType type) {
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
static size_t Fy_Lexer_removeWhitespace(Fy_Lexer *lexer) {
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
static bool Fy_Lexer_lexConst(Fy_Lexer *lexer) {
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

static bool Fy_Lexer_lexSymbol(Fy_Lexer *lexer) {
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

static bool Fy_Lexer_lexString(Fy_Lexer *lexer) {
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
 * Returns false if not found newline.
 */
static bool Fy_Lexer_lexNewline(Fy_Lexer *lexer) {
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
 * Lex a single byte token (like '+' or ':').
 * Returns false if didn't find token.
 */
static bool Fy_Lexer_lexSingleByteToken(Fy_Lexer *lexer) {
    for (size_t i = 0; i < sizeof(Fy_singleByteTokens) / sizeof(Fy_SingleByteTokenDef); ++i) {
        const Fy_SingleByteTokenDef *token = &Fy_singleByteTokens[i];
        if (lexer->stream[0] == token->character) {
            lexer->token.start = lexer->stream;
            lexer->token.length = 1;
            lexer->token.type = token->type;
            ++lexer->stream;
            ++lexer->column;
            return true;
        }
    }
    return false;
}

/*
 * Lex a keyword from the keywords defined (like 'mov').
 * Returns false if didn't find token.
 */
static bool Fy_Lexer_lexAnyKeyword(Fy_Lexer *lexer) {
    for (size_t i = 0; i < sizeof(Fy_keywordTokens) / sizeof(Fy_KeywordTokenDef); ++i) {
        const Fy_KeywordTokenDef *token = &Fy_keywordTokens[i];
        if (Fy_Lexer_matchKeyword(lexer, token->match_string, token->type))
            return true;
    }
    return false;
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

    // Loop through all lex functions and return true if managed to lex using one of them
    for (size_t i = 0; i < sizeof(Fy_lexFuncs) / sizeof(Fy_LexerLexFunc); ++i) {
        Fy_LexerLexFunc func = Fy_lexFuncs[i];
        if (func(lexer))
            return true;
    }

    Fy_Lexer_error(lexer, Fy_LexerError_Syntax);
    return false; // Unreachable
}
