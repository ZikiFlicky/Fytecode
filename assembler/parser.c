#include "fy.h"

/* Parse-function declarations */
static Fy_Instruction *Fy_ParseMovReg16Const(Fy_Parser *parser, Fy_Token *token_arg1, Fy_Token *token_arg2);
static Fy_Instruction *Fy_ParseMovReg16Reg16(Fy_Parser *parser, Fy_Token *token_arg1, Fy_Token *token_arg2);
static Fy_Instruction *Fy_ParseDebug(Fy_Parser *parser);
static Fy_Instruction *Fy_ParseEnd(Fy_Parser *parser);
static Fy_Instruction *Fy_ParseJmp(Fy_Parser *parser, Fy_Token *token_arg);

/* Process-function declarations */
static void Fy_ProcessJmp(Fy_Parser *parser, Fy_Instruction_Jmp *instruction);

/* Define rules */
Fy_ParserParseRule Fy_parseRuleMovReg16Const = {
    .type = Fy_ParserParseRuleType_TwoParams,
    .start_token = Fy_TokenType_Mov,
    .arg1 = {
        .type = Fy_ParserArgType_Reg16,
        .possible_tokens = NULL
    },
    .arg2 = {
        .type = Fy_ParserArgType_Constant
    },
    .func_two_params = Fy_ParseMovReg16Const,
    .func_process = NULL
};
Fy_ParserParseRule Fy_parseRuleMovReg16Reg16 = {
    .type = Fy_ParserParseRuleType_TwoParams,
    .start_token = Fy_TokenType_Mov,
    .arg1 = {
        .type = Fy_ParserArgType_Reg16,
        .possible_tokens = NULL
    },
    .arg2 = {
        .type = Fy_ParserArgType_Reg16,
        .possible_tokens = NULL
    },
    .func_two_params = Fy_ParseMovReg16Reg16,
    .func_process = NULL
};
Fy_ParserParseRule Fy_parseRuleDebug = {
    .type = Fy_ParserParseRuleType_NoParams,
    .start_token = Fy_TokenType_Debug,
    .func_no_params = Fy_ParseDebug,
    .func_process = NULL
};
Fy_ParserParseRule Fy_parseRuleEnd = {
    .type = Fy_ParserParseRuleType_NoParams,
    .start_token = Fy_TokenType_End,
    .func_no_params = Fy_ParseEnd,
    .func_process = NULL
};
Fy_ParserParseRule Fy_parseRuleJmp = {
    .type = Fy_ParserParseRuleType_OneParam,
    .start_token = Fy_TokenType_Jmp,
    .arg1 = {
        .type = Fy_ParserArgType_Label
    },
    .func_one_param = Fy_ParseJmp,
    .func_process = (Fy_InstructionProcessFunc)Fy_ProcessJmp
};
/* Array that stores all rules (pointers to rules) */
Fy_ParserParseRule *Fy_parserRules[] = {
    &Fy_parseRuleMovReg16Const,
    &Fy_parseRuleMovReg16Reg16,
    &Fy_parseRuleDebug,
    &Fy_parseRuleEnd,
    &Fy_parseRuleJmp
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
    case Fy_ParserError_InvalidInstruction:
        return "Invalid instruction";
    case Fy_ParserError_SyntaxError:
        return "Syntax error";
    default:
        FY_UNREACHABLE();
    }
}

/* Initialize a parser instance */
void Fy_Parser_Init(Fy_Lexer *lexer, Fy_Parser *out) {
    out->lexer = lexer;
    out->amount_allocated = 0;
    out->amount_used = 0;
    out->code_offset = 0;
    Fy_Labelmap_Init(&out->labelmap);
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
    char *line_start;
    printf("ParserError[%zu,%zu]: %s\n",
            parser->lexer->line, parser->lexer->column,
            Fy_ParserError_toString(error));
    printf("| ");
    line_start = parser->lexer->stream + 1 - parser->lexer->column;
    for (size_t i = 0; line_start[i] != '\n' && line_start[i] != '\0'; ++i)
        putchar(line_start[i]);
    printf("\n| ");
    for (size_t i = 1; i < parser->lexer->column; ++i)
        putchar(' ');
    printf("^\n");
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

static Fy_Instruction *Fy_ParseMovReg16Const(Fy_Parser *parser, Fy_Token *token_arg1, Fy_Token *token_arg2) {
    Fy_Instruction_MovReg16Const *instruction = FY_INSTRUCTION_NEW(Fy_Instruction_MovReg16Const, Fy_InstructionType_MovReg16Const);
    instruction->reg_id = Fy_TokenType_toReg16(token_arg1->type);
    instruction->val = Fy_Token_toConst16(token_arg2, parser);
    return (Fy_Instruction*)instruction;
}

static Fy_Instruction *Fy_ParseMovReg16Reg16(Fy_Parser *parser, Fy_Token *token_arg1, Fy_Token *token_arg2) {
    Fy_Instruction_MovReg16Reg16 *instruction = FY_INSTRUCTION_NEW(Fy_Instruction_MovReg16Reg16, Fy_InstructionType_MovReg16Reg16);
    (void)parser;
    // FIXME: This should be a different register type (VMRegister instead of ParserRegister)
    instruction->reg_id = Fy_TokenType_toReg16(token_arg1->type);
    instruction->reg2_id = Fy_TokenType_toReg16(token_arg2->type);
    return (Fy_Instruction*)instruction;
}

static Fy_Instruction *Fy_ParseDebug(Fy_Parser *parser) {
    Fy_Instruction *instruction = FY_INSTRUCTION_NEW(Fy_Instruction, Fy_InstructionType_Debug);
    (void)parser;
    return instruction;
}

static Fy_Instruction *Fy_ParseEnd(Fy_Parser *parser) {
    Fy_Instruction *instruction = FY_INSTRUCTION_NEW(Fy_Instruction, Fy_InstructionType_EndProgram);
    (void)parser;
    return instruction;
}

static Fy_Instruction *Fy_ParseJmp(Fy_Parser *parser, Fy_Token *token_arg) {
    Fy_Instruction_Jmp *instruction = FY_INSTRUCTION_NEW(Fy_Instruction_Jmp, Fy_InstructionType_Jmp);
    (void)parser;
    instruction->name = Fy_Token_toLowercaseCStr(token_arg);
    return (Fy_Instruction*)instruction;
}

static void Fy_ProcessJmp(Fy_Parser *parser, Fy_Instruction_Jmp *instruction) {
    uint16_t address;
    if (!Fy_Labelmap_getEntry(&parser->labelmap, instruction->name, &address)) {
        FY_UNREACHABLE();
        // Fy_Parser_error(parser, Fy_ParserError_LabelNotFound);
    }
    instruction->address = address;
}

static Fy_Instruction *Fy_Parser_parseInstruction(Fy_Parser *parser) {
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

        // If we take no params
        if (rule->type == Fy_ParserParseRuleType_NoParams) {
            Fy_Instruction *instruction = rule->func_no_params(parser);
            instruction->parse_rule = rule;
            return instruction;
        }

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

        if (rule->type == Fy_ParserParseRuleType_OneParam) {
            Fy_Instruction *instruction = rule->func_one_param(parser, &token_arg1);
            instruction->parse_rule = rule;
            return instruction;
        }

        if (!Fy_Parser_lex(parser)) {
            Fy_Parser_loadState(parser, &backtrack);
            continue;
        }

        if (!Fy_TokenType_isPossibleArg(parser->token.type, rule->arg2.type)) {
            Fy_Parser_loadState(parser, &backtrack);
            continue;
        }

        token_arg2 = parser->token;

        if (rule->type == Fy_ParserParseRuleType_TwoParams) {
            Fy_Instruction *instruction = rule->func_two_params(parser, &token_arg1, &token_arg2);
            instruction->parse_rule = rule;
            return instruction;
        }

        FY_UNREACHABLE();
    }

    // TODO: Add clever error message that tells us why we were wrong

    Fy_Parser_error(parser, Fy_ParserError_InvalidInstruction);

    FY_UNREACHABLE();
}

/* Returns whether we parsed a label */
bool Fy_Parser_parseLabel(Fy_Parser *parser) {
    Fy_Token label_token;
    char *label_string;

    if (!Fy_Parser_match(parser, Fy_TokenType_Label))
        return false;
    label_token = parser->token;

    if (!Fy_Parser_match(parser, Fy_TokenType_Colon))
        Fy_Parser_error(parser, Fy_ParserError_SyntaxError);

    label_string = Fy_Token_toLowercaseCStr(&label_token);
    Fy_Labelmap_addEntry(&parser->labelmap, label_string, parser->code_offset);

    return true;
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

/* Step 1: reading all of the instructions and creating a vector of them */
static void Fy_Parser_readInstructions(Fy_Parser *parser) {
    // If there is a newline, advance it
    Fy_Parser_expectNewline(parser, false);

    for (;;) {
        Fy_Instruction *instruction;
        Fy_ParserState backtrack;

        if (Fy_Parser_parseLabel(parser))
            continue;

        Fy_Parser_expectNewline(parser, false);

        if ((instruction = Fy_Parser_parseInstruction(parser))) {
            if (parser->amount_allocated == 0)
                parser->instructions = malloc((parser->amount_allocated = 8) * sizeof(Fy_Instruction*));
            else if (parser->amount_used == parser->amount_allocated)
                parser->instructions = realloc(parser->instructions, (parser->amount_allocated += 8) * sizeof(Fy_Instruction*));

            parser->instructions[parser->amount_used++] = instruction;
            // Update offset
            parser->code_offset += 1 + instruction->type->additional_size;
            Fy_Parser_expectNewline(parser, true);
            continue;
        }

        Fy_Parser_dumpState(parser, &backtrack);
        if (!Fy_Parser_lex(parser)) {
            break;
        } else {
            Fy_Parser_loadState(parser, &backtrack);
            Fy_Parser_error(parser, Fy_ParserError_SyntaxError);
        }
    }
}


/* Step 2: processing parsed instructions */
static void Fy_Parser_processInstructions(Fy_Parser *parser) {
    for (size_t i = 0; i < parser->amount_used; ++i) {
        Fy_Instruction *instruction = parser->instructions[i];
        if (instruction->parse_rule->func_process) {
            instruction->parse_rule->func_process(parser, instruction);
        }
    }
}

void Fy_Parser_parseAll(Fy_Parser *parser) {
    Fy_Parser_readInstructions(parser);
    Fy_Parser_processInstructions(parser);
}

void Fy_Parser_logParsed(Fy_Parser *parser) {
    (void)parser;
    FY_UNREACHABLE();
    // printf("--- %zu instructions ---\n", parser->amount_used);
    // for (size_t i = 0; i < parser->amount_used; ++i) {
    //     Fy_Instruction *instruction = parser->instructions[i];
    //     switch (instruction->type) {
    //     case Fy_InstructionType_MovReg16Const:
    //         printf("mov %s %d",
    //                 Fy_ParserReg16_toString(instruction->mov_reg16_const.reg_id),
    //                 instruction->mov_reg16_const.const16);
    //         break;
    //     case Fy_InstructionType_MovReg16Reg16:
    //         printf("mov %s %s",
    //                 Fy_ParserReg16_toString(instruction->mov_reg16_reg16.reg_id),
    //                 Fy_ParserReg16_toString(instruction->mov_reg16_reg16.reg2_id));
    //         break;
    //     default:
    //         FY_UNREACHABLE();
    //     }
    //     printf("\n");
    // }
}

/* Generate bytecode from parsed values */
void Fy_Parser_generateBytecode(Fy_Parser *parser, Fy_Generator *out) {
    Fy_Generator generator;
    Fy_Generator_Init(&generator);

    for (size_t i = 0; i < parser->amount_used; ++i) {
        Fy_Instruction *instruction = parser->instructions[i];
        Fy_Generator_addInstruction(&generator, instruction);
    }

    *out = generator;
}

void Fy_Parser_generateToFile(Fy_Parser *parser, char *filename) {
    FILE *file = fopen(filename, "w+");
    Fy_Generator generator;

    // FIXME:
    if (!file)
        FY_UNREACHABLE();

    Fy_Parser_generateBytecode(parser, &generator);
    fwrite(generator.output, 1, generator.idx, file);

    fclose(file);
}
