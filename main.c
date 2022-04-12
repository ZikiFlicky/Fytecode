#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <assert.h>

#define FY_UNREACHABLE() assert(0)

/* --- Token structures --- */

typedef enum Fy_TokenType {
    Fy_TokenType_Mov = 1,
    Fy_TokenType_Ax,
    Fy_TokenType_Bx,
    Fy_TokenType_Const,
    Fy_TokenType_Newline
} Fy_TokenType;

typedef struct Fy_Token {
    Fy_TokenType type;
    char *start;
    size_t length;
} Fy_Token;

/* --- Lexer structures --- */

typedef struct Fy_Lexer {
    char *stream_base, *stream;
    Fy_Token token;
    size_t line, column;
} Fy_Lexer;

typedef enum Fy_LexerError {
    Fy_LexerError_Syntax = 1,
} Fy_LexerError;

/* --- Parser structures --- */

typedef struct Fy_ParserState {
    char *stream;
    size_t line, column;
} Fy_ParserState;

typedef enum Fy_ParserInstructionType {
    Fy_ParserInstructionType_MovReg8Const = 1,
    Fy_ParserInstructionType_MovReg8Mem,
    Fy_ParserInstructionType_MovReg16Const,
    Fy_ParserInstructionType_MovReg16Mem
} Fy_ParserInstructionType;

typedef enum Fy_ParserReg16 {
    Fy_ParserReg16_Ax = 1,
    Fy_ParserReg16_Bx
} Fy_ParserReg16;

typedef struct Fy_ParserInstruction {
    Fy_ParserInstructionType type;
    union {
        struct {
            Fy_ParserReg16 reg_id;
            uint8_t const8;
        } mov_reg8_const;
        struct {
            Fy_ParserReg16 reg_id;
            uint16_t const16;
        } mov_reg16_const;
    };
} Fy_ParserInstruction;

typedef enum Fy_ParserError {
    Fy_ParserError_UnexpectedToken = 1,
    Fy_ParserError_UnexpectedEof,
    Fy_ParserError_ExpectedReg,
    Fy_ParserError_ConstTooBig,
    Fy_ParserError_ExpectedNewline
} Fy_ParserError;

typedef struct Fy_Parser {
    Fy_Lexer *lexer;
    Fy_Token token;

    size_t amount_used, amount_allocated;
    Fy_ParserInstruction **instructions;
} Fy_Parser;

/* ----- Misc ----- */

/*
 * Reads all of the file into memory and returns the pointer to that memory.
 * Returns a NULL on failure.
 */
char *Fy_LoadFile(char *name) {
    FILE *file;
    size_t length;
    char *stream;

    file = fopen(name, "r");
    if (!file)
        return NULL;

    fseek(file, 0, SEEK_END);
    length = ftell(file);
    fseek(file, 0, SEEK_SET);

    stream = malloc((length + 1) * sizeof(char));
    fread(stream, sizeof(char), length, file);
    fclose(file);

    return stream;
}

/* ----- Lexer functions ----- */

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
    do {
        ++lexer->stream;
        ++lexer->line;
        ++lexer->token.length;
        Fy_Lexer_removeWhitespace(lexer);
    } while (lexer->stream[0] == '\n');
    lexer->column = 1;
    return true;
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

/* ----- Parser functions ----- */

/* Initialize a parser instance */
void Fy_Parser_Init(Fy_Lexer *lexer, Fy_Parser *out) {
    out->lexer = lexer;
    out->amount_allocated = 0;
    out->amount_used = 0;
}

void Fy_Parser_dumpState(Fy_Parser *parser, Fy_ParserState *out_state) {
    out_state->stream = parser->lexer->stream;
    out_state->line = parser->lexer->line;
    out_state->column = parser->lexer->column;
}

void Fy_Parser_loadState(Fy_Parser *parser, Fy_ParserState *state) {
    parser->lexer->stream = state->stream;
    parser->lexer->line = state->line;
    parser->lexer->column = state->column;
}

bool Fy_Parser_lex(Fy_Parser *parser) {
    if (Fy_Lexer_lex(parser->lexer)) {
        parser->token = parser->lexer->token;
        return true;
    } else {
        return false;
    }
}

char *Fy_ParserError_toString(Fy_ParserError error) {
    switch (error) {
    case Fy_ParserError_UnexpectedToken:
        return "Unexpected token";
    case Fy_ParserError_UnexpectedEof:
        return "Unexpected EOF";
    case Fy_ParserError_ExpectedReg:
        return "Expected register";
    case Fy_ParserError_ConstTooBig:
        return "Constant too big";
    case Fy_ParserError_ExpectedNewline:
        return "Expected newline";
    default:
        FY_UNREACHABLE();
    }
}

void Fy_Parser_error(Fy_Parser *parser, Fy_ParserError error) {
    printf("ParserError[%zu,%zu]: %s\n",
            parser->lexer->line, parser->lexer->column,
            Fy_ParserError_toString(error));
    exit(1);
}

/* Returns a boolean specifying whether a token of the given type was found. */
bool Fy_Parser_match(Fy_Parser *parser, Fy_TokenType type) {
    Fy_ParserState backtrack;
    Fy_Parser_dumpState(parser, &backtrack);

    if (!Fy_Parser_lex(parser) || parser->token.type != type) {
        Fy_Parser_loadState(parser, &backtrack);
        return false;
    }

    return true;
}

bool Fy_TokenType_isReg8(Fy_TokenType type) {
    switch (type) {
    default:
        return false;
    }
}

bool Fy_TokenType_isReg16(Fy_TokenType type) {
    switch (type) {
    case Fy_TokenType_Ax:
    case Fy_TokenType_Bx:
        return true;
    default:
        return false;
    }
}

Fy_ParserReg16 Fy_TokenType_toReg16(Fy_TokenType type) {
    switch (type) {
    case Fy_TokenType_Ax:
        return Fy_ParserReg16_Ax;
    case Fy_TokenType_Bx:
        return Fy_ParserReg16_Bx;
    default:
        FY_UNREACHABLE();
    }
}

Fy_ParserInstruction *Fy_ParserInstruction_new(Fy_ParserInstructionType type) {
    Fy_ParserInstruction *instruction = malloc(sizeof(Fy_ParserInstruction));
    instruction->type = type;
    return instruction;
}

/* Convert Fy_Token with type Fy_TokenType_Const to 16-bit integer */
int16_t Fy_Token_toConst16(Fy_Token *token, Fy_Parser *parser) {
    bool is_neg;
    size_t i = 0;
    size_t positive = 0;

    assert(token->type == Fy_TokenType_Const);

    if (token->start[0] == '-') {
        is_neg = true;
        ++i;
    }

    do {
        positive *= 10;
        positive += token->start[i] - '0';

        if (positive >= (1 << 15))
            Fy_Parser_error(parser, Fy_ParserError_ConstTooBig);

        ++i;
    } while (i < token->length);

    return (int16_t)((is_neg ? -1 : 1) * positive);
}

/* Convert Fy_Token with type Fy_TokenType_Const to 8-bit integer */
int8_t Fy_Token_toConst8(Fy_Token *token, Fy_Parser *parser) {
    bool is_neg;
    size_t i = 0;
    size_t positive = 0;

    assert(token->type == Fy_TokenType_Const);

    if (token->start[0] == '-') {
        is_neg = true;
        ++i;
    }

    do {
        positive *= 10;
        positive += token->start[i] - '0';

        if (positive >= (1 << 7))
            Fy_Parser_error(parser, Fy_ParserError_ConstTooBig);

        ++i;
    } while (i < token->length);

    return (int8_t)((is_neg ? -1 : 1) * positive);
}

Fy_ParserInstruction *Fy_Parser_parseInstruction(Fy_Parser *parser) {
    Fy_ParserInstruction *instruction;

    // If you can't even lex, don't start parsing
    if (!Fy_Parser_lex(parser))
        return NULL;

    switch (parser->token.type) {
    case Fy_TokenType_Mov:
        if (!Fy_Parser_lex(parser))
            Fy_Parser_error(parser, Fy_ParserError_UnexpectedEof);

        if (Fy_TokenType_isReg16(parser->token.type)) {
            Fy_ParserReg16 reg = Fy_TokenType_toReg16(parser->token.type);

            // If we can't lex after the `mov reg` show error
            if (!Fy_Parser_lex(parser))
                Fy_Parser_error(parser, Fy_ParserError_UnexpectedEof);

            // TODO: Check for isReg8 and tell that 8-bit register can't be moved to 16-bit register

            if (parser->token.type == Fy_TokenType_Const) {
                int16_t c = Fy_Token_toConst16(&parser->token, parser);
                instruction = Fy_ParserInstruction_new(Fy_ParserInstructionType_MovReg16Const);
                instruction->mov_reg16_const.reg_id = reg;
                instruction->mov_reg16_const.const16 = c;
            } else {
                Fy_Parser_error(parser, Fy_ParserError_UnexpectedToken);
            }

            // if (Fy_TokenType_isReg16(parser->token.type)) {
            //     instruction = Fy_ParserInstruction_new(Fy_ParserInstructionType_)
            // }
        } else if (Fy_TokenType_isReg8(parser->token.type)) {
            // TODO: implement this
            FY_UNREACHABLE();
        } else {
            Fy_Parser_error(parser, Fy_ParserError_ExpectedReg);
        }

        break;
    default:
        Fy_Parser_error(parser, Fy_ParserError_UnexpectedToken);
    }

    return instruction;
}

void Fy_Parser_expectNewline(Fy_Parser *parser, bool do_error) {
    Fy_ParserState state;
    Fy_Parser_dumpState(parser, &state);

    if (!Fy_Parser_lex(parser))
        return; // Eof = Eol for us
    if (parser->token.type == Fy_TokenType_Newline)
        return;

    // Load last state if we didn't get a newline
    Fy_Parser_loadState(parser, &state);
    if (do_error)
        Fy_Parser_error(parser, Fy_ParserError_ExpectedNewline);
}

void Fy_Parser_parseAll(Fy_Parser *parser) {
    Fy_ParserInstruction *instruction;

    // If there is a newline, advance it
    Fy_Parser_expectNewline(parser, false);

    while ((instruction = Fy_Parser_parseInstruction(parser))) {
        if (parser->amount_allocated == 0)
            parser->instructions = malloc((parser->amount_allocated = 8) * sizeof(Fy_ParserInstruction*));
        else if (parser->amount_used == parser->amount_allocated)
            parser->instructions = realloc(parser->instructions, (parser->amount_allocated += 8) * sizeof(Fy_ParserInstruction*));

        parser->instructions[parser->amount_used++] = instruction;
        Fy_Parser_expectNewline(parser, true);
    }
}

/* Returns lowercase string representation of 16-bit register */
char *Fy_ParserReg16_toString(Fy_ParserReg16 reg) {
    switch (reg) {
    case Fy_ParserReg16_Ax:
        return "ax";
    case Fy_ParserReg16_Bx:
        return "bx";
    default:
        FY_UNREACHABLE();
    }
}

void Fy_Parser_logParsed(Fy_Parser *parser) {
    printf("--- %zu instructions ---\n", parser->amount_used);
    for (size_t i = 0; i < parser->amount_used; ++i) {
        Fy_ParserInstruction *instruction = parser->instructions[i];
        switch (instruction->type) {
        case Fy_ParserInstructionType_MovReg16Const:
            printf("mov %s %d",
                    Fy_ParserReg16_toString(instruction->mov_reg16_const.reg_id),
                    instruction->mov_reg16_const.const16);
            break;
        default:
            FY_UNREACHABLE();
        }
        printf("\n");
    }
}

int main(int argc, char **argv) {
    char *stream;
    Fy_Lexer lexer;
    Fy_Parser parser;

    if (argc != 2) {
        printf("Expected 1 argument\n");
        return 1;
    }

    stream = Fy_LoadFile(argv[1]);
    if (!stream) {
        printf("Failed in opening %s\n", argv[1]);
        return 1;
    }

    Fy_Lexer_Init(&lexer, stream);
    Fy_Parser_Init(&lexer, &parser);
    Fy_Parser_parseAll(&parser);

    Fy_Parser_logParsed(&parser);

    // printf("%zu\n", parser.amount_used);

    // while (Fy_Lexer_lex(&lexer))
    //     printf("%d\n", lexer.token.type);

    free(stream);

    return 0;
}
