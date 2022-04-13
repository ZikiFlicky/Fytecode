#include "fy.h"

Fy_ParserParseRule Fy_parseRuleMovReg16Const = {
    .start_token = Fy_TokenType_Mov,
    .arg1 = {
        .type = Fy_ParserArgType_Reg16,
        .possible_tokens = NULL
    },
    .arg2 = {
        .type = Fy_ParserArgType_Constant
    },
    .func = Fy_ParseMovReg16Const
};

Fy_ParserParseRule Fy_parseRuleMovReg16Reg16 = {
    .start_token = Fy_TokenType_Mov,
    .arg1 = {
        .type = Fy_ParserArgType_Reg16,
        .possible_tokens = NULL
    },
    .arg2 = {
        .type = Fy_ParserArgType_Reg16,
        .possible_tokens = NULL
    },
    .func = Fy_ParseMovReg16Reg16
};

Fy_ParserParseRule *Fy_parserRules[] = {
    &Fy_parseRuleMovReg16Const,
    &Fy_parseRuleMovReg16Reg16
};

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
    case Fy_ParserError_InvalidInstructionParams:
        return "Invalid instruction parameters";
    default:
        FY_UNREACHABLE();
    }
}

Fy_ParserInstruction *Fy_ParserInstruction_New(Fy_ParserInstructionType type) {
    Fy_ParserInstruction *instruction = malloc(sizeof(Fy_ParserInstruction));
    instruction->type = type;
    return instruction;
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

Fy_ParserInstruction *Fy_ParseMovReg16Const(Fy_Parser *parser, Fy_Token *token_arg1, Fy_Token *token_arg2) {
    Fy_ParserInstruction *instruction = Fy_ParserInstruction_New(Fy_ParserInstructionType_MovReg16Const);
    instruction->mov_reg16_const.reg_id = Fy_TokenType_toReg16(token_arg1->type);
    instruction->mov_reg16_const.const16 = Fy_Token_toConst16(token_arg2, parser);
    return instruction;
}

Fy_ParserInstruction *Fy_ParseMovReg16Reg16(Fy_Parser *parser, Fy_Token *token_arg1, Fy_Token *token_arg2) {
    Fy_ParserInstruction *instruction = Fy_ParserInstruction_New(Fy_ParserInstructionType_MovReg16Reg16);
    (void)parser;
    instruction->mov_reg16_reg16.reg_id = Fy_TokenType_toReg16(token_arg1->type);
    instruction->mov_reg16_reg16.reg2_id = Fy_TokenType_toReg16(token_arg2->type);
    return instruction;
}

Fy_ParserInstruction *Fy_Parser_parseInstruction(Fy_Parser *parser) {
    Fy_ParserState start_backtrack, backtrack;
    Fy_TokenType start_token;

    Fy_Parser_dumpState(parser, &start_backtrack);

    // If you can't even lex, don't start parsing
    if (!Fy_Parser_lex(parser))
        return NULL;

    start_token = parser->token.type;

    Fy_Parser_dumpState(parser, &backtrack);

    for (size_t i = 0; i < sizeof(Fy_parserRules) / sizeof(Fy_ParserParseRule*); ++i) {
        Fy_ParserParseRule *rule = Fy_parserRules[i];
        Fy_Token token_arg1, token_arg2;

        if (start_token != rule->start_token)
            continue;

        if (!Fy_Parser_lex(parser))
            continue;

        if (!Fy_TokenType_isPossibleArg(parser->token.type, rule->arg1.type)) {
            Fy_Parser_loadState(parser, &backtrack);
            continue;
        }

        // If there is a token limit
        if (rule->arg1.possible_tokens) {
            bool found = false;
            for (size_t j = 0; !found && rule->arg1.possible_tokens[j] != 0; ++j) {
                if (parser->token.type == rule->arg1.possible_tokens[j]) {
                    found = true;
                }
            }
            // If the token didn't much any possible token, it is invalid
            if (!found) {
                Fy_Parser_loadState(parser, &backtrack);
                continue;
            }
        }

        token_arg1 = parser->token;

        if (!Fy_Parser_lex(parser)) {
            Fy_Parser_loadState(parser, &backtrack);
            continue;
        }

        if (!Fy_TokenType_isPossibleArg(parser->token.type, rule->arg2.type)) {
            Fy_Parser_loadState(parser, &backtrack);
            continue;
        }

        token_arg2 = parser->token;

        return rule->func(parser, &token_arg1, &token_arg2);
    }

    // TODO: Add clever error message that tells us why we were wrong

    Fy_Parser_error(parser, Fy_ParserError_InvalidInstructionParams);

    // switch (parser->token.type) {
    // case Fy_TokenType_Mov:
    //     if (!Fy_Parser_lex(parser))
    //         Fy_Parser_error(parser, Fy_ParserError_UnexpectedEof);

    //     if (Fy_TokenType_isReg16(parser->token.type)) {
    //         Fy_ParserReg16 reg = Fy_TokenType_toReg16(parser->token.type);

    //         // If we can't lex after the `mov reg` show error
    //         if (!Fy_Parser_lex(parser))
    //             Fy_Parser_error(parser, Fy_ParserError_UnexpectedEof);

    //         // TODO: Check for isReg8 and tell that 8-bit register can't be moved to 16-bit register

    //         if (parser->token.type == Fy_TokenType_Const) {
    //             int16_t c = Fy_Token_toConst16(&parser->token, parser);
    //             instruction = Fy_ParserInstruction_New(Fy_ParserInstructionType_MovReg16Const);
    //             instruction->mov_reg16_const.reg_id = reg;
    //             instruction->mov_reg16_const.const16 = c;
    //         } else {
    //             Fy_Parser_error(parser, Fy_ParserError_UnexpectedToken);
    //         }

    //         // if (Fy_TokenType_isReg16(parser->token.type)) {
    //         //     instruction = Fy_ParserInstruction_New(Fy_ParserInstructionType_)
    //         // }
    //     } else if (Fy_TokenType_isReg8(parser->token.type)) {
    //         // TODO: implement this
    //         FY_UNREACHABLE();
    //     } else {
    //         Fy_Parser_error(parser, Fy_ParserError_ExpectedReg);
    //     }

    //     break;
    // default:
    //     Fy_Parser_error(parser, Fy_ParserError_UnexpectedToken);
    // }

    FY_UNREACHABLE();
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
        case Fy_ParserInstructionType_MovReg16Reg16:
            printf("mov %s %s",
                    Fy_ParserReg16_toString(instruction->mov_reg16_reg16.reg_id),
                    Fy_ParserReg16_toString(instruction->mov_reg16_reg16.reg2_id));
            break;
        default:
            FY_UNREACHABLE();
        }
        printf("\n");
    }
}

/* Generate bytecode from parsed values */
void Fy_Parser_generateBytecode(Fy_Parser *parser, Fy_Generator *out) {
    Fy_Generator generator;
    Fy_Generator_Init(&generator);

    for (size_t i = 0; i < parser->amount_used; ++i) {
        Fy_ParserInstruction *instruction = parser->instructions[i];
        Fy_Generator_addInstruction(&generator, instruction);
    }

    *out = generator;
}

void Fy_Parser_generateToFile(Fy_Parser *parser, char *filename) {
    FILE *file = fopen(filename, "w+");
    Fy_Generator generator;

    Fy_Parser_generateBytecode(parser, &generator);
    for (size_t i = 0; i < generator.idx; ++i) {
        if (i != 0)
            printf(" ");
        printf("%x", generator.output[i]);
    }
    printf("\n");
    fwrite(generator.output, 1, generator.idx, file);

    fclose(file);
}
