#include "fy.h"

/* Parse-function (step 1) declarations */
static Fy_Instruction *Fy_ParseNop(Fy_Parser *parser);
static Fy_Instruction *Fy_ParseMovReg8Const(Fy_Parser *parser, Fy_Token *token_arg1, Fy_Token *token_arg2);
static Fy_Instruction *Fy_ParseMovReg8Reg8(Fy_Parser *parser, Fy_Token *token_arg1, Fy_Token *token_arg2);
static Fy_Instruction *Fy_ParseMovReg16Const(Fy_Parser *parser, Fy_Token *token_arg1, Fy_Token *token_arg2);
static Fy_Instruction *Fy_ParseMovReg16Reg16(Fy_Parser *parser, Fy_Token *token_arg1, Fy_Token *token_arg2);
static Fy_Instruction *Fy_ParseDebug(Fy_Parser *parser);
static Fy_Instruction *Fy_ParseEnd(Fy_Parser *parser);
static Fy_Instruction *Fy_ParseAddReg16Const(Fy_Parser *parser, Fy_Token *token_arg1, Fy_Token *token_arg2);
static Fy_Instruction *Fy_ParseAddReg16Reg16(Fy_Parser *parser, Fy_Token *token_arg1, Fy_Token *token_arg2);
static Fy_Instruction *Fy_ParseSubReg16Const(Fy_Parser *parser, Fy_Token *token_arg1, Fy_Token *token_arg2);
static Fy_Instruction *Fy_ParseSubReg16Reg16(Fy_Parser *parser, Fy_Token *token_arg1, Fy_Token *token_arg2);
static Fy_Instruction *Fy_ParseCmpReg16Const(Fy_Parser *parser, Fy_Token *token_arg1, Fy_Token *token_arg2);
static Fy_Instruction *Fy_ParseCmpReg16Reg16(Fy_Parser *parser, Fy_Token *token_arg1, Fy_Token *token_arg2);
static Fy_Instruction *Fy_ParseJmp(Fy_Parser *parser, Fy_Token *token_arg);
static Fy_Instruction *Fy_ParseJe(Fy_Parser *parser, Fy_Token *token_arg);
static Fy_Instruction *Fy_ParseJl(Fy_Parser *parser, Fy_Token *token_arg);
static Fy_Instruction *Fy_ParseJg(Fy_Parser *parser, Fy_Token *token_arg);
static Fy_Instruction *Fy_ParseCall(Fy_Parser *parser, Fy_Token *token_arg);
static Fy_Instruction *Fy_ParseRet(Fy_Parser *parser);
static Fy_Instruction *Fy_ParseRetConst16(Fy_Parser *parser, Fy_Token *token_arg);
static Fy_Instruction *Fy_ParsePushConst(Fy_Parser *parser, Fy_Token *token_arg);
static Fy_Instruction *Fy_ParsePushReg16(Fy_Parser *parser, Fy_Token *token_arg);
static Fy_Instruction *Fy_ParsePop(Fy_Parser *parser, Fy_Token *token_arg);

/* Process-function (parsing step 2) declarations */
static void Fy_ProcessOpLabel(Fy_Parser *parser, Fy_Instruction_OpLabel *instruction);

/* Function to parse anything found in text */
static bool Fy_Parser_parseLine(Fy_Parser *parser);

/* Define rules */
Fy_ParserParseRule Fy_parseRuleNop = {
    .type = Fy_ParserParseRuleType_NoParams,
    .start_token = Fy_TokenType_Nop,
    .func_no_params = Fy_ParseNop,
    .func_process = NULL
};
Fy_ParserParseRule Fy_parseRuleMovReg8Const = {
    .type = Fy_ParserParseRuleType_TwoParams,
    .start_token = Fy_TokenType_Mov,
    .arg1 = {
        .type = Fy_ParserArgType_Reg8,
        .possible_tokens = NULL
    },
    .arg2 = {
        .type = Fy_ParserArgType_Constant
    },
    .func_two_params = Fy_ParseMovReg8Const,
    .func_process = NULL
};
Fy_ParserParseRule Fy_parseRuleMovReg8Reg8 = {
    .type = Fy_ParserParseRuleType_TwoParams,
    .start_token = Fy_TokenType_Mov,
    .arg1 = {
        .type = Fy_ParserArgType_Reg8,
        .possible_tokens = NULL
    },
    .arg2 = {
        .type = Fy_ParserArgType_Reg8,
        .possible_tokens = NULL
    },
    .func_two_params = Fy_ParseMovReg8Reg8,
    .func_process = NULL
};
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
Fy_ParserParseRule Fy_parseRuleAddReg16Const = {
    .type = Fy_ParserParseRuleType_TwoParams,
    .start_token = Fy_TokenType_Add,
    .arg1 = {
        .type = Fy_ParserArgType_Reg16,
        .possible_tokens = NULL
    },
    .arg2 = {
        .type = Fy_ParserArgType_Constant
    },
    .func_two_params = Fy_ParseAddReg16Const,
    .func_process = NULL
};
Fy_ParserParseRule Fy_parseRuleAddReg16Reg16 = {
    .type = Fy_ParserParseRuleType_TwoParams,
    .start_token = Fy_TokenType_Add,
    .arg1 = {
        .type = Fy_ParserArgType_Reg16,
        .possible_tokens = NULL
    },
    .arg2 = {
        .type = Fy_ParserArgType_Reg16,
        .possible_tokens = NULL
    },
    .func_two_params = Fy_ParseAddReg16Reg16,
    .func_process = NULL
};
Fy_ParserParseRule Fy_parseRuleSubReg16Const = {
    .type = Fy_ParserParseRuleType_TwoParams,
    .start_token = Fy_TokenType_Sub,
    .arg1 = {
        .type = Fy_ParserArgType_Reg16,
        .possible_tokens = NULL
    },
    .arg2 = {
        .type = Fy_ParserArgType_Constant
    },
    .func_two_params = Fy_ParseSubReg16Const,
    .func_process = NULL
};
Fy_ParserParseRule Fy_parseRuleSubReg16Reg16 = {
    .type = Fy_ParserParseRuleType_TwoParams,
    .start_token = Fy_TokenType_Sub,
    .arg1 = {
        .type = Fy_ParserArgType_Reg16,
        .possible_tokens = NULL
    },
    .arg2 = {
        .type = Fy_ParserArgType_Reg16,
        .possible_tokens = NULL
    },
    .func_two_params = Fy_ParseSubReg16Reg16,
    .func_process = NULL
};
Fy_ParserParseRule Fy_parseRuleCmpReg16Const = {
    .type = Fy_ParserParseRuleType_TwoParams,
    .start_token = Fy_TokenType_Cmp,
    .arg1 = {
        .type = Fy_ParserArgType_Reg16,
        .possible_tokens = NULL
    },
    .arg2 = {
        .type = Fy_ParserArgType_Constant
    },
    .func_two_params = Fy_ParseCmpReg16Const,
    .func_process = NULL
};
Fy_ParserParseRule Fy_parseRuleCmpReg16Reg16 = {
    .type = Fy_ParserParseRuleType_TwoParams,
    .start_token = Fy_TokenType_Cmp,
    .arg1 = {
        .type = Fy_ParserArgType_Reg16,
        .possible_tokens = NULL
    },
    .arg2 = {
        .type = Fy_ParserArgType_Reg16,
        .possible_tokens = NULL
    },
    .func_two_params = Fy_ParseCmpReg16Reg16,
    .func_process = NULL
};
Fy_ParserParseRule Fy_parseRuleJmp = {
    .type = Fy_ParserParseRuleType_OneParam,
    .start_token = Fy_TokenType_Jmp,
    .arg1 = {
        .type = Fy_ParserArgType_Label
    },
    .func_one_param = Fy_ParseJmp,
    .func_process = (Fy_InstructionProcessFunc)Fy_ProcessOpLabel
};
Fy_ParserParseRule Fy_parseRuleJe = {
    .type = Fy_ParserParseRuleType_OneParam,
    .start_token = Fy_TokenType_Je,
    .arg1 = {
        .type = Fy_ParserArgType_Label
    },
    .func_one_param = Fy_ParseJe,
    .func_process = (Fy_InstructionProcessFunc)Fy_ProcessOpLabel
};
Fy_ParserParseRule Fy_parseRuleJl = {
    .type = Fy_ParserParseRuleType_OneParam,
    .start_token = Fy_TokenType_Jl,
    .arg1 = {
        .type = Fy_ParserArgType_Label
    },
    .func_one_param = Fy_ParseJl,
    .func_process = (Fy_InstructionProcessFunc)Fy_ProcessOpLabel
};
Fy_ParserParseRule Fy_parseRuleJg = {
    .type = Fy_ParserParseRuleType_OneParam,
    .start_token = Fy_TokenType_Jg,
    .arg1 = {
        .type = Fy_ParserArgType_Label
    },
    .func_one_param = Fy_ParseJg,
    .func_process = (Fy_InstructionProcessFunc)Fy_ProcessOpLabel
};
Fy_ParserParseRule Fy_parseRulePushConst = {
    .type = Fy_ParserParseRuleType_OneParam,
    .start_token = Fy_TokenType_Push,
    .arg1 = {
        .type = Fy_ParserArgType_Constant
    },
    .func_one_param = Fy_ParsePushConst,
    .func_process = NULL
};
Fy_ParserParseRule Fy_parseRulePushReg16 = {
    .type = Fy_ParserParseRuleType_OneParam,
    .start_token = Fy_TokenType_Push,
    .arg1 = {
        .type = Fy_ParserArgType_Reg16,
        .possible_tokens = NULL
    },
    .func_one_param = Fy_ParsePushReg16,
    .func_process = NULL
};
Fy_ParserParseRule Fy_parseRulePop = {
    .type = Fy_ParserParseRuleType_OneParam,
    .start_token = Fy_TokenType_Pop,
    .arg1 = {
        .type = Fy_ParserArgType_Reg16,
        .possible_tokens = NULL
    },
    .func_one_param = Fy_ParsePop,
    .func_process = NULL
};
Fy_ParserParseRule Fy_parseRuleCall = {
    .type = Fy_ParserParseRuleType_OneParam,
    .start_token = Fy_TokenType_Call,
    .arg1 = {
        .type = Fy_ParserArgType_Label,
        .possible_tokens = NULL
    },
    .func_one_param = Fy_ParseCall,
    .func_process = (Fy_InstructionProcessFunc)Fy_ProcessOpLabel
};
Fy_ParserParseRule Fy_parseRuleRet = {
    .type = Fy_ParserParseRuleType_NoParams,
    .start_token = Fy_TokenType_Ret,
    .func_no_params = Fy_ParseRet,
    .func_process = NULL
};
Fy_ParserParseRule Fy_parseRuleRetConst16 = {
    .type = Fy_ParserParseRuleType_OneParam,
    .start_token = Fy_TokenType_Ret,
    .arg1 = {
        .type = Fy_ParserArgType_Constant
    },
    .func_one_param = Fy_ParseRetConst16,
    .func_process = NULL
};

/* Array that stores all rules (pointers to rules) */
Fy_ParserParseRule *Fy_parserRules[] = {
    &Fy_parseRuleNop,
    &Fy_parseRuleMovReg8Const,
    &Fy_parseRuleMovReg8Reg8,
    &Fy_parseRuleMovReg16Const,
    &Fy_parseRuleMovReg16Reg16,
    &Fy_parseRuleAddReg16Const,
    &Fy_parseRuleAddReg16Reg16,
    &Fy_parseRuleSubReg16Const,
    &Fy_parseRuleSubReg16Reg16,
    &Fy_parseRuleCmpReg16Const,
    &Fy_parseRuleCmpReg16Reg16,
    &Fy_parseRuleDebug,
    &Fy_parseRuleEnd,
    &Fy_parseRuleJmp,
    &Fy_parseRuleJe,
    &Fy_parseRuleJl,
    &Fy_parseRuleJg,
    &Fy_parseRulePushConst,
    &Fy_parseRulePushReg16,
    &Fy_parseRulePop,
    &Fy_parseRuleCall,
    &Fy_parseRuleRet,
    &Fy_parseRuleRetConst16
};

static char *Fy_ParserError_toString(Fy_ParserError error) {
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
    case Fy_ParserError_CannotOpenFileForWrite:
        return "Cannot open file for writing";
    case Fy_ParserError_LabelNotFound:
        return "Label not found";
    case Fy_ParserError_UnexpectedLabel:
        return "Unexpected label";
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

static void Fy_Parser_dumpState(Fy_Parser *parser, Fy_ParserState *out_state) {
    out_state->stream = parser->lexer->stream;
    out_state->line = parser->lexer->line;
    out_state->column = parser->lexer->column;
}

static void Fy_Parser_loadState(Fy_Parser *parser, Fy_ParserState *state) {
    parser->lexer->stream = state->stream;
    parser->lexer->line = state->line;
    parser->lexer->column = state->column;
}

static bool Fy_Parser_lex(Fy_Parser *parser) {
    if (Fy_Lexer_lex(parser->lexer)) {
        parser->token = parser->lexer->token;
        return true;
    } else {
        return false;
    }
}

void Fy_Parser_error(Fy_Parser *parser, Fy_ParserError error, char *additional, ...) {
    char *line_start;
    printf("ParserError[%zu,%zu]: %s",
            parser->lexer->line, parser->lexer->column,
            Fy_ParserError_toString(error));

    // If there is additional text to be printed
    if (additional) {
        va_list va;
        printf(": ");
        va_start(va, additional);
        vprintf(additional, va);
        va_end(va);
    }

    printf("\n| ");
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
static bool Fy_Parser_match(Fy_Parser *parser, Fy_TokenType type) {
    Fy_ParserState backtrack;
    Fy_Parser_dumpState(parser, &backtrack);

    if (!Fy_Parser_lex(parser) || parser->token.type != type) {
        Fy_Parser_loadState(parser, &backtrack);
        return false;
    }

    return true;
}

/* Parsing helpers */
static Fy_Instruction *Fy_ParseOpReg8Const(Fy_Parser *parser, Fy_Token *token_arg1, Fy_Token *token_arg2, Fy_InstructionType *type) {
    Fy_Instruction_OpReg8Const *instruction = FY_INSTRUCTION_NEW(Fy_Instruction_OpReg8Const, *type);
    instruction->reg_id = Fy_TokenType_toReg8(token_arg1->type);
    instruction->value = Fy_Token_toConst8(token_arg2, parser);
    return (Fy_Instruction*)instruction;
}

static Fy_Instruction *Fy_ParseOpReg8Reg8(Fy_Parser *parser, Fy_Token *token_arg1, Fy_Token *token_arg2, Fy_InstructionType *type) {
    Fy_Instruction_OpReg8Reg8 *instruction = FY_INSTRUCTION_NEW(Fy_Instruction_OpReg8Reg8, *type);
    (void)parser;
    instruction->reg_id = Fy_TokenType_toReg8(token_arg1->type);
    instruction->reg2_id = Fy_TokenType_toReg8(token_arg2->type);
    return (Fy_Instruction*)instruction;
}

static Fy_Instruction *Fy_ParseOpReg16Const(Fy_Parser *parser, Fy_Token *token_arg1, Fy_Token *token_arg2, Fy_InstructionType *type) {
    Fy_Instruction_OpReg16Const *instruction = FY_INSTRUCTION_NEW(Fy_Instruction_OpReg16Const, *type);
    instruction->reg_id = Fy_TokenType_toReg16(token_arg1->type);
    instruction->value = Fy_Token_toConst16(token_arg2, parser);
    return (Fy_Instruction*)instruction;
}

static Fy_Instruction *Fy_ParseOpReg16Reg16(Fy_Parser *parser, Fy_Token *token_arg1, Fy_Token *token_arg2, Fy_InstructionType *type) {
    Fy_Instruction_OpReg16Reg16 *instruction = FY_INSTRUCTION_NEW(Fy_Instruction_OpReg16Reg16, *type);
    (void)parser;
    instruction->reg_id = Fy_TokenType_toReg16(token_arg1->type);
    instruction->reg2_id = Fy_TokenType_toReg16(token_arg2->type);
    return (Fy_Instruction*)instruction;
}

static Fy_Instruction *Fy_ParseOpLabel(Fy_Parser *parser, Fy_Token *token_arg, Fy_InstructionType *type) {
    Fy_Instruction_OpLabel *instruction = FY_INSTRUCTION_NEW(Fy_Instruction_OpLabel, *type);
    (void)parser;
    instruction->name = Fy_Token_toLowercaseCStr(token_arg);
    return (Fy_Instruction*)instruction;
}

static Fy_Instruction *Fy_ParseOpReg16(Fy_Parser *parser, Fy_Token *token_arg, Fy_InstructionType *type) {
    Fy_Instruction_OpReg16 *instruction = FY_INSTRUCTION_NEW(Fy_Instruction_OpReg16, *type);
    (void)parser;
    instruction->reg_id = Fy_TokenType_toReg16(token_arg->type);
    return (Fy_Instruction*)instruction;
}

static Fy_Instruction *Fy_ParseOpNoParams(Fy_Parser *parser, Fy_InstructionType *type) {
    (void)parser;
    return FY_INSTRUCTION_NEW(Fy_Instruction, *type);
}

static Fy_Instruction *Fy_ParseOpConst16(Fy_Parser *parser, Fy_Token *token_arg, Fy_InstructionType *type) {
    Fy_Instruction_OpConst16 *instruction = FY_INSTRUCTION_NEW(Fy_Instruction_OpConst16, *type);
    instruction->value = Fy_Token_toConst16(token_arg, parser);
    return (Fy_Instruction*)instruction;
}

/* Parsing functions */
static Fy_Instruction *Fy_ParseNop(Fy_Parser *parser) {
    (void)parser;
    return Fy_ParseOpNoParams(parser, &Fy_InstructionType_Nop);
}

static Fy_Instruction *Fy_ParseMovReg8Const(Fy_Parser *parser, Fy_Token *token_arg1, Fy_Token *token_arg2) {
    return Fy_ParseOpReg8Const(parser, token_arg1, token_arg2, &Fy_InstructionType_MovReg8Const);
}

static Fy_Instruction *Fy_ParseMovReg8Reg8(Fy_Parser *parser, Fy_Token *token_arg1, Fy_Token *token_arg2) {
    return Fy_ParseOpReg8Reg8(parser, token_arg1, token_arg2, &Fy_InstructionType_MovReg8Reg8);
}

static Fy_Instruction *Fy_ParseMovReg16Const(Fy_Parser *parser, Fy_Token *token_arg1, Fy_Token *token_arg2) {
    return Fy_ParseOpReg16Const(parser, token_arg1, token_arg2, &Fy_InstructionType_MovReg16Const);
}

static Fy_Instruction *Fy_ParseMovReg16Reg16(Fy_Parser *parser, Fy_Token *token_arg1, Fy_Token *token_arg2) {
    return Fy_ParseOpReg16Reg16(parser, token_arg1, token_arg2, &Fy_InstructionType_MovReg16Reg16);
}

static Fy_Instruction *Fy_ParseDebug(Fy_Parser *parser) {
    return Fy_ParseOpNoParams(parser, &Fy_InstructionType_Debug);
}

static Fy_Instruction *Fy_ParseEnd(Fy_Parser *parser) {
    return Fy_ParseOpNoParams(parser, &Fy_InstructionType_EndProgram);
}

static Fy_Instruction *Fy_ParseAddReg16Const(Fy_Parser *parser, Fy_Token *token_arg1, Fy_Token *token_arg2) {
    return Fy_ParseOpReg16Const(parser, token_arg1, token_arg2, &Fy_InstructionType_AddReg16Const);
}

static Fy_Instruction *Fy_ParseAddReg16Reg16(Fy_Parser *parser, Fy_Token *token_arg1, Fy_Token *token_arg2) {
    return Fy_ParseOpReg16Reg16(parser, token_arg1, token_arg2, &Fy_InstructionType_AddReg16Reg16);
}

static Fy_Instruction *Fy_ParseSubReg16Const(Fy_Parser *parser, Fy_Token *token_arg1, Fy_Token *token_arg2) {
    return Fy_ParseOpReg16Const(parser, token_arg1, token_arg2, &Fy_InstructionType_SubReg16Const);
}

static Fy_Instruction *Fy_ParseSubReg16Reg16(Fy_Parser *parser, Fy_Token *token_arg1, Fy_Token *token_arg2) {
    return Fy_ParseOpReg16Reg16(parser, token_arg1, token_arg2, &Fy_InstructionType_SubReg16Reg16);
}

static Fy_Instruction *Fy_ParseCmpReg16Const(Fy_Parser *parser, Fy_Token *token_arg1, Fy_Token *token_arg2) {
    return Fy_ParseOpReg16Const(parser, token_arg1, token_arg2, &Fy_InstructionType_CmpReg16Const);
}

static Fy_Instruction *Fy_ParseCmpReg16Reg16(Fy_Parser *parser, Fy_Token *token_arg1, Fy_Token *token_arg2) {
    return Fy_ParseOpReg16Reg16(parser, token_arg1, token_arg2, &Fy_InstructionType_CmpReg16Reg16);
}

static Fy_Instruction *Fy_ParseJmp(Fy_Parser *parser, Fy_Token *token_arg) {
    return Fy_ParseOpLabel(parser, token_arg, &Fy_InstructionType_Jmp);
}

static Fy_Instruction *Fy_ParseJe(Fy_Parser *parser, Fy_Token *token_arg) {
    return Fy_ParseOpLabel(parser, token_arg, &Fy_InstructionType_Je);
}

static Fy_Instruction *Fy_ParseJl(Fy_Parser *parser, Fy_Token *token_arg) {
    return Fy_ParseOpLabel(parser, token_arg, &Fy_InstructionType_Jl);
}

static Fy_Instruction *Fy_ParseJg(Fy_Parser *parser, Fy_Token *token_arg) {
    return Fy_ParseOpLabel(parser, token_arg, &Fy_InstructionType_Jg);
}

static Fy_Instruction *Fy_ParseCall(Fy_Parser *parser, Fy_Token *token_arg) {
    return Fy_ParseOpLabel(parser, token_arg, &Fy_InstructionType_Call);
}

static Fy_Instruction *Fy_ParseRet(Fy_Parser *parser) {
    return Fy_ParseOpNoParams(parser, &Fy_InstructionType_Ret);
}

static Fy_Instruction *Fy_ParseRetConst16(Fy_Parser *parser, Fy_Token *token_arg) {
    return Fy_ParseOpConst16(parser, token_arg, &Fy_InstructionType_RetConst16);
}

static Fy_Instruction *Fy_ParsePushConst(Fy_Parser *parser, Fy_Token *token_arg) {
    return Fy_ParseOpConst16(parser, token_arg, &Fy_InstructionType_PushConst);
}

static Fy_Instruction *Fy_ParsePushReg16(Fy_Parser *parser, Fy_Token *token_arg) {
    return Fy_ParseOpReg16(parser, token_arg, &Fy_InstructionType_PushReg16);
}

static Fy_Instruction *Fy_ParsePop(Fy_Parser *parser, Fy_Token *token_arg) {
    return Fy_ParseOpReg16(parser, token_arg, &Fy_InstructionType_Pop);
}

/* Processing functions */

static void Fy_ProcessOpLabel(Fy_Parser *parser, Fy_Instruction_OpLabel *instruction) {
    uint16_t address;
    if (!Fy_Labelmap_getEntry(&parser->labelmap, instruction->name, &address)) {
        // FIXME: This needs to have the right line and columns
        Fy_Parser_error(parser, Fy_ParserError_LabelNotFound, "%s", instruction->name);
    }
    instruction->address = address;
}

/* General parsing functions */

static bool Fy_Parser_expectNewline(Fy_Parser *parser, bool do_error) {
    Fy_ParserState state;
    Fy_Parser_dumpState(parser, &state);

    if (!Fy_Parser_lex(parser))
        return true; // Eof = Eol for us
    if (parser->token.type == Fy_TokenType_Newline)
        return true;

    // Load last state if we didn't get a newline
    Fy_Parser_loadState(parser, &state);
    if (do_error)
        Fy_Parser_error(parser, Fy_ParserError_ExpectedNewline, NULL);

    return false;
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
            Fy_Instruction *instruction;
            if (!Fy_Parser_expectNewline(parser, false)) {
                Fy_Parser_loadState(parser, &backtrack);
                continue;
            }
            instruction = rule->func_no_params(parser);
            instruction->parse_rule = rule;
            instruction->start_state = start_backtrack;
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
            Fy_Instruction *instruction;
            if (!Fy_Parser_expectNewline(parser, false)) {
                Fy_Parser_loadState(parser, &backtrack);
                continue;
            }
            instruction = rule->func_one_param(parser, &token_arg1);
            instruction->parse_rule = rule;
            instruction->start_state = start_backtrack;
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
            Fy_Instruction *instruction;
            if (!Fy_Parser_expectNewline(parser, false)) {
                Fy_Parser_loadState(parser, &backtrack);
                continue;
            }
            instruction = rule->func_two_params(parser, &token_arg1, &token_arg2);
            instruction->parse_rule = rule;
            instruction->start_state = start_backtrack;
            return instruction;
        }

        FY_UNREACHABLE();
    }

    // TODO: Add clever error message that tells us why we were wrong

    Fy_Parser_error(parser, Fy_ParserError_InvalidInstruction, NULL);

    FY_UNREACHABLE();
}

/* Returns whether we parsed a label */
static bool Fy_Parser_parseLabel(Fy_Parser *parser) {
    Fy_Token label_token;
    char *label_string;

    if (!Fy_Parser_match(parser, Fy_TokenType_Label))
        return false;
    label_token = parser->token;

    if (!Fy_Parser_match(parser, Fy_TokenType_Colon))
        Fy_Parser_error(parser, Fy_ParserError_SyntaxError, NULL);

    // Advance newline if there is
    Fy_Parser_expectNewline(parser, false);

    label_string = Fy_Token_toLowercaseCStr(&label_token);
    Fy_Labelmap_addEntry(&parser->labelmap, label_string, parser->code_offset);

    return true;
}

static bool Fy_Parser_parseProc(Fy_Parser *parser) {
    char *label_string;
    uint16_t start_offset = parser->code_offset;

    if (!Fy_Parser_match(parser, Fy_TokenType_Proc))
        return false;

    if (!Fy_Parser_match(parser, Fy_TokenType_Label))
        Fy_Parser_error(parser, Fy_ParserError_SyntaxError, NULL);

    label_string = Fy_Token_toLowercaseCStr(&parser->token);

    Fy_Parser_expectNewline(parser, true);

    for (;;) {
        if (Fy_Parser_match(parser, Fy_TokenType_Endp)) {
            char *endp_label_string;
            if (!Fy_Parser_match(parser, Fy_TokenType_Label))
                Fy_Parser_error(parser, Fy_ParserError_SyntaxError, NULL);
            endp_label_string = Fy_Token_toLowercaseCStr(&parser->token);
            if (strcmp(label_string, endp_label_string) != 0)
                Fy_Parser_error(parser, Fy_ParserError_UnexpectedLabel, "Expected '%s'", label_string);

            Fy_Parser_expectNewline(parser, true);
            break;
        }
        if (!Fy_Parser_parseLine(parser))
            Fy_Parser_error(parser, Fy_ParserError_UnexpectedToken, NULL);
    }

    Fy_Labelmap_addEntry(&parser->labelmap, label_string, start_offset);

    return true;
}

/* Parse label, procedure or instruction */
static bool Fy_Parser_parseLine(Fy_Parser *parser) {
    Fy_Instruction *instruction;

    Fy_Parser_expectNewline(parser, false);

    if (Fy_Parser_parseLabel(parser))
        return true;

    if (Fy_Parser_parseProc(parser))
        return true;

    if ((instruction = Fy_Parser_parseInstruction(parser))) {
        if (parser->amount_allocated == 0)
            parser->instructions = malloc((parser->amount_allocated = 8) * sizeof(Fy_Instruction*));
        else if (parser->amount_used == parser->amount_allocated)
            parser->instructions = realloc(parser->instructions, (parser->amount_allocated += 8) * sizeof(Fy_Instruction*));

        parser->instructions[parser->amount_used++] = instruction;
        // Update offset
        parser->code_offset += 1 + instruction->type->additional_size;
        return true;
    }

    return false;
}

/* Step 1: reading all of the instructions and creating a vector of them */
static void Fy_Parser_readInstructions(Fy_Parser *parser) {
    Fy_ParserState backtrack;

    // If there is a newline, advance it
    Fy_Parser_expectNewline(parser, false);

    while (Fy_Parser_parseLine(parser))
        ;

    Fy_Parser_dumpState(parser, &backtrack);
    if (Fy_Parser_lex(parser)) {
        Fy_Parser_loadState(parser, &backtrack);
        Fy_Parser_error(parser, Fy_ParserError_SyntaxError, NULL);
    }
}


/* Step 2: processing parsed instructions */
static void Fy_Parser_processInstructions(Fy_Parser *parser) {
    for (size_t i = 0; i < parser->amount_used; ++i) {
        Fy_Instruction *instruction = parser->instructions[i];
        Fy_Parser_loadState(parser, &instruction->start_state);
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

    if (!file)
        Fy_Parser_error(parser, Fy_ParserError_CannotOpenFileForWrite, "%s", filename);

    Fy_Parser_generateBytecode(parser, &generator);
    fwrite(generator.output, 1, generator.idx, file);

    fclose(file);
}
