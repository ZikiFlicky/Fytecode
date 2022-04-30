#include "fy.h"

/* Parse-function (step 1) declarations */
static Fy_Instruction *Fy_ParseNop(Fy_Parser *parser);
static Fy_Instruction *Fy_ParseMovReg8Const(Fy_Parser *parser, Fy_InstructionArg *arg1, Fy_InstructionArg *arg2);
static Fy_Instruction *Fy_ParseMovReg8Reg8(Fy_Parser *parser, Fy_InstructionArg *arg1, Fy_InstructionArg *arg2);
static Fy_Instruction *Fy_ParseMovReg16Const(Fy_Parser *parser, Fy_InstructionArg *arg1, Fy_InstructionArg *arg2);
static Fy_Instruction *Fy_ParseMovReg16Reg16(Fy_Parser *parser, Fy_InstructionArg *arg1, Fy_InstructionArg *arg2);
static Fy_Instruction *Fy_ParseDebug(Fy_Parser *parser);
static Fy_Instruction *Fy_ParseDebugStack(Fy_Parser *parser);
static Fy_Instruction *Fy_ParseEnd(Fy_Parser *parser);
static Fy_Instruction *Fy_ParseAddReg16Const(Fy_Parser *parser, Fy_InstructionArg *arg1, Fy_InstructionArg *arg2);
static Fy_Instruction *Fy_ParseAddReg16Reg16(Fy_Parser *parser, Fy_InstructionArg *arg1, Fy_InstructionArg *arg2);
static Fy_Instruction *Fy_ParseSubReg16Const(Fy_Parser *parser, Fy_InstructionArg *arg1, Fy_InstructionArg *arg2);
static Fy_Instruction *Fy_ParseSubReg16Reg16(Fy_Parser *parser, Fy_InstructionArg *arg1, Fy_InstructionArg *arg2);
static Fy_Instruction *Fy_ParseCmpReg16Const(Fy_Parser *parser, Fy_InstructionArg *arg1, Fy_InstructionArg *arg2);
static Fy_Instruction *Fy_ParseCmpReg16Reg16(Fy_Parser *parser, Fy_InstructionArg *arg1, Fy_InstructionArg *arg2);
static Fy_Instruction *Fy_ParseJmp(Fy_Parser *parser, Fy_InstructionArg *arg);
static Fy_Instruction *Fy_ParseJe(Fy_Parser *parser, Fy_InstructionArg *arg);
static Fy_Instruction *Fy_ParseJl(Fy_Parser *parser, Fy_InstructionArg *arg);
static Fy_Instruction *Fy_ParseJg(Fy_Parser *parser, Fy_InstructionArg *arg);
static Fy_Instruction *Fy_ParseCall(Fy_Parser *parser, Fy_InstructionArg *arg);
static Fy_Instruction *Fy_ParseRet(Fy_Parser *parser);
static Fy_Instruction *Fy_ParseRetConst16(Fy_Parser *parser, Fy_InstructionArg *arg);
static Fy_Instruction *Fy_ParsePushConst(Fy_Parser *parser, Fy_InstructionArg *arg);
static Fy_Instruction *Fy_ParsePushReg16(Fy_Parser *parser, Fy_InstructionArg *arg);
static Fy_Instruction *Fy_ParsePop(Fy_Parser *parser, Fy_InstructionArg *arg);
static Fy_Instruction *Fy_ParseMovReg16Mem(Fy_Parser *parser, Fy_InstructionArg *arg1, Fy_InstructionArg *arg2);
static Fy_Instruction *Fy_ParseLea(Fy_Parser *parser, Fy_InstructionArg *arg1, Fy_InstructionArg *arg2);

/* Process-function (parsing step 2) declarations */
static void Fy_ProcessOpCodeLabel(Fy_Parser *parser, Fy_Instruction_OpLabel *instruction);
static void Fy_ProcessOpReg16Mem(Fy_Parser *parser, Fy_Instruction_OpReg16Mem *instruction);

/* Process-label-function (parsing step 3) declarations */
static void Fy_ProcessLabelOpCodeLabel(Fy_Instruction_OpLabel *instruction, Fy_Parser *parser);

/* Function to parse anything found in text */
static bool Fy_Parser_parseLine(Fy_Parser *parser);

/* Define rules */
Fy_ParserParseRule Fy_parseRuleNop = {
    .type = Fy_ParserParseRuleType_NoParams,
    .start_token = Fy_TokenType_Nop,
    .func_no_params = Fy_ParseNop,
    .process_func = NULL,
    .process_label_func = NULL
};
Fy_ParserParseRule Fy_parseRuleMovReg8Const = {
    .type = Fy_ParserParseRuleType_TwoParams,
    .start_token = Fy_TokenType_Mov,
    .arg1_type = Fy_InstructionArgType_Reg8,
    .arg2_type = Fy_InstructionArgType_Constant,
    .func_two_params = Fy_ParseMovReg8Const,
    .process_func = NULL,
    .process_label_func = NULL
};
Fy_ParserParseRule Fy_parseRuleMovReg8Reg8 = {
    .type = Fy_ParserParseRuleType_TwoParams,
    .start_token = Fy_TokenType_Mov,
    .arg1_type = Fy_InstructionArgType_Reg8,
    .arg2_type = Fy_InstructionArgType_Reg8,
    .func_two_params = Fy_ParseMovReg8Reg8,
    .process_func = NULL,
    .process_label_func = NULL
};
Fy_ParserParseRule Fy_parseRuleMovReg16Const = {
    .type = Fy_ParserParseRuleType_TwoParams,
    .start_token = Fy_TokenType_Mov,
    .arg1_type = Fy_InstructionArgType_Reg16,
    .arg2_type = Fy_InstructionArgType_Constant,
    .func_two_params = Fy_ParseMovReg16Const,
    .process_func = NULL,
    .process_label_func = NULL
};
Fy_ParserParseRule Fy_parseRuleMovReg16Reg16 = {
    .type = Fy_ParserParseRuleType_TwoParams,
    .start_token = Fy_TokenType_Mov,
    .arg1_type = Fy_InstructionArgType_Reg16,
    .arg2_type = Fy_InstructionArgType_Reg16,
    .func_two_params = Fy_ParseMovReg16Reg16,
    .process_func = NULL,
    .process_label_func = NULL
};
Fy_ParserParseRule Fy_parseRuleDebug = {
    .type = Fy_ParserParseRuleType_NoParams,
    .start_token = Fy_TokenType_Debug,
    .func_no_params = Fy_ParseDebug,
    .process_func = NULL,
    .process_label_func = NULL
};
Fy_ParserParseRule Fy_parseRuleDebugStack = {
    .type = Fy_ParserParseRuleType_NoParams,
    .start_token = Fy_TokenType_DebugStack,
    .func_no_params = Fy_ParseDebugStack,
    .process_func = NULL,
    .process_label_func = NULL
};
Fy_ParserParseRule Fy_parseRuleEnd = {
    .type = Fy_ParserParseRuleType_NoParams,
    .start_token = Fy_TokenType_End,
    .func_no_params = Fy_ParseEnd,
    .process_func = NULL,
    .process_label_func = NULL
};
Fy_ParserParseRule Fy_parseRuleAddReg16Const = {
    .type = Fy_ParserParseRuleType_TwoParams,
    .start_token = Fy_TokenType_Add,
    .arg1_type = Fy_InstructionArgType_Reg16,
    .arg2_type = Fy_InstructionArgType_Constant,
    .func_two_params = Fy_ParseAddReg16Const,
    .process_func = NULL,
    .process_label_func = NULL
};
Fy_ParserParseRule Fy_parseRuleAddReg16Reg16 = {
    .type = Fy_ParserParseRuleType_TwoParams,
    .start_token = Fy_TokenType_Add,
    .arg1_type = Fy_InstructionArgType_Reg16,
    .arg2_type = Fy_InstructionArgType_Reg16,
    .func_two_params = Fy_ParseAddReg16Reg16,
    .process_func = NULL,
    .process_label_func = NULL
};
Fy_ParserParseRule Fy_parseRuleSubReg16Const = {
    .type = Fy_ParserParseRuleType_TwoParams,
    .start_token = Fy_TokenType_Sub,
    .arg1_type = Fy_InstructionArgType_Reg16,
    .arg2_type = Fy_InstructionArgType_Constant,
    .func_two_params = Fy_ParseSubReg16Const,
    .process_func = NULL,
    .process_label_func = NULL
};
Fy_ParserParseRule Fy_parseRuleSubReg16Reg16 = {
    .type = Fy_ParserParseRuleType_TwoParams,
    .start_token = Fy_TokenType_Sub,
    .arg1_type = Fy_InstructionArgType_Reg16,
    .arg2_type = Fy_InstructionArgType_Reg16,
    .func_two_params = Fy_ParseSubReg16Reg16,
    .process_func = NULL,
    .process_label_func = NULL
};
Fy_ParserParseRule Fy_parseRuleCmpReg16Const = {
    .type = Fy_ParserParseRuleType_TwoParams,
    .start_token = Fy_TokenType_Cmp,
    .arg1_type = Fy_InstructionArgType_Reg16,
    .arg2_type = Fy_InstructionArgType_Constant,
    .func_two_params = Fy_ParseCmpReg16Const,
    .process_func = NULL,
    .process_label_func = NULL
};
Fy_ParserParseRule Fy_parseRuleCmpReg16Reg16 = {
    .type = Fy_ParserParseRuleType_TwoParams,
    .start_token = Fy_TokenType_Cmp,
    .arg1_type = Fy_InstructionArgType_Reg16,
    .arg2_type = Fy_InstructionArgType_Reg16,
    .func_two_params = Fy_ParseCmpReg16Reg16,
    .process_func = NULL,
    .process_label_func = NULL
};
Fy_ParserParseRule Fy_parseRuleJmp = {
    .type = Fy_ParserParseRuleType_OneParam,
    .start_token = Fy_TokenType_Jmp,
    .arg1_type = Fy_InstructionArgType_Label,
    .func_one_param = Fy_ParseJmp,
    .process_func = (Fy_InstructionProcessFunc)Fy_ProcessOpCodeLabel,
    .process_label_func = (Fy_InstructionProcessLabelFunc)Fy_ProcessLabelOpCodeLabel
};
Fy_ParserParseRule Fy_parseRuleJe = {
    .type = Fy_ParserParseRuleType_OneParam,
    .start_token = Fy_TokenType_Je,
    .arg1_type = Fy_InstructionArgType_Label,
    .func_one_param = Fy_ParseJe,
    .process_func = (Fy_InstructionProcessFunc)Fy_ProcessOpCodeLabel,
    .process_label_func = (Fy_InstructionProcessLabelFunc)Fy_ProcessLabelOpCodeLabel
};
Fy_ParserParseRule Fy_parseRuleJl = {
    .type = Fy_ParserParseRuleType_OneParam,
    .start_token = Fy_TokenType_Jl,
    .arg1_type = Fy_InstructionArgType_Label,
    .func_one_param = Fy_ParseJl,
    .process_func = (Fy_InstructionProcessFunc)Fy_ProcessOpCodeLabel,
    .process_label_func = (Fy_InstructionProcessLabelFunc)Fy_ProcessLabelOpCodeLabel
};
Fy_ParserParseRule Fy_parseRuleJg = {
    .type = Fy_ParserParseRuleType_OneParam,
    .start_token = Fy_TokenType_Jg,
    .arg1_type = Fy_InstructionArgType_Label,
    .func_one_param = Fy_ParseJg,
    .process_func = (Fy_InstructionProcessFunc)Fy_ProcessOpCodeLabel,
    .process_label_func = (Fy_InstructionProcessLabelFunc)Fy_ProcessLabelOpCodeLabel
};
Fy_ParserParseRule Fy_parseRulePushConst = {
    .type = Fy_ParserParseRuleType_OneParam,
    .start_token = Fy_TokenType_Push,
    .arg1_type = Fy_InstructionArgType_Constant,
    .func_one_param = Fy_ParsePushConst,
    .process_func = NULL,
    .process_label_func = NULL
};
Fy_ParserParseRule Fy_parseRulePushReg16 = {
    .type = Fy_ParserParseRuleType_OneParam,
    .start_token = Fy_TokenType_Push,
    .arg1_type = Fy_InstructionArgType_Reg16,
    .func_one_param = Fy_ParsePushReg16,
    .process_func = NULL,
    .process_label_func = NULL
};
Fy_ParserParseRule Fy_parseRulePop = {
    .type = Fy_ParserParseRuleType_OneParam,
    .start_token = Fy_TokenType_Pop,
    .arg1_type = Fy_InstructionArgType_Reg16,
    .func_one_param = Fy_ParsePop,
    .process_func = NULL,
    .process_label_func = NULL
};
Fy_ParserParseRule Fy_parseRuleCall = {
    .type = Fy_ParserParseRuleType_OneParam,
    .start_token = Fy_TokenType_Call,
    .arg1_type = Fy_InstructionArgType_Label,
    .func_one_param = Fy_ParseCall,
    .process_func = (Fy_InstructionProcessFunc)Fy_ProcessOpCodeLabel,
    .process_label_func = (Fy_InstructionProcessLabelFunc)Fy_ProcessLabelOpCodeLabel
};
Fy_ParserParseRule Fy_parseRuleRet = {
    .type = Fy_ParserParseRuleType_NoParams,
    .start_token = Fy_TokenType_Ret,
    .func_no_params = Fy_ParseRet,
    .process_func = NULL,
    .process_label_func = NULL
};
Fy_ParserParseRule Fy_parseRuleRetConst16 = {
    .type = Fy_ParserParseRuleType_OneParam,
    .start_token = Fy_TokenType_Ret,
    .arg1_type = Fy_InstructionArgType_Constant,
    .func_one_param = Fy_ParseRetConst16,
    .process_func = NULL,
    .process_label_func = NULL
};
Fy_ParserParseRule Fy_parseRuleMovReg16Mem = {
    .type = Fy_ParserParseRuleType_TwoParams,
    .start_token = Fy_TokenType_Mov,
    .arg1_type = Fy_InstructionArgType_Reg16,
    .arg2_type = Fy_InstructionArgType_Memory,
    .func_two_params = Fy_ParseMovReg16Mem,
    .process_func = (Fy_InstructionProcessFunc)Fy_ProcessOpReg16Mem,
    .process_label_func = NULL
};
Fy_ParserParseRule Fy_parseRuleLea = {
    .type = Fy_ParserParseRuleType_TwoParams,
    .start_token = Fy_TokenType_Lea,
    .arg1_type = Fy_InstructionArgType_Reg16,
    .arg2_type = Fy_InstructionArgType_Memory,
    .func_two_params = Fy_ParseLea,
    .process_func = (Fy_InstructionProcessFunc)Fy_ProcessOpReg16Mem,
    .process_label_func = NULL
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
    &Fy_parseRuleDebugStack,
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
    &Fy_parseRuleRetConst16,
    &Fy_parseRuleMovReg16Mem,
    &Fy_parseRuleLea
};

static void Fy_InstructionArg_Destruct(Fy_InstructionArg *arg) {
    switch (arg->type) {
    case Fy_InstructionArgType_Label:
        free(arg->as_label);
        break;
    default:
        break;
    }
}

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
    case Fy_ParserError_ExpectedDifferentToken:
        return "Expected different token";
    case Fy_ParserError_LabelNotCode:
        return "Label doesn't reference code and is most likely a variable";
    case Fy_ParserError_LabelNotVariable:
        return "Label doesn't reference a variable and is most likely a code/procedure reference";
    case Fy_ParserError_LabelAlreadyExists:
        return "Label already exists";
    default:
        FY_UNREACHABLE();
    }
}

/* Initialize a parser instance */
void Fy_Parser_Init(Fy_Lexer *lexer, Fy_Parser *out) {
    out->lexer = lexer;
    out->amount_allocated = 0;
    out->amount_used = 0;
    out->data_allocated = 0;
    out->data_size = 0;
    Fy_Labelmap_Init(&out->labelmap);
}

void Fy_Parser_Destruct(Fy_Parser *parser) {
    if (parser->amount_allocated > 0) {
        for (size_t i = 0; i < parser->amount_used; ++i) {
            Fy_Instruction *instruction = parser->instructions[i];
            free(instruction);
        }
        free(parser->instructions);
    }
    if (parser->data_allocated > 0)
        free(parser->data_part);
    Fy_Labelmap_Destruct(&parser->labelmap);
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

void Fy_Parser_error(Fy_Parser *parser, Fy_ParserError error, Fy_ParserState *state, char *additional, ...) {
    char *line_start;

    // If this is a state error
    if (state)
        Fy_Parser_loadState(parser, state);

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

    Fy_Parser_Destruct(parser);
    exit(1);
}

/* Other parser methods */

static void Fy_Parser_extendData(Fy_Parser *parser, uint16_t amount) {
    if (parser->data_allocated == 0)
        parser->data_part = malloc((parser->data_allocated = 16));
    else if (parser->data_size + amount >= parser->data_allocated)
        parser->data_part = realloc(parser->data_part, (parser->data_allocated += 16));
}

static void Fy_Parser_addData8(Fy_Parser *parser, uint8_t value) {
    Fy_Parser_extendData(parser, 1);
    parser->data_part[parser->data_size] = value;
    parser->data_size += 1;
}

static void Fy_Parser_addData16(Fy_Parser *parser, uint16_t value) {
    Fy_Parser_extendData(parser, 2);
    parser->data_part[parser->data_size] = value & 0xFF;
    parser->data_part[parser->data_size + 1] = value >> 8;
    parser->data_size += 2;
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
static Fy_Instruction *Fy_ParseOpReg8Const(Fy_Parser *parser, Fy_InstructionArg *arg1, Fy_InstructionArg *arg2, Fy_InstructionType *type) {
    Fy_Instruction_OpReg8Const *instruction = FY_INSTRUCTION_NEW(Fy_Instruction_OpReg8Const, *type);
    (void)parser;
    instruction->reg_id = arg1->as_reg8;
    instruction->value = arg2->as_const; // FIXME: Verify that this is indeed 8-bit
    return (Fy_Instruction*)instruction;
}

static Fy_Instruction *Fy_ParseOpReg8Reg8(Fy_Parser *parser, Fy_InstructionArg *arg1, Fy_InstructionArg *arg2, Fy_InstructionType *type) {
    Fy_Instruction_OpReg8Reg8 *instruction = FY_INSTRUCTION_NEW(Fy_Instruction_OpReg8Reg8, *type);
    (void)parser;
    instruction->reg_id = arg1->as_reg8;
    instruction->reg2_id = arg2->as_reg8;
    return (Fy_Instruction*)instruction;
}

static Fy_Instruction *Fy_ParseOpReg16Const(Fy_Parser *parser, Fy_InstructionArg *arg1, Fy_InstructionArg *arg2, Fy_InstructionType *type) {
    Fy_Instruction_OpReg16Const *instruction = FY_INSTRUCTION_NEW(Fy_Instruction_OpReg16Const, *type);
    (void)parser;
    instruction->reg_id = arg1->as_reg8;
    instruction->value = arg2->as_const;
    return (Fy_Instruction*)instruction;
}

static Fy_Instruction *Fy_ParseOpReg16Reg16(Fy_Parser *parser, Fy_InstructionArg *arg1, Fy_InstructionArg *arg2, Fy_InstructionType *type) {
    Fy_Instruction_OpReg16Reg16 *instruction = FY_INSTRUCTION_NEW(Fy_Instruction_OpReg16Reg16, *type);
    (void)parser;
    instruction->reg_id = arg1->as_reg16;
    instruction->reg2_id = arg2->as_reg16;
    return (Fy_Instruction*)instruction;
}

static Fy_Instruction *Fy_ParseOpLabel(Fy_Parser *parser, Fy_InstructionArg *arg, Fy_InstructionType *type) {
    Fy_Instruction_OpLabel *instruction = FY_INSTRUCTION_NEW(Fy_Instruction_OpLabel, *type);
    (void)parser;
    instruction->name = arg->as_label;
    return (Fy_Instruction*)instruction;
}

static Fy_Instruction *Fy_ParseOpReg16(Fy_Parser *parser, Fy_InstructionArg *arg, Fy_InstructionType *type) {
    Fy_Instruction_OpReg16 *instruction = FY_INSTRUCTION_NEW(Fy_Instruction_OpReg16, *type);
    (void)parser;
    instruction->reg_id = arg->as_reg16;
    return (Fy_Instruction*)instruction;
}

static Fy_Instruction *Fy_ParseOpNoParams(Fy_Parser *parser, Fy_InstructionType *type) {
    (void)parser;
    return FY_INSTRUCTION_NEW(Fy_Instruction, *type);
}

static Fy_Instruction *Fy_ParseOpConst16(Fy_Parser *parser, Fy_InstructionArg *arg, Fy_InstructionType *type) {
    Fy_Instruction_OpConst16 *instruction = FY_INSTRUCTION_NEW(Fy_Instruction_OpConst16, *type);
    (void)parser;
    instruction->value = arg->as_const;
    return (Fy_Instruction*)instruction;
}

static Fy_Instruction *Fy_ParseOpReg16Mem(Fy_Parser *parser, Fy_InstructionArg *arg1, Fy_InstructionArg *arg2, Fy_InstructionType *type) {
    Fy_Instruction_OpReg16Mem *instruction = FY_INSTRUCTION_NEW(Fy_Instruction_OpReg16Mem, *type);
    (void)parser;
    instruction->reg_id = arg1->as_reg16;
    instruction->address_ast = arg2->as_memory;
    return (Fy_Instruction*)instruction;
}

/* Parsing functions */
static Fy_Instruction *Fy_ParseNop(Fy_Parser *parser) {
    (void)parser;
    return Fy_ParseOpNoParams(parser, &Fy_instructionTypeNop);
}

static Fy_Instruction *Fy_ParseMovReg8Const(Fy_Parser *parser, Fy_InstructionArg *arg1, Fy_InstructionArg *arg2) {
    return Fy_ParseOpReg8Const(parser, arg1, arg2, &Fy_instructionTypeMovReg8Const);
}

static Fy_Instruction *Fy_ParseMovReg8Reg8(Fy_Parser *parser, Fy_InstructionArg *arg1, Fy_InstructionArg *arg2) {
    return Fy_ParseOpReg8Reg8(parser, arg1, arg2, &Fy_instructionTypeMovReg8Reg8);
}

static Fy_Instruction *Fy_ParseMovReg16Const(Fy_Parser *parser, Fy_InstructionArg *arg1, Fy_InstructionArg *arg2) {
    return Fy_ParseOpReg16Const(parser, arg1, arg2, &Fy_instructionTypeMovReg16Const);
}

static Fy_Instruction *Fy_ParseMovReg16Reg16(Fy_Parser *parser, Fy_InstructionArg *arg1, Fy_InstructionArg *arg2) {
    return Fy_ParseOpReg16Reg16(parser, arg1, arg2, &Fy_instructionTypeMovReg16Reg16);
}

static Fy_Instruction *Fy_ParseDebug(Fy_Parser *parser) {
    return Fy_ParseOpNoParams(parser, &Fy_instructionTypeDebug);
}

static Fy_Instruction *Fy_ParseDebugStack(Fy_Parser *parser) {
    return Fy_ParseOpNoParams(parser, &Fy_instructionTypeDebugStack);
}

static Fy_Instruction *Fy_ParseEnd(Fy_Parser *parser) {
    return Fy_ParseOpNoParams(parser, &Fy_instructionTypeEndProgram);
}

static Fy_Instruction *Fy_ParseAddReg16Const(Fy_Parser *parser, Fy_InstructionArg *arg1, Fy_InstructionArg *arg2) {
    return Fy_ParseOpReg16Const(parser, arg1, arg2, &Fy_instructionTypeAddReg16Const);
}

static Fy_Instruction *Fy_ParseAddReg16Reg16(Fy_Parser *parser, Fy_InstructionArg *arg1, Fy_InstructionArg *arg2) {
    return Fy_ParseOpReg16Reg16(parser, arg1, arg2, &Fy_instructionTypeAddReg16Reg16);
}

static Fy_Instruction *Fy_ParseSubReg16Const(Fy_Parser *parser, Fy_InstructionArg *arg1, Fy_InstructionArg *arg2) {
    return Fy_ParseOpReg16Const(parser, arg1, arg2, &Fy_instructionTypeSubReg16Const);
}

static Fy_Instruction *Fy_ParseSubReg16Reg16(Fy_Parser *parser, Fy_InstructionArg *arg1, Fy_InstructionArg *arg2) {
    return Fy_ParseOpReg16Reg16(parser, arg1, arg2, &Fy_instructionTypeSubReg16Reg16);
}

static Fy_Instruction *Fy_ParseCmpReg16Const(Fy_Parser *parser, Fy_InstructionArg *arg1, Fy_InstructionArg *arg2) {
    return Fy_ParseOpReg16Const(parser, arg1, arg2, &Fy_instructionTypeCmpReg16Const);
}

static Fy_Instruction *Fy_ParseCmpReg16Reg16(Fy_Parser *parser, Fy_InstructionArg *arg1, Fy_InstructionArg *arg2) {
    return Fy_ParseOpReg16Reg16(parser, arg1, arg2, &Fy_instructionTypeCmpReg16Reg16);
}

static Fy_Instruction *Fy_ParseJmp(Fy_Parser *parser, Fy_InstructionArg *arg) {
    return Fy_ParseOpLabel(parser, arg, &Fy_instructionTypeJmp);
}

static Fy_Instruction *Fy_ParseJe(Fy_Parser *parser, Fy_InstructionArg *arg) {
    return Fy_ParseOpLabel(parser, arg, &Fy_instructionTypeJe);
}

static Fy_Instruction *Fy_ParseJl(Fy_Parser *parser, Fy_InstructionArg *arg) {
    return Fy_ParseOpLabel(parser, arg, &Fy_instructionTypeJl);
}

static Fy_Instruction *Fy_ParseJg(Fy_Parser *parser, Fy_InstructionArg *arg) {
    return Fy_ParseOpLabel(parser, arg, &Fy_instructionTypeJg);
}

static Fy_Instruction *Fy_ParseCall(Fy_Parser *parser, Fy_InstructionArg *arg) {
    return Fy_ParseOpLabel(parser, arg, &Fy_instructionTypeCall);
}

static Fy_Instruction *Fy_ParseRet(Fy_Parser *parser) {
    return Fy_ParseOpNoParams(parser, &Fy_instructionTypeRet);
}

static Fy_Instruction *Fy_ParseRetConst16(Fy_Parser *parser, Fy_InstructionArg *arg) {
    return Fy_ParseOpConst16(parser, arg, &Fy_instructionTypeRetConst16);
}

static Fy_Instruction *Fy_ParsePushConst(Fy_Parser *parser, Fy_InstructionArg *arg) {
    return Fy_ParseOpConst16(parser, arg, &Fy_instructionTypePushConst);
}

static Fy_Instruction *Fy_ParsePushReg16(Fy_Parser *parser, Fy_InstructionArg *arg) {
    return Fy_ParseOpReg16(parser, arg, &Fy_instructionTypePushReg16);
}

static Fy_Instruction *Fy_ParsePop(Fy_Parser *parser, Fy_InstructionArg *arg) {
    return Fy_ParseOpReg16(parser, arg, &Fy_instructionTypePop);
}

static Fy_Instruction *Fy_ParseMovReg16Mem(Fy_Parser *parser, Fy_InstructionArg *arg1, Fy_InstructionArg *arg2) {
    return Fy_ParseOpReg16Mem(parser, arg1, arg2, &Fy_instructionTypeMovReg16Mem);
}

static Fy_Instruction *Fy_ParseLea(Fy_Parser *parser, Fy_InstructionArg *arg1, Fy_InstructionArg *arg2) {
    return Fy_ParseOpReg16Mem(parser, arg1, arg2, &Fy_instructionTypeLea);
}

/* Processing functions */

static void Fy_ProcessOpCodeLabel(Fy_Parser *parser, Fy_Instruction_OpLabel *instruction) {
    Fy_BucketNode *node;
    node = Fy_Labelmap_getEntry(&parser->labelmap, instruction->name);
    if (!node) {
        // FIXME: This needs to have the right line and columns
        Fy_Parser_error(parser, Fy_ParserError_LabelNotFound, NULL, "%s", instruction->name);
    }
    if (node->type != Fy_MapEntryType_CodeLabel)
        Fy_Parser_error(parser, Fy_ParserError_LabelNotCode, NULL, "%s", instruction->name);
    instruction->instruction_offset = node->code_label;
    // We don't need this anymore
    free(instruction->name);
}

static void Fy_ProcessOpReg16Mem(Fy_Parser *parser, Fy_Instruction_OpReg16Mem *instruction) {
    Fy_AST_eval(instruction->address_ast, parser, &instruction->value);
}

/* Label processing functions */

static void Fy_ProcessLabelOpCodeLabel(Fy_Instruction_OpLabel *instruction, Fy_Parser *parser) {
    instruction->address = Fy_Parser_getCodeOffsetByInstructionIndex(parser, instruction->instruction_offset);
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
        Fy_Parser_error(parser, Fy_ParserError_ExpectedNewline, NULL, NULL);

    return false;
}

static Fy_AST *Fy_Parser_parseLiteralExpr(Fy_Parser *parser) {
    Fy_AST *expr;
    Fy_ParserState backtrack;
    Fy_Parser_dumpState(parser, &backtrack);

    if (!Fy_Parser_lex(parser))
        return NULL;

    switch (parser->token.type) {
    case Fy_TokenType_Const: {
        uint16_t literal = Fy_Token_toConst16(&parser->token, parser);
        expr = Fy_AST_New(Fy_ASTType_Number);
        expr->as_number = literal;
        break;
    }
    case Fy_TokenType_Label: {
        expr = Fy_AST_New(Fy_ASTType_Label);
        expr->as_label = Fy_Token_toLowercaseCStr(&parser->token);
        break;
    }
    case Fy_TokenType_Bx:
        expr = Fy_AST_New(Fy_ASTType_Bx);
        break;
    case Fy_TokenType_Bp:
        expr = Fy_AST_New(Fy_ASTType_Bp);
        break;
    default:
        Fy_Parser_loadState(parser, &backtrack);
        return NULL;
    }
    expr->state = backtrack;
    return expr;
}

static Fy_AST *Fy_Parser_parseSumExpr(Fy_Parser *parser) {
    Fy_AST *expr;

    if (!(expr = Fy_Parser_parseLiteralExpr(parser)))
        return NULL;

    for (;;) {
        Fy_ParserState backtrack;
        Fy_AST *new_expr, *rhs;
        Fy_ASTType type;

        Fy_Parser_dumpState(parser, &backtrack);

        if (!Fy_Parser_lex(parser))
            break;

        switch (parser->token.type) {
        case Fy_TokenType_Plus:
            type = Fy_ASTType_Add;
            break;
        case Fy_TokenType_Minus:
            type = Fy_ASTType_Sub;
            break;
        default:
            Fy_Parser_loadState(parser, &backtrack);
            return expr;
        }

        if (!(rhs = Fy_Parser_parseLiteralExpr(parser)))
            Fy_Parser_error(parser, Fy_ParserError_SyntaxError, NULL, NULL);

        new_expr = Fy_AST_New(type);
        new_expr->lhs = expr;
        new_expr->rhs = rhs;
        new_expr->state = backtrack;
        expr = new_expr;
    }

    return expr;
}

static Fy_AST *Fy_Parser_parseMemExpr(Fy_Parser *parser) {
    Fy_AST *ast;

    if (!Fy_Parser_match(parser, Fy_TokenType_LeftBracket))
        return NULL;

    ast = Fy_Parser_parseSumExpr(parser);

    if (!Fy_Parser_match(parser, Fy_TokenType_RightBracket))
        Fy_Parser_error(parser, Fy_ParserError_ExpectedDifferentToken, NULL, "']'");

    return ast;
}

static bool Fy_Parser_parseArgument(Fy_Parser *parser, Fy_InstructionArg *out) {
    Fy_ParserState backtrack;

    if ((out->as_memory = Fy_Parser_parseMemExpr(parser))) {
        out->type = Fy_InstructionArgType_Memory;
        return true;
    }

    Fy_Parser_dumpState(parser, &backtrack);
    if (!Fy_Parser_lex(parser))
        return false;

    if (Fy_TokenType_isReg16(parser->token.type)) {
        out->type = Fy_InstructionArgType_Reg16;
        out->as_reg16 = Fy_TokenType_toReg16(parser->token.type);
        return true;
    }

    if (Fy_TokenType_isReg8(parser->token.type)) {
        out->type = Fy_InstructionArgType_Reg8;
        out->as_reg8 = Fy_TokenType_toReg8(parser->token.type);
        return true;
    }

    if (parser->token.type == Fy_TokenType_Label) {
        out->type = Fy_InstructionArgType_Label;
        out->as_label = Fy_Token_toLowercaseCStr(&parser->token);
        return true;
    }

    if (parser->token.type == Fy_TokenType_Const) {
        out->type = Fy_InstructionArgType_Constant;
        out->as_const = Fy_Token_toConst16(&parser->token, parser);
        return true;
    }

    Fy_Parser_loadState(parser, &backtrack);
    return false;
}

static Fy_Instruction *Fy_Parser_parseInstruction(Fy_Parser *parser) {
    Fy_ParserState start_backtrack;
    Fy_TokenType start_token;
    Fy_ParserParseRuleType type;
    Fy_InstructionArg arg1, arg2;
    Fy_ParserParseRule *rule;
    Fy_Instruction *instruction = NULL;

    Fy_Parser_dumpState(parser, &start_backtrack);

    // If you can't even lex, don't start parsing
    if (!Fy_Parser_lex(parser))
        return NULL;
    start_token = parser->token.type;

    if (Fy_Parser_parseArgument(parser, &arg1)) {
        if (Fy_Parser_parseArgument(parser, &arg2))
            type = Fy_ParserParseRuleType_TwoParams;
        else
            type = Fy_ParserParseRuleType_OneParam;
    } else {
        type = Fy_ParserParseRuleType_NoParams;
    }
    Fy_Parser_expectNewline(parser, true);

    for (size_t i = 0; !instruction && i < sizeof(Fy_parserRules) / sizeof(Fy_ParserParseRule*); ++i) {
        rule = Fy_parserRules[i];

        if (start_token == rule->start_token && type == rule->type) {
            switch (type) {
            case Fy_ParserParseRuleType_NoParams:
                instruction = rule->func_no_params(parser);
                break;
            case Fy_ParserParseRuleType_OneParam:
                if (arg1.type == rule->arg1_type)
                    instruction = rule->func_one_param(parser, &arg1);
                break;
            case Fy_ParserParseRuleType_TwoParams:
                if (arg1.type == rule->arg1_type && arg2.type == rule->arg2_type)
                    instruction = rule->func_two_params(parser, &arg1, &arg2);
                break;
            default:
                FY_UNREACHABLE();
            }
        }
    }

    if (instruction) {
        instruction->parse_rule = rule;
        instruction->start_state = start_backtrack;
        return instruction;
    }

    // Remove arguments
    switch (type) {
    case Fy_ParserParseRuleType_NoParams:
        break;
    case Fy_ParserParseRuleType_OneParam:
        Fy_InstructionArg_Destruct(&arg1);
        break;
    case Fy_ParserParseRuleType_TwoParams:
        Fy_InstructionArg_Destruct(&arg1);
        Fy_InstructionArg_Destruct(&arg2);
        break;
    default:
        FY_UNREACHABLE();
    }

    // TODO: Add clever error message that tells us why we were wrong

    Fy_Parser_loadState(parser, &start_backtrack);
    Fy_Parser_error(parser, Fy_ParserError_InvalidInstruction, NULL, NULL);

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
        Fy_Parser_error(parser, Fy_ParserError_SyntaxError, NULL, NULL);

    // Advance newline if there is
    Fy_Parser_expectNewline(parser, false);

    label_string = Fy_Token_toLowercaseCStr(&label_token);
    Fy_Labelmap_addMemLabelDecl(&parser->labelmap, label_string, parser->amount_used);

    return true;
}

static bool Fy_Parser_parseProc(Fy_Parser *parser) {
    char *label_string;
    uint16_t amount_prev_instructions = parser->amount_used;

    if (!Fy_Parser_match(parser, Fy_TokenType_Proc))
        return false;

    if (!Fy_Parser_match(parser, Fy_TokenType_Label))
        Fy_Parser_error(parser, Fy_ParserError_SyntaxError, NULL, NULL);

    label_string = Fy_Token_toLowercaseCStr(&parser->token);

    Fy_Parser_expectNewline(parser, true);

    for (;;) {
        if (Fy_Parser_match(parser, Fy_TokenType_Endp)) {
            char *endp_label_string;
            if (!Fy_Parser_match(parser, Fy_TokenType_Label))
                Fy_Parser_error(parser, Fy_ParserError_SyntaxError, NULL, NULL);
            endp_label_string = Fy_Token_toLowercaseCStr(&parser->token);
            if (strcmp(label_string, endp_label_string) != 0)
                Fy_Parser_error(parser, Fy_ParserError_UnexpectedLabel, NULL, "Expected '%s'", label_string);
            free(endp_label_string);
            Fy_Parser_expectNewline(parser, true);
            break;
        }
        if (!Fy_Parser_parseLine(parser))
            Fy_Parser_error(parser, Fy_ParserError_UnexpectedToken, NULL, NULL);
    }

    Fy_Labelmap_addMemLabelDecl(&parser->labelmap, label_string, amount_prev_instructions);

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
        return true;
    }

    return false;
}

static void Fy_Parser_parseSetVariable(Fy_Parser *parser) {
    char *label;
    uint16_t variable_offset;

    if (Fy_Parser_match(parser, Fy_TokenType_Label)) {
        label = Fy_Token_toLowercaseCStr(&parser->token);
        // If there is a label defined with that name
        if (Fy_Labelmap_getEntry(&parser->labelmap, label))
            Fy_Parser_error(parser, Fy_ParserError_LabelAlreadyExists, NULL, "%s", label);
    } else {
        label = NULL;
    }

    variable_offset = parser->data_size;

    if (Fy_Parser_match(parser, Fy_TokenType_Eb)) {
        do {
            int8_t value;
            if (!Fy_Parser_match(parser, Fy_TokenType_Const))
                Fy_Parser_error(parser, Fy_ParserError_ExpectedDifferentToken, NULL, "Constant");
            value = Fy_Token_toConst8(&parser->token, parser);
            Fy_Parser_addData8(parser, value);
        } while (Fy_Parser_match(parser, Fy_TokenType_Comma));
    } else if (Fy_Parser_match(parser, Fy_TokenType_Ew)) {
        do {
            int16_t value;
            if (!Fy_Parser_match(parser, Fy_TokenType_Const))
                Fy_Parser_error(parser, Fy_ParserError_ExpectedDifferentToken, NULL, "Constant");
            value = Fy_Token_toConst16(&parser->token, parser);
            Fy_Parser_addData16(parser, value);
        } while (Fy_Parser_match(parser, Fy_TokenType_Comma));
    } else {
        Fy_Parser_error(parser, Fy_ParserError_ExpectedDifferentToken, NULL, "EB or EW");
        FY_UNREACHABLE();
    }

    if (label)
        Fy_Labelmap_addVariable(&parser->labelmap, label, variable_offset);

    Fy_Parser_expectNewline(parser, true);
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
        Fy_Parser_error(parser, Fy_ParserError_SyntaxError, NULL, NULL);
    }
}

/* Step 2: processing parsed instructions */
static void Fy_Parser_processInstructions(Fy_Parser *parser) {
    for (size_t i = 0; i < parser->amount_used; ++i) {
        Fy_Instruction *instruction = parser->instructions[i];
        Fy_Parser_loadState(parser, &instruction->start_state);
        if (instruction->parse_rule->process_func) {
            instruction->parse_rule->process_func(parser, instruction);
        }
    }
}

uint16_t Fy_Parser_getCodeOffsetByInstructionIndex(Fy_Parser *parser, size_t index) {
    assert(index <= parser->amount_used);
    if (index < parser->amount_used) {
        return parser->instructions[index]->code_offset;
    } else {
        return parser->code_size;
    }
}


/* Store code offsets in the instructions themselves so we can later store the offsets as labels */
static void Fy_Parser_storeOffsetsInInstructions(Fy_Parser *parser) {
    parser->code_size = 0;
    for (size_t i = 0; i < parser->amount_used; ++i) {
        Fy_Instruction *instruction = parser->instructions[i];
        uint16_t size;
        // Calculate new offset
        if (i == 0) {
            instruction->code_offset = 0;
        } else {
            Fy_Instruction *prev_instruction = parser->instructions[i - 1];
            instruction->code_offset = prev_instruction->code_offset + prev_instruction->size;
        }
        if (instruction->type->variable_size)
            size = 1 + instruction->type->getsize_func(instruction);
        else
            size = 1 + instruction->type->additional_size;
        instruction->size = size;
        parser->code_size += size;
    }
}

/* Step 3: assigning relative addresses to label references */
static void Fy_Parser_processLabels(Fy_Parser *parser) {
    Fy_Parser_storeOffsetsInInstructions(parser);
    for (size_t i = 0; i < parser->amount_used; ++i) {
        Fy_Instruction *instruction = parser->instructions[i];
        if (instruction->parse_rule->process_label_func) {
            instruction->parse_rule->process_label_func(instruction, parser);
        }
    }
}

void Fy_Parser_parseAll(Fy_Parser *parser) {
    // Advance newline if there is one
    Fy_Parser_expectNewline(parser, false);
    if (Fy_Parser_match(parser, Fy_TokenType_Data)) {
        Fy_Parser_expectNewline(parser, true);
        while (!Fy_Parser_match(parser, Fy_TokenType_Code)) {
            Fy_Parser_parseSetVariable(parser);
        }
    } else if (!Fy_Parser_match(parser, Fy_TokenType_Code)) {
        Fy_Parser_error(parser, Fy_ParserError_ExpectedDifferentToken, NULL, "CODE");
    }

    Fy_Parser_readInstructions(parser);
    Fy_Parser_processInstructions(parser);
    Fy_Parser_processLabels(parser);
}

void Fy_Parser_logParsed(Fy_Parser *parser) {
    (void)parser;
    FY_UNREACHABLE();
    // printf("--- %zu instructions ---\n", parser->amount_used);
    // for (size_t i = 0; i < parser->amount_used; ++i) {
    //     Fy_Instruction *instruction = parser->instructions[i];
    //     switch (instruction->type) {
    //     case Fy_instructionTypeMovReg16Const:
    //         printf("mov %s %d",
    //                 Fy_ParserReg16_toString(instruction->mov_reg16_const.reg_id),
    //                 instruction->mov_reg16_const.const16);
    //         break;
    //     case Fy_instructionTypeMovReg16Reg16:
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

    // Add size of data, code and stack to header
    Fy_Generator_addWord(&generator, parser->data_size);
    Fy_Generator_addWord(&generator, parser->code_size);
    Fy_Generator_addWord(&generator, 0x100); // Stack size

    // TODO: Optimize this
    // Add all of the data bytes
    for (size_t i = 0; i < parser->data_size; ++i)
        Fy_Generator_addByte(&generator, parser->data_part[i]);

    // Add all of the instructions
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
        Fy_Parser_error(parser, Fy_ParserError_CannotOpenFileForWrite, NULL, "%s", filename);

    Fy_Parser_generateBytecode(parser, &generator);
    fwrite(generator.output, 1, generator.idx, file);
    Fy_Generator_Deallocate(&generator);

    fclose(file);
}
