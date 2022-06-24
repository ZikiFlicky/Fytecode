#include "fy.h"

/* Parse-function (step 1) declarations */
static Fy_Instruction *Fy_ParseNop(Fy_Parser *parser);
static Fy_Instruction *Fy_ParseDebug(Fy_Parser *parser);
static Fy_Instruction *Fy_ParseDebugStack(Fy_Parser *parser);
static Fy_Instruction *Fy_ParseEnd(Fy_Parser *parser);
static Fy_Instruction *Fy_ParseRet(Fy_Parser *parser);
static Fy_Instruction *Fy_ParseCbw(Fy_Parser *parser);
static Fy_Instruction *Fy_ParseRetConst16(Fy_Parser *parser, Fy_InstructionArg *arg);
static Fy_Instruction *Fy_ParsePushConst(Fy_Parser *parser, Fy_InstructionArg *arg);
static Fy_Instruction *Fy_ParsePushReg16(Fy_Parser *parser, Fy_InstructionArg *arg);
static Fy_Instruction *Fy_ParsePop(Fy_Parser *parser, Fy_InstructionArg *arg);
static Fy_Instruction *Fy_ParseInt(Fy_Parser *parser, Fy_InstructionArg *arg);
static Fy_Instruction *Fy_ParseMulReg16(Fy_Parser *parser, Fy_InstructionArg *arg);
static Fy_Instruction *Fy_ParseMulReg8(Fy_Parser *parser, Fy_InstructionArg *arg);
static Fy_Instruction *Fy_ParseImulReg16(Fy_Parser *parser, Fy_InstructionArg *arg);
static Fy_Instruction *Fy_ParseImulReg8(Fy_Parser *parser, Fy_InstructionArg *arg);
static Fy_Instruction *Fy_ParseDivReg16(Fy_Parser *parser, Fy_InstructionArg *arg);
static Fy_Instruction *Fy_ParseDivReg8(Fy_Parser *parser, Fy_InstructionArg *arg);
static Fy_Instruction *Fy_ParseIdivReg16(Fy_Parser *parser, Fy_InstructionArg *arg);
static Fy_Instruction *Fy_ParseIdivReg8(Fy_Parser *parser, Fy_InstructionArg *arg);
static Fy_Instruction *Fy_ParseLea(Fy_Parser *parser, Fy_InstructionArg *arg1, Fy_InstructionArg *arg2);

/* Process-function (parsing step 2) declarations */
static void Fy_ProcessOpLabel(Fy_Parser *parser, Fy_Instruction_OpLabel *instruction);
static void Fy_ProcessOpReg16Mem(Fy_Parser *parser, Fy_Instruction_OpReg16Mem *instruction);

/* Process-label-function (parsing step 3) declarations */
static void Fy_ProcessLabelOpLabel(Fy_Parser *parser, Fy_Instruction_OpLabel *instruction);

/* Custom instruction delete functions */
static void Fy_DeleteLea(Fy_Instruction_OpReg16Mem *instruction);

/* Function to parse anything found in text */
static bool Fy_Parser_parseLine(Fy_Parser *parser);

// TODO: Move this from here
typedef struct Fy_BinaryOperatorRule {
    Fy_InstructionArgType arg1_type, arg2_type;
    Fy_BinaryOperatorArgsType binary_instruction_type;
} Fy_BinaryOperatorRule;

typedef struct Fy_UnaryOperatorRule {
    Fy_InstructionArgType arg_type;
    Fy_UnaryOperatorArgsType unary_instruction_type;
} Fy_UnaryOperatorRule;

/* Define rules */
static const Fy_ParserParseRule Fy_parseRuleNop = {
    .type = Fy_ParserParseRuleType_Custom,
    .start_token = Fy_TokenType_Nop,
    .as_custom = {
        .amount_params = 0,
        .func_no_params = Fy_ParseNop,
        .process_func = NULL,
        .process_label_func = NULL,
        .delete_func = NULL
    }
};
static const Fy_ParserParseRule Fy_parseRuleDebug = {
    .type = Fy_ParserParseRuleType_Custom,
    .start_token = Fy_TokenType_Debug,
    .as_custom = {
        .amount_params = 0,
        .func_no_params = Fy_ParseDebug,
        .process_func = NULL,
        .process_label_func = NULL,
        .delete_func = NULL
    }
};
static const Fy_ParserParseRule Fy_parseRuleDebugStack = {
    .type = Fy_ParserParseRuleType_Custom,
    .start_token = Fy_TokenType_DebugStack,
    .as_custom = {
        .amount_params = 0,
        .func_no_params = Fy_ParseDebugStack,
        .process_func = NULL,
        .process_label_func = NULL,
        .delete_func = NULL
    }
};
static const Fy_ParserParseRule Fy_parseRuleEnd = {
    .type = Fy_ParserParseRuleType_Custom,
    .start_token = Fy_TokenType_End,
    .as_custom = {
        .amount_params = 0,
        .func_no_params = Fy_ParseEnd,
        .process_func = NULL,
        .process_label_func = NULL,
        .delete_func = NULL
    }
};
static const Fy_ParserParseRule Fy_parseRuleJmp = {
    .type = Fy_ParserParseRuleType_Jump,
    .start_token = Fy_TokenType_Jmp,
    .as_jump = {
        .instruction_type = &Fy_instructionTypeJmp
    }
};
static const Fy_ParserParseRule Fy_parseRuleJe = {
    .type = Fy_ParserParseRuleType_Jump,
    .start_token = Fy_TokenType_Je,
    .as_jump = {
        .instruction_type = &Fy_instructionTypeJe
    }
};
static const Fy_ParserParseRule Fy_parseRuleJne = {
    .type = Fy_ParserParseRuleType_Jump,
    .start_token = Fy_TokenType_Jne,
    .as_jump = {
        .instruction_type = &Fy_instructionTypeJne
    }
};
static const Fy_ParserParseRule Fy_parseRuleJz = {
    .type = Fy_ParserParseRuleType_Jump,
    .start_token = Fy_TokenType_Jz,
    .as_jump = {
        .instruction_type = &Fy_instructionTypeJe
    }
};
static const Fy_ParserParseRule Fy_parseRuleJnz = {
    .type = Fy_ParserParseRuleType_Jump,
    .start_token = Fy_TokenType_Jnz,
    .as_jump = {
        .instruction_type = &Fy_instructionTypeJne
    }
};
static const Fy_ParserParseRule Fy_parseRuleJb = {
    .type = Fy_ParserParseRuleType_Jump,
    .start_token = Fy_TokenType_Jb,
    .as_jump = {
        .instruction_type = &Fy_instructionTypeJb
    }
};
static const Fy_ParserParseRule Fy_parseRuleJbe = {
    .type = Fy_ParserParseRuleType_Jump,
    .start_token = Fy_TokenType_Jbe,
    .as_jump = {
        .instruction_type = &Fy_instructionTypeJbe
    }
};
static const Fy_ParserParseRule Fy_parseRuleJa = {
    .type = Fy_ParserParseRuleType_Jump,
    .start_token = Fy_TokenType_Ja,
    .as_jump = {
        .instruction_type = &Fy_instructionTypeJa
    }
};
static const Fy_ParserParseRule Fy_parseRuleJae = {
    .type = Fy_ParserParseRuleType_Jump,
    .start_token = Fy_TokenType_Jae,
    .as_jump = {
        .instruction_type = &Fy_instructionTypeJae
    }
};
static const Fy_ParserParseRule Fy_parseRuleJl = {
    .type = Fy_ParserParseRuleType_Jump,
    .start_token = Fy_TokenType_Jl,
    .as_jump = {
        .instruction_type = &Fy_instructionTypeJl
    }
};
static const Fy_ParserParseRule Fy_parseRuleJle = {
    .type = Fy_ParserParseRuleType_Jump,
    .start_token = Fy_TokenType_Jle,
    .as_jump = {
        .instruction_type = &Fy_instructionTypeJle
    }
};
static const Fy_ParserParseRule Fy_parseRuleJg = {
    .type = Fy_ParserParseRuleType_Jump,
    .start_token = Fy_TokenType_Jg,
    .as_jump = {
        .instruction_type = &Fy_instructionTypeJg
    }
};
static const Fy_ParserParseRule Fy_parseRuleJge = {
    .type = Fy_ParserParseRuleType_Jump,
    .start_token = Fy_TokenType_Jge,
    .as_jump = {
        .instruction_type = &Fy_instructionTypeJge
    }
};
static const Fy_ParserParseRule Fy_parseRulePushConst = {
    .type = Fy_ParserParseRuleType_Custom,
    .start_token = Fy_TokenType_Push,
    .as_custom = {
        .amount_params = 1,
        .arg1_type = Fy_InstructionArgType_Const16,
        .func_one_param = Fy_ParsePushConst,
        .process_func = NULL,
        .process_label_func = NULL,
        .delete_func = NULL
    }
};
static const Fy_ParserParseRule Fy_parseRulePushReg16 = {
    .type = Fy_ParserParseRuleType_Custom,
    .start_token = Fy_TokenType_Push,
    .as_custom = {
        .amount_params = 1,
        .arg1_type = Fy_InstructionArgType_Reg16,
        .func_one_param = Fy_ParsePushReg16,
        .process_func = NULL,
        .process_label_func = NULL,
        .delete_func = NULL
    }
};
static const Fy_ParserParseRule Fy_parseRulePop = {
    .type = Fy_ParserParseRuleType_Custom,
    .start_token = Fy_TokenType_Pop,
    .as_custom = {
        .amount_params = 1,
        .arg1_type = Fy_InstructionArgType_Reg16,
        .func_one_param = Fy_ParsePop,
        .process_func = NULL,
        .process_label_func = NULL,
        .delete_func = NULL
    }
};
static const Fy_ParserParseRule Fy_parseRuleCall = {
    .type = Fy_ParserParseRuleType_Jump,
    .start_token = Fy_TokenType_Call,
    .as_jump = {
        .instruction_type = &Fy_instructionTypeCall
    }
};
static const Fy_ParserParseRule Fy_parseRuleRet = {
    .type = Fy_ParserParseRuleType_Custom,
    .start_token = Fy_TokenType_Ret,
    .as_custom = {
        .amount_params = 0,
        .func_no_params = Fy_ParseRet,
        .process_func = NULL,
        .process_label_func = NULL,
        .delete_func = NULL
    }
};
static const Fy_ParserParseRule Fy_parseRuleRetConst16 = {
    .type = Fy_ParserParseRuleType_Custom,
    .start_token = Fy_TokenType_Ret,
    .as_custom = {
        .amount_params = 1,
        .arg1_type = Fy_InstructionArgType_Const16,
        .func_one_param = Fy_ParseRetConst16,
        .process_func = NULL,
        .process_label_func = NULL,
        .delete_func = NULL
    }
};
static const Fy_ParserParseRule Fy_parseRuleLea = {
    .type = Fy_ParserParseRuleType_Custom,
    .start_token = Fy_TokenType_Lea,
    .as_custom = {
        .amount_params = 2,
        .arg1_type = Fy_InstructionArgType_Reg16,
        .arg2_type = Fy_InstructionArgType_Memory16,
        .func_two_params = Fy_ParseLea,
        .process_func = (Fy_InstructionProcessFunc)Fy_ProcessOpReg16Mem,
        .process_label_func = NULL,
        .delete_func = (Fy_InstructionCustomDeleteFunc)Fy_DeleteLea
    }
};
static const Fy_ParserParseRule Fy_parseRuleInt = {
    .type = Fy_ParserParseRuleType_Custom,
    .start_token = Fy_TokenType_Int,
    .as_custom = {
        .amount_params = 1,
        .arg1_type = Fy_InstructionArgType_Const16,
        .func_one_param = Fy_ParseInt,
        .process_func = NULL,
        .process_label_func = NULL,
        .delete_func = NULL
    }
};
static const Fy_ParserParseRule Fy_parseRuleDivReg16 = {
    .type = Fy_ParserParseRuleType_Custom,
    .start_token = Fy_TokenType_Div,
    .as_custom = {
        .amount_params = 1,
        .arg1_type = Fy_InstructionArgType_Reg16,
        .func_one_param = Fy_ParseDivReg16,
        .process_func = NULL,
        .process_label_func = NULL,
        .delete_func = NULL
    }
};
static const Fy_ParserParseRule Fy_parseRuleDivReg8 = {
    .type = Fy_ParserParseRuleType_Custom,
    .start_token = Fy_TokenType_Div,
    .as_custom = {
        .amount_params = 1,
        .arg1_type = Fy_InstructionArgType_Reg8,
        .func_one_param = Fy_ParseDivReg8,
        .process_func = NULL,
        .process_label_func = NULL,
        .delete_func = NULL
    }
};
static const Fy_ParserParseRule Fy_parseRuleIdivReg16 = {
    .type = Fy_ParserParseRuleType_Custom,
    .start_token = Fy_TokenType_Idiv,
    .as_custom = {
        .amount_params = 1,
        .arg1_type = Fy_InstructionArgType_Reg16,
        .func_one_param = Fy_ParseIdivReg16,
        .process_func = NULL,
        .process_label_func = NULL,
        .delete_func = NULL
    }
};
static const Fy_ParserParseRule Fy_parseRuleIdivReg8 = {
    .type = Fy_ParserParseRuleType_Custom,
    .start_token = Fy_TokenType_Idiv,
    .as_custom = {
        .amount_params = 1,
        .arg1_type = Fy_InstructionArgType_Reg8,
        .func_one_param = Fy_ParseIdivReg8,
        .process_func = NULL,
        .process_label_func = NULL,
        .delete_func = NULL
    }
};
static const Fy_ParserParseRule Fy_parseRuleMulReg16 = {
    .type = Fy_ParserParseRuleType_Custom,
    .start_token = Fy_TokenType_Mul,
    .as_custom = {
        .amount_params = 1,
        .arg1_type = Fy_InstructionArgType_Reg16,
        .func_one_param = Fy_ParseMulReg16,
        .process_func = NULL,
        .process_label_func = NULL,
        .delete_func = NULL
    }
};
static const Fy_ParserParseRule Fy_parseRuleMulReg8 = {
    .type = Fy_ParserParseRuleType_Custom,
    .start_token = Fy_TokenType_Mul,
    .as_custom = {
        .amount_params = 1,
        .arg1_type = Fy_InstructionArgType_Reg8,
        .func_one_param = Fy_ParseMulReg8,
        .process_func = NULL,
        .process_label_func = NULL,
        .delete_func = NULL
    }
};
static const Fy_ParserParseRule Fy_parseRuleImulReg16 = {
    .type = Fy_ParserParseRuleType_Custom,
    .start_token = Fy_TokenType_Imul,
    .as_custom = {
        .amount_params = 1,
        .arg1_type = Fy_InstructionArgType_Reg16,
        .func_one_param = Fy_ParseImulReg16,
        .process_func = NULL,
        .process_label_func = NULL,
        .delete_func = NULL
    }
};
static const Fy_ParserParseRule Fy_parseRuleImulReg8 = {
    .type = Fy_ParserParseRuleType_Custom,
    .start_token = Fy_TokenType_Imul,
    .as_custom = {
        .amount_params = 1,
        .arg1_type = Fy_InstructionArgType_Reg8,
        .func_one_param = Fy_ParseImulReg8,
        .process_func = NULL,
        .process_label_func = NULL,
        .delete_func = NULL
    }
};
static const Fy_ParserParseRule Fy_parseRuleMov = {
    .type = Fy_ParserParseRuleType_BinaryOperator,
    .start_token = Fy_TokenType_Mov,
    .as_binary = {
        .operator_id = Fy_BinaryOperator_Mov
    }
};
static const Fy_ParserParseRule Fy_parseRuleAdd = {
    .type = Fy_ParserParseRuleType_BinaryOperator,
    .start_token = Fy_TokenType_Add,
    .as_binary = {
        .operator_id = Fy_BinaryOperator_Add
    }
};
static const Fy_ParserParseRule Fy_parseRuleSub = {
    .type = Fy_ParserParseRuleType_BinaryOperator,
    .start_token = Fy_TokenType_Sub,
    .as_binary = {
        .operator_id = Fy_BinaryOperator_Sub
    }
};
static const Fy_ParserParseRule Fy_parseRuleAnd = {
    .type = Fy_ParserParseRuleType_BinaryOperator,
    .start_token = Fy_TokenType_And,
    .as_binary = {
        .operator_id = Fy_BinaryOperator_And
    }
};
static const Fy_ParserParseRule Fy_parseRuleOr = {
    .type = Fy_ParserParseRuleType_BinaryOperator,
    .start_token = Fy_TokenType_Or,
    .as_binary = {
        .operator_id = Fy_BinaryOperator_Or
    }
};
static const Fy_ParserParseRule Fy_parseRuleXor = {
    .type = Fy_ParserParseRuleType_BinaryOperator,
    .start_token = Fy_TokenType_Xor,
    .as_binary = {
        .operator_id = Fy_BinaryOperator_Xor
    }
};
static const Fy_ParserParseRule Fy_parseRuleShl = {
    .type = Fy_ParserParseRuleType_BinaryOperator,
    .start_token = Fy_TokenType_Shl,
    .as_binary = {
        .operator_id = Fy_BinaryOperator_Shl
    }
};
static const Fy_ParserParseRule Fy_parseRuleShr = {
    .type = Fy_ParserParseRuleType_BinaryOperator,
    .start_token = Fy_TokenType_Shr,
    .as_binary = {
        .operator_id = Fy_BinaryOperator_Shr
    }
};
static const Fy_ParserParseRule Fy_parseRuleCmp = {
    .type = Fy_ParserParseRuleType_BinaryOperator,
    .start_token = Fy_TokenType_Cmp,
    .as_binary = {
        .operator_id = Fy_BinaryOperator_Cmp
    }
};
static const Fy_ParserParseRule Fy_parseRuleNeg = {
    .type = Fy_ParserParseRuleType_UnaryOperator,
    .start_token = Fy_TokenType_Neg,
    .as_unary = {
        .operator_id = Fy_UnaryOperator_Neg
    }
};
static const Fy_ParserParseRule Fy_parseRuleInc = {
    .type = Fy_ParserParseRuleType_UnaryOperator,
    .start_token = Fy_TokenType_Inc,
    .as_unary = {
        .operator_id = Fy_UnaryOperator_Inc
    }
};
static const Fy_ParserParseRule Fy_parseRuleDec = {
    .type = Fy_ParserParseRuleType_UnaryOperator,
    .start_token = Fy_TokenType_Dec,
    .as_unary = {
        .operator_id = Fy_UnaryOperator_Dec
    }
};
static const Fy_ParserParseRule Fy_parseRuleNot = {
    .type = Fy_ParserParseRuleType_UnaryOperator,
    .start_token = Fy_TokenType_Not,
    .as_unary = {
        .operator_id = Fy_UnaryOperator_Not
    }
};
static const Fy_ParserParseRule Fy_parseRuleCbw = {
    .type = Fy_ParserParseRuleType_Custom,
    .start_token = Fy_TokenType_Cbw,
    .as_custom = {
        .amount_params = 0,
        .func_no_params = Fy_ParseCbw,
        .process_func = NULL,
        .process_label_func = NULL,
        .delete_func = NULL
    }
};

/* Array that stores all rules (pointers to rules) */
static const Fy_ParserParseRule* const Fy_parserRules[] = {
    &Fy_parseRuleNop,
    &Fy_parseRuleMov,
    &Fy_parseRuleAdd,
    &Fy_parseRuleSub,
    &Fy_parseRuleAnd,
    &Fy_parseRuleOr,
    &Fy_parseRuleXor,
    &Fy_parseRuleShl,
    &Fy_parseRuleShr,
    &Fy_parseRuleCmp,
    &Fy_parseRuleDebug,
    &Fy_parseRuleDebugStack,
    &Fy_parseRuleEnd,
    &Fy_parseRuleJmp,
    &Fy_parseRuleJe,
    &Fy_parseRuleJne,
    &Fy_parseRuleJz,
    &Fy_parseRuleJnz,
    &Fy_parseRuleJb,
    &Fy_parseRuleJbe,
    &Fy_parseRuleJa,
    &Fy_parseRuleJae,
    &Fy_parseRuleJl,
    &Fy_parseRuleJle,
    &Fy_parseRuleJg,
    &Fy_parseRuleJge,
    &Fy_parseRulePushConst,
    &Fy_parseRulePushReg16,
    &Fy_parseRulePop,
    &Fy_parseRuleCall,
    &Fy_parseRuleRet,
    &Fy_parseRuleRetConst16,
    &Fy_parseRuleLea,
    &Fy_parseRuleInt,
    &Fy_parseRuleDivReg16,
    &Fy_parseRuleDivReg8,
    &Fy_parseRuleIdivReg16,
    &Fy_parseRuleIdivReg8,
    &Fy_parseRuleMulReg16,
    &Fy_parseRuleMulReg8,
    &Fy_parseRuleImulReg16,
    &Fy_parseRuleImulReg8,
    &Fy_parseRuleNeg,
    &Fy_parseRuleInc,
    &Fy_parseRuleDec,
    &Fy_parseRuleNot,
    &Fy_parseRuleCbw
};

/* Binary expression instruction rules */
static const Fy_BinaryOperatorRule Fy_binaryOperatorRules[] = {
    { Fy_InstructionArgType_Reg16, Fy_InstructionArgType_Const16, Fy_BinaryOperatorArgsType_Reg16Const },
    { Fy_InstructionArgType_Reg16, Fy_InstructionArgType_Reg16, Fy_BinaryOperatorArgsType_Reg16Reg16 },
    { Fy_InstructionArgType_Reg16, Fy_InstructionArgType_Memory16, Fy_BinaryOperatorArgsType_Reg16Memory16 },
    { Fy_InstructionArgType_Reg8, Fy_InstructionArgType_Const8, Fy_BinaryOperatorArgsType_Reg8Const },
    { Fy_InstructionArgType_Reg8, Fy_InstructionArgType_Reg8, Fy_BinaryOperatorArgsType_Reg8Reg8 },
    { Fy_InstructionArgType_Reg8, Fy_InstructionArgType_Memory8, Fy_BinaryOperatorArgsType_Reg8Memory8 },
    { Fy_InstructionArgType_Memory16, Fy_InstructionArgType_Const16, Fy_BinaryOperatorArgsType_Memory16Const },
    { Fy_InstructionArgType_Memory16, Fy_InstructionArgType_Reg16, Fy_BinaryOperatorArgsType_Memory16Reg16 },
    { Fy_InstructionArgType_Memory8, Fy_InstructionArgType_Const8, Fy_BinaryOperatorArgsType_Memory8Const },
    { Fy_InstructionArgType_Memory8, Fy_InstructionArgType_Reg8, Fy_BinaryOperatorArgsType_Memory8Reg8 }
};

/* Unary expression instruction rules */
static const Fy_UnaryOperatorRule Fy_unaryOperatorRules[] = {
    { Fy_InstructionArgType_Reg16, Fy_UnaryOperatorArgsType_Reg16 },
    { Fy_InstructionArgType_Memory16, Fy_UnaryOperatorArgsType_Mem16 },
    { Fy_InstructionArgType_Reg8, Fy_UnaryOperatorArgsType_Reg8 },
    { Fy_InstructionArgType_Memory8, Fy_UnaryOperatorArgsType_Mem8 },
};

/* Returns whether `type1` can be considered of type `type2` */
static bool Fy_InstructionArgType_is(Fy_InstructionArgType type1, Fy_InstructionArgType type2) {
    if (type1 == type2)
        return true;
    if (type1 == Fy_InstructionArgType_MemoryUnknownSize && type2 == Fy_InstructionArgType_Memory16)
        return true;
    if (type1 == Fy_InstructionArgType_MemoryUnknownSize && type2 == Fy_InstructionArgType_Memory8)
        return true;
    if (type1 == Fy_InstructionArgType_Const8 && type2 == Fy_InstructionArgType_Const16)
        return true;
    return false;
}

static void Fy_InstructionArg_Destruct(Fy_InstructionArg *arg) {
    switch (arg->type) {
    case Fy_InstructionArgType_Label:
        free(arg->as_label);
        break;
    case Fy_InstructionArgType_Memory8:
    case Fy_InstructionArgType_Memory16:
        Fy_AST_Delete(arg->as_memory);
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
    case Fy_ParserError_SymbolNotFound:
        return "Symbol not found";
    case Fy_ParserError_UnexpectedSymbol:
        return "Unexpected symbol";
    case Fy_ParserError_ExpectedDifferentToken:
        return "Expected different token";
    case Fy_ParserError_SymbolNotCode:
        return "Symbol doesn't reference code and is most likely a variable";
    case Fy_ParserError_SymbolNotVariable:
        return "Symbol doesn't reference a variable and is most likely a code/procedure reference";
    case Fy_ParserError_SymbolAlreadyDefined:
        return "Symbol already defined";
    case Fy_ParserError_RecursiveMacro:
        return "Recursive macro definition";
    case Fy_ParserError_MaxMacroDepthReached:
        return "Max macro depth reached";
    case Fy_ParserError_InvalidInlineValue:
        return "Invalid inline value";
    case Fy_ParserError_InterruptNotFound:
        return "Interrupt not found";
    case Fy_ParserError_InvalidOperation:
        return "Invalid operation";
    case Fy_ParserError_AmbiguousInstructionParameters:
        return "Ambiguous instruction parameters";
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
    out->amount_macros = 0;
    Fy_Symbolmap_Init(&out->symmap);
}

void Fy_Parser_Destruct(Fy_Parser *parser) {
    if (parser->amount_allocated > 0) {
        for (size_t i = 0; i < parser->amount_used; ++i) {
            Fy_Instruction *instruction = parser->instructions[i];
            Fy_Instruction_Delete(instruction);
        }
        free(parser->instructions);
    }
    if (parser->data_allocated > 0)
        free(parser->data_part);
    Fy_Symbolmap_Destruct(&parser->symmap);
}

void Fy_Parser_dumpState(Fy_Parser *parser, Fy_ParserState *out_state) {
    out_state->stream = parser->lexer->stream;
    out_state->line = parser->lexer->line;
    out_state->column = parser->lexer->column;
    out_state->amount_macros = parser->amount_macros;
    memcpy(out_state->macros, parser->macros, parser->amount_macros * sizeof(Fy_MacroEvalInstance));
}

void Fy_Parser_loadState(Fy_Parser *parser, Fy_ParserState *state) {
    parser->lexer->stream = state->stream;
    parser->lexer->line = state->line;
    parser->lexer->column = state->column;
    parser->amount_macros = state->amount_macros;
    memcpy(parser->macros, state->macros, state->amount_macros * sizeof(Fy_MacroEvalInstance));
}

static bool Fy_Parser_loadToken(Fy_Parser *parser, Fy_Token *token) {
    if (token->type == Fy_TokenType_Symbol) {
        char *name = Fy_Token_toLowercaseCStr(token);
        Fy_Macro *macro = Fy_Symbolmap_getMacro(&parser->symmap, name);
        free(name);
        if (macro) {
            Fy_MacroEvalInstance new_instance;

            for (size_t i = 0; i < parser->amount_macros; ++i) {
                if (macro == parser->macros[i].macro) {
                    // TODO: Show the macros that define themselves. maybe something like Fy_Parser_errorMacros
                    Fy_Parser_error(parser, Fy_ParserError_RecursiveMacro, NULL, NULL);
                }
            }

            if (parser->amount_macros >= FY_MACRO_DEPTH) {
                Fy_Parser_error(parser, Fy_ParserError_MaxMacroDepthReached, NULL, NULL);
            }

            new_instance.macro = macro;
            new_instance.macro_idx = 0;
            parser->macros[parser->amount_macros++] = new_instance;
            return Fy_Parser_lex(parser, true);
        }
    }
    parser->token = *token;
    return true;
}

bool Fy_Parser_lex(Fy_Parser *parser, bool macro_eval) {
    if (macro_eval && parser->amount_macros > 0) {
        Fy_MacroEvalInstance *instance = &parser->macros[parser->amount_macros - 1];
        if (instance->macro_idx < instance->macro->token_amount) {
            Fy_Token *token = &instance->macro->tokens[instance->macro_idx++];
            return Fy_Parser_loadToken(parser, token);
        }
        // If we got to the end of the macro
        --parser->amount_macros;
        return Fy_Parser_lex(parser, true);
    }

    if (!Fy_Lexer_lex(parser->lexer))
        return false;

    if (macro_eval) {
        return Fy_Parser_loadToken(parser, &parser->lexer->token);
    } else {
        parser->token = parser->lexer->token;
        return true;
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
    parser->data_part[parser->data_size] = value & 0xff;
    parser->data_part[parser->data_size + 1] = value >> 8;
    parser->data_size += 2;
}

/* Returns a boolean specifying whether a token of the given type was found. */
bool Fy_Parser_match(Fy_Parser *parser, Fy_TokenType type, bool macro_eval) {
    Fy_ParserState backtrack;
    Fy_Parser_dumpState(parser, &backtrack);

    if (!Fy_Parser_lex(parser, macro_eval) || parser->token.type != type) {
        Fy_Parser_loadState(parser, &backtrack);
        return false;
    }

    return true;
}

/* Parsing helpers */
static Fy_Instruction *Fy_ParseOpReg8(Fy_Parser *parser, Fy_InstructionArg *arg, const Fy_InstructionType *type) {
    Fy_Instruction_OpReg8 *instruction = FY_INSTRUCTION_NEW(Fy_Instruction_OpReg8, *type);
    (void)parser;
    instruction->reg_id = arg->as_reg8;
    return (Fy_Instruction*)instruction;
}

static Fy_Instruction *Fy_ParseOpReg16(Fy_Parser *parser, Fy_InstructionArg *arg, const Fy_InstructionType *type) {
    Fy_Instruction_OpReg16 *instruction = FY_INSTRUCTION_NEW(Fy_Instruction_OpReg16, *type);
    (void)parser;
    instruction->reg_id = arg->as_reg16;
    return (Fy_Instruction*)instruction;
}

static Fy_Instruction *Fy_ParseOpNoParams(Fy_Parser *parser, const Fy_InstructionType *type) {
    (void)parser;
    return FY_INSTRUCTION_NEW(Fy_Instruction, *type);
}

static Fy_Instruction *Fy_ParseOpConst16(Fy_Parser *parser, Fy_InstructionArg *arg, const Fy_InstructionType *type) {
    Fy_Instruction_OpConst16 *instruction = FY_INSTRUCTION_NEW(Fy_Instruction_OpConst16, *type);
    (void)parser;
    instruction->value = arg->as_const;
    return (Fy_Instruction*)instruction;
}

static Fy_Instruction *Fy_ParseOpReg16Mem(Fy_Parser *parser, Fy_InstructionArg *arg1, Fy_InstructionArg *arg2, const Fy_InstructionType *type) {
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

static Fy_Instruction *Fy_ParseDebug(Fy_Parser *parser) {
    return Fy_ParseOpNoParams(parser, &Fy_instructionTypeDebug);
}

static Fy_Instruction *Fy_ParseDebugStack(Fy_Parser *parser) {
    return Fy_ParseOpNoParams(parser, &Fy_instructionTypeDebugStack);
}

static Fy_Instruction *Fy_ParseEnd(Fy_Parser *parser) {
    return Fy_ParseOpNoParams(parser, &Fy_instructionTypeEndProgram);
}

static Fy_Instruction *Fy_ParseRet(Fy_Parser *parser) {
    return Fy_ParseOpNoParams(parser, &Fy_instructionTypeRet);
}

static Fy_Instruction *Fy_ParseCbw(Fy_Parser *parser) {
    return Fy_ParseOpNoParams(parser, &Fy_instructionTypeCbw);
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

static Fy_Instruction *Fy_ParseInt(Fy_Parser *parser, Fy_InstructionArg *arg) {
    Fy_Instruction_OpConst8 *instruction;
    Fy_InterruptRunFunc func;
    (void)parser;
    if (arg->as_const > 0xff)
        Fy_Parser_error(parser, Fy_ParserError_ConstTooBig, NULL, "%d", arg->as_const);
    func = Fy_findInterruptFuncByOpcode((uint8_t)arg->as_const);
    if (!func)
        Fy_Parser_error(parser, Fy_ParserError_InterruptNotFound, &arg->state, "%d", arg->as_const);
    instruction = FY_INSTRUCTION_NEW(Fy_Instruction_OpConst8, Fy_instructionTypeInt);
    instruction->value = arg->as_const;
    return (Fy_Instruction*)instruction;
}

static Fy_Instruction *Fy_ParseMulReg16(Fy_Parser *parser, Fy_InstructionArg *arg) {
    return Fy_ParseOpReg16(parser, arg, &Fy_instructionTypeMulReg16);
}

static Fy_Instruction *Fy_ParseMulReg8(Fy_Parser *parser, Fy_InstructionArg *arg) {
    return Fy_ParseOpReg8(parser, arg, &Fy_instructionTypeMulReg8);
}

static Fy_Instruction *Fy_ParseImulReg16(Fy_Parser *parser, Fy_InstructionArg *arg) {
    return Fy_ParseOpReg16(parser, arg, &Fy_instructionTypeImulReg16);
}

static Fy_Instruction *Fy_ParseImulReg8(Fy_Parser *parser, Fy_InstructionArg *arg) {
    return Fy_ParseOpReg8(parser, arg, &Fy_instructionTypeImulReg8);
}

static Fy_Instruction *Fy_ParseDivReg16(Fy_Parser *parser, Fy_InstructionArg *arg) {
    return Fy_ParseOpReg16(parser, arg, &Fy_instructionTypeDivReg16);
}

static Fy_Instruction *Fy_ParseDivReg8(Fy_Parser *parser, Fy_InstructionArg *arg) {
    return Fy_ParseOpReg8(parser, arg, &Fy_instructionTypeDivReg8);
}

static Fy_Instruction *Fy_ParseIdivReg16(Fy_Parser *parser, Fy_InstructionArg *arg) {
    return Fy_ParseOpReg16(parser, arg, &Fy_instructionTypeIdivReg16);
}

static Fy_Instruction *Fy_ParseIdivReg8(Fy_Parser *parser, Fy_InstructionArg *arg) {
    return Fy_ParseOpReg8(parser, arg, &Fy_instructionTypeIdivReg8);
}

static Fy_Instruction *Fy_ParseLea(Fy_Parser *parser, Fy_InstructionArg *arg1, Fy_InstructionArg *arg2) {
    return Fy_ParseOpReg16Mem(parser, arg1, arg2, &Fy_instructionTypeLea);
}

/* Processing functions */

static void Fy_ProcessOpLabel(Fy_Parser *parser, Fy_Instruction_OpLabel *instruction) {
    Fy_BucketNode *node;
    node = Fy_Symbolmap_getEntry(&parser->symmap, instruction->name);
    if (!node) {
        // FIXME: This needs to have the right line and columns
        Fy_Parser_error(parser, Fy_ParserError_SymbolNotFound, NULL, "%s", instruction->name);
    }
    if (node->type != Fy_MapEntryType_Label)
        Fy_Parser_error(parser, Fy_ParserError_SymbolNotCode, NULL, "%s", instruction->name);
    instruction->instruction_offset = node->code_label;
}

static void Fy_ProcessOpReg16Mem(Fy_Parser *parser, Fy_Instruction_OpReg16Mem *instruction) {
    Fy_AST_eval(instruction->address_ast, parser, &instruction->value);
}

/* Label processing functions */

static void Fy_ProcessLabelOpLabel(Fy_Parser *parser, Fy_Instruction_OpLabel *instruction) {
    instruction->address = Fy_Parser_getCodeOffsetByInstructionIndex(parser, instruction->instruction_offset);
}

/* Custom delete functions */

static void Fy_DeleteLea(Fy_Instruction_OpReg16Mem *instruction) {
    Fy_AST_Delete(instruction->address_ast);
}

/* General parsing functions */

static bool Fy_Parser_expectNewline(Fy_Parser *parser, bool do_error) {
    Fy_ParserState state;
    Fy_Parser_dumpState(parser, &state);

    if (!Fy_Parser_lex(parser, true))
        return true; // Eof = Eol for us
    if (parser->token.type == Fy_TokenType_Newline)
        return true;

    // Load last state if we didn't get a newline
    Fy_Parser_loadState(parser, &state);
    if (do_error)
        Fy_Parser_error(parser, Fy_ParserError_ExpectedNewline, NULL, NULL);

    return false;
}

static bool Fy_Parser_parseArgument(Fy_Parser *parser, Fy_InstructionArg *out) {
    Fy_ParserState backtrack;

    Fy_Parser_dumpState(parser, &backtrack);

    if ((out->as_memory = Fy_Parser_parseMemExpr(parser, &out->type))) {
        ;
    } else if (Fy_Parser_getConst16(parser, &out->as_const)) {
        int16_t as_signed = (int16_t)out->as_const;
        // If in range of 8-bit integers you can mark it as 8-bit
        if (as_signed >= -0x80 && as_signed <= 0xff) {
            out->type = Fy_InstructionArgType_Const8;
        } else {
            out->type = Fy_InstructionArgType_Const16;
        }
    } else {
        if (!Fy_Parser_lex(parser, true))
            return false;

        if (Fy_TokenType_isReg16(parser->token.type)) {
            out->type = Fy_InstructionArgType_Reg16;
            out->as_reg16 = Fy_TokenType_toReg16(parser->token.type);
        } else if (Fy_TokenType_isReg8(parser->token.type)) {
            out->type = Fy_InstructionArgType_Reg8;
            out->as_reg8 = Fy_TokenType_toReg8(parser->token.type);
        } else if (parser->token.type == Fy_TokenType_Symbol) {
            out->type = Fy_InstructionArgType_Label;
            out->as_label = Fy_Token_toLowercaseCStr(&parser->token);
        } else {
            Fy_Parser_loadState(parser, &backtrack);
            return false;
        }
    }

    out->state = backtrack;
    return true;
}

static Fy_Instruction *Fy_Parser_parseByCustomRule(Fy_Parser *parser, const Fy_ParserParseRule *rule,
                                                    uint8_t amount_args, Fy_InstructionArg *arg1, Fy_InstructionArg *arg2) {
    Fy_Instruction *instruction = NULL;

    assert(rule->type == Fy_ParserParseRuleType_Custom);

    if (amount_args == rule->as_custom.amount_params) {
        switch (amount_args) {
        case 0:
            instruction = rule->as_custom.func_no_params(parser);
            break;
        case 1:
            if (Fy_InstructionArgType_is(arg1->type, rule->as_custom.arg1_type))
                instruction = rule->as_custom.func_one_param(parser, arg1);
            break;
        case 2:
            if (Fy_InstructionArgType_is(arg1->type, rule->as_custom.arg1_type) && Fy_InstructionArgType_is(arg2->type, rule->as_custom.arg2_type))
                instruction = rule->as_custom.func_two_params(parser, arg1, arg2);
            break;
        default:
            FY_UNREACHABLE();
        }
    }

    return instruction;
}

static Fy_Instruction *Fy_Parser_parseByBinaryOperatorRule(Fy_Parser *parser, const Fy_ParserParseRule *rule, uint8_t amount_args,
                                                            Fy_InstructionArg *arg1, Fy_InstructionArg *arg2, Fy_ParserState *start_state) {
    Fy_Instruction_BinaryOperator *instruction;
    Fy_BinaryOperatorArgsType args_type;
    size_t amount_matches = 0;

    if (amount_args != 2)
        return NULL;

    // Decide how we handle the instruction arguments

    for (size_t i = 0; i < sizeof(Fy_binaryOperatorRules) / sizeof(Fy_BinaryOperatorRule); ++i) {
        if (Fy_InstructionArgType_is(arg1->type, Fy_binaryOperatorRules[i].arg1_type) && Fy_InstructionArgType_is(arg2->type, Fy_binaryOperatorRules[i].arg2_type)) {
            args_type = Fy_binaryOperatorRules[i].binary_instruction_type;
            ++amount_matches;
        }
    }

    switch (amount_matches) {
    case 0:
        return NULL;
    case 1:
        break;
    default:
        // Got more than 1 match
        Fy_Parser_error(parser, Fy_ParserError_AmbiguousInstructionParameters, start_state, NULL);
    }

    instruction = FY_INSTRUCTION_NEW(Fy_Instruction_BinaryOperator, Fy_instructionTypeBinaryOperator);
    instruction->operator = rule->as_binary.operator_id;
    instruction->type = args_type;
    switch (args_type) {
    case Fy_BinaryOperatorArgsType_Reg16Const:
        instruction->as_reg16const.reg_id = arg1->as_reg16;
        instruction->as_reg16const.value = arg2->as_const;
        break;
    case Fy_BinaryOperatorArgsType_Reg16Reg16:
        instruction->as_reg16reg16.reg_id = arg1->as_reg16;
        instruction->as_reg16reg16.reg2_id = arg2->as_reg16;
        break;
    case Fy_BinaryOperatorArgsType_Reg16Memory16:
        instruction->as_reg16mem16.reg_id = arg1->as_reg16;
        instruction->as_reg16mem16.ast = arg2->as_memory;
        break;
    case Fy_BinaryOperatorArgsType_Reg8Const:
        instruction->as_reg8const.reg_id = arg1->as_reg8;
        instruction->as_reg8const.value = arg2->as_const;
        break;
    case Fy_BinaryOperatorArgsType_Reg8Reg8:
        instruction->as_reg8reg8.reg_id = arg1->as_reg8;
        instruction->as_reg8reg8.reg2_id = arg2->as_reg8;
        break;
    case Fy_BinaryOperatorArgsType_Reg8Memory8:
        instruction->as_reg8mem8.reg_id = arg1->as_reg8;
        instruction->as_reg8mem8.ast = arg2->as_memory;
        break;
    case Fy_BinaryOperatorArgsType_Memory16Const:
        instruction->as_mem16const.ast = arg1->as_memory;
        instruction->as_mem16const.value = arg2->as_const;
        break;
    case Fy_BinaryOperatorArgsType_Memory16Reg16:
        instruction->as_mem16reg16.ast = arg1->as_memory;
        instruction->as_mem16reg16.reg_id = arg2->as_reg16;
        break;
    case Fy_BinaryOperatorArgsType_Memory8Const:
        instruction->as_mem8const.ast = arg1->as_memory;
        instruction->as_mem8const.value = arg2->as_const;
        break;
    case Fy_BinaryOperatorArgsType_Memory8Reg8:
        instruction->as_mem8reg8.ast = arg1->as_memory;
        instruction->as_mem8reg8.reg_id = arg2->as_reg8;
        break;
    default:
        FY_UNREACHABLE();
    }

    return (Fy_Instruction*)instruction;
}


static Fy_Instruction *Fy_Parser_parseByUnaryOperatorRule(Fy_Parser *parser, const Fy_ParserParseRule *rule, uint8_t amount_args,
                                                            Fy_InstructionArg *arg1, Fy_InstructionArg *arg2, Fy_ParserState *start_state) {
    Fy_Instruction_UnaryOperator *instruction;
    Fy_UnaryOperatorArgsType args_type;
    size_t amount_matches = 0;

    (void)arg2;

    if (amount_args != 1)
        return NULL;

    // Decide how we handle the instruction arguments

    for (size_t i = 0; i < sizeof(Fy_unaryOperatorRules) / sizeof(Fy_UnaryOperatorRule); ++i) {
        if (Fy_InstructionArgType_is(arg1->type, Fy_unaryOperatorRules[i].arg_type)) {
            args_type = Fy_unaryOperatorRules[i].unary_instruction_type;
            ++amount_matches;
        }
    }

    switch (amount_matches) {
    case 0:
        return NULL;
    case 1:
        break;
    default:
        // Got more than 1 match
        Fy_Parser_error(parser, Fy_ParserError_AmbiguousInstructionParameters, start_state, NULL);
    }

    instruction = FY_INSTRUCTION_NEW(Fy_Instruction_UnaryOperator, Fy_instructionTypeUnaryOperator);
    instruction->operator = rule->as_unary.operator_id;
    instruction->type = args_type;
    switch (args_type) {
    case Fy_UnaryOperatorArgsType_Reg16:
        instruction->as_reg16 = arg1->as_reg16;
        break;
    case Fy_UnaryOperatorArgsType_Mem16:
        instruction->as_mem16.ast = arg1->as_memory;
        break;
    case Fy_UnaryOperatorArgsType_Reg8:
        instruction->as_reg8 = arg1->as_reg8;
        break;
    case Fy_UnaryOperatorArgsType_Mem8:
        instruction->as_mem8.ast = arg1->as_memory;
        break;
    default:
        FY_UNREACHABLE();
    }

    return (Fy_Instruction*)instruction;
}

static Fy_Instruction *Fy_Parser_parseByJumpRule(Fy_Parser *parser, const Fy_ParserParseRule *rule, uint8_t amount_args,
                                                Fy_InstructionArg *arg1, Fy_InstructionArg *arg2) {
    Fy_Instruction_OpLabel *instruction;

    (void)parser;
    (void)arg2;

    if (amount_args != 1)
        return NULL;

    if (arg1->type != Fy_InstructionArgType_Label)
        return NULL;

    // Create label instruction and add the code label to it
    instruction = FY_INSTRUCTION_NEW(Fy_Instruction_OpLabel, *rule->as_jump.instruction_type);
    instruction->name = arg1->as_label;
    return (Fy_Instruction*)instruction;
}

static Fy_Instruction *Fy_Parser_parseInstruction(Fy_Parser *parser) {
    Fy_ParserState start_backtrack;
    Fy_TokenType start_token;
    uint8_t amount_args;
    Fy_InstructionArg arg1, arg2;
    const Fy_ParserParseRule *rule, *parsedRule;
    Fy_Instruction *instruction = NULL;

    Fy_Parser_dumpState(parser, &start_backtrack);

    // If you can't even lex, don't start parsing
    if (!Fy_Parser_lex(parser, true))
        return NULL;
    start_token = parser->token.type;

    amount_args = 0;
    if (Fy_Parser_parseArgument(parser, &arg1)) {
        ++amount_args;
        if (Fy_Parser_parseArgument(parser, &arg2))
            ++amount_args;
    }
    Fy_Parser_expectNewline(parser, true);

    for (size_t i = 0; i < sizeof(Fy_parserRules) / sizeof(Fy_ParserParseRule*); ++i) {
        rule = Fy_parserRules[i];

        if (start_token == rule->start_token) {
            Fy_Instruction *new_instruction;

            switch (rule->type) {
            case Fy_ParserParseRuleType_Custom:
                new_instruction = Fy_Parser_parseByCustomRule(parser, rule, amount_args, &arg1, &arg2);
                break;
            case Fy_ParserParseRuleType_BinaryOperator:
                new_instruction = Fy_Parser_parseByBinaryOperatorRule(parser, rule, amount_args, &arg1, &arg2, &start_backtrack);
                break;
            case Fy_ParserParseRuleType_UnaryOperator:
                new_instruction = Fy_Parser_parseByUnaryOperatorRule(parser, rule, amount_args, &arg1, &arg2, &start_backtrack);
                break;
            case Fy_ParserParseRuleType_Jump:
                new_instruction = Fy_Parser_parseByJumpRule(parser, rule, amount_args, &arg1, &arg2);
                break;
            default:
                FY_UNREACHABLE();
            }

            if (new_instruction) {
                // If we were able to parse a new instruction from the same information
                if (instruction)
                    Fy_Parser_error(parser, Fy_ParserError_AmbiguousInstructionParameters, &start_backtrack, NULL);
                else // FIXME: Free here
                    instruction = new_instruction;
                // Store the rule we parsed with
                parsedRule = rule;
            }
        }
    }

    if (instruction) {
        instruction->parse_rule = parsedRule;
        instruction->start_state = start_backtrack;
        return instruction;
    }

    // Remove arguments
    switch (amount_args) {
    case 0:
        break;
    case 1:
        Fy_InstructionArg_Destruct(&arg1);
        break;
    case 2:
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

static bool Fy_Parser_parseMacroDef(Fy_Parser *parser) {
    Fy_ParserState backtrack;
    char *name;
    Fy_Token symbol_token;
    Fy_Token *tokens = NULL;
    size_t allocated = 0, idx = 0;
    Fy_Macro macro;

    Fy_Parser_dumpState(parser, &backtrack);

    if (!Fy_Parser_match(parser, Fy_TokenType_Symbol, false))
        return false;
    symbol_token = parser->token;
    if (!Fy_Parser_match(parser, Fy_TokenType_EqualSign, true)) {
        Fy_Parser_loadState(parser, &backtrack);
        return false;
    }

    // Get tokens
    while (!Fy_Parser_expectNewline(parser, false)) {
        if (!Fy_Parser_lex(parser, true))
            Fy_Parser_error(parser, Fy_ParserError_SyntaxError, NULL, NULL);
        if (allocated == 0) {
            tokens = malloc((allocated = 4) * sizeof(Fy_Token));
        } else if (idx == allocated) {
            tokens = realloc(tokens, (allocated += 4) * sizeof(Fy_Token));
        }
        // Store token
        tokens[idx++] = parser->token;
    }

    // Convert macro name token to cstring
    name = Fy_Token_toLowercaseCStr(&symbol_token);

    // Create macro
    macro.tokens = tokens;
    macro.token_amount = idx;

    Fy_Symbolmap_addMacro(&parser->symmap, name, macro);
    return true;
}

/* Returns whether we parsed a label */
static bool Fy_Parser_parseLabel(Fy_Parser *parser) {
    Fy_Token label_token;
    char *label_string;

    if (!Fy_Parser_match(parser, Fy_TokenType_Symbol, true))
        return false;
    label_token = parser->token;

    if (!Fy_Parser_match(parser, Fy_TokenType_Colon, true))
        Fy_Parser_error(parser, Fy_ParserError_SyntaxError, NULL, NULL);

    // Advance newline if there is
    Fy_Parser_expectNewline(parser, false);

    label_string = Fy_Token_toLowercaseCStr(&label_token);
    Fy_Symbolmap_addLabelDecl(&parser->symmap, label_string, parser->amount_used);

    return true;
}

static bool Fy_Parser_parseProc(Fy_Parser *parser) {
    char *label_string;
    uint16_t amount_prev_instructions = parser->amount_used;

    if (!Fy_Parser_match(parser, Fy_TokenType_Proc, true))
        return false;

    if (!Fy_Parser_match(parser, Fy_TokenType_Symbol, true))
        Fy_Parser_error(parser, Fy_ParserError_SyntaxError, NULL, NULL);

    label_string = Fy_Token_toLowercaseCStr(&parser->token);

    Fy_Parser_expectNewline(parser, true);

    for (;;) {
        if (Fy_Parser_match(parser, Fy_TokenType_Endp, true)) {
            char *endp_label_string;
            if (!Fy_Parser_match(parser, Fy_TokenType_Symbol, true))
                Fy_Parser_error(parser, Fy_ParserError_SyntaxError, NULL, NULL);
            endp_label_string = Fy_Token_toLowercaseCStr(&parser->token);
            if (strcmp(label_string, endp_label_string) != 0)
                Fy_Parser_error(parser, Fy_ParserError_UnexpectedSymbol, NULL, "Expected '%s'", label_string);
            free(endp_label_string);
            Fy_Parser_expectNewline(parser, true);
            break;
        }
        if (!Fy_Parser_parseLine(parser))
            Fy_Parser_error(parser, Fy_ParserError_UnexpectedToken, NULL, NULL);
    }

    Fy_Symbolmap_addLabelDecl(&parser->symmap, label_string, amount_prev_instructions);

    return true;
}

/* Parse label, procedure or instruction */
static bool Fy_Parser_parseLine(Fy_Parser *parser) {
    Fy_Instruction *instruction;

    Fy_Parser_expectNewline(parser, false);

    if (Fy_Parser_parseMacroDef(parser))
        return true;

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

static bool Fy_Parser_parseDupByte(Fy_Parser *parser) {
    Fy_ParserState backtrack;
    uint16_t amount_duplicates;
    uint16_t amount_values = 0, allocated_values = 0;
    uint8_t *values;

    Fy_Parser_dumpState(parser, &backtrack);

    if (!Fy_Parser_getConst16(parser, &amount_duplicates))
        return false;

    if (!Fy_Parser_match(parser, Fy_TokenType_Dup, true)) {
        Fy_Parser_loadState(parser, &backtrack);
        return false;
    }

    if (!Fy_Parser_match(parser, Fy_TokenType_LeftParen, true))
        Fy_Parser_error(parser, Fy_ParserError_ExpectedDifferentToken, NULL, "'('");

    do {
        uint8_t value;
        if (!Fy_Parser_getConst8(parser, &value))
            Fy_Parser_error(parser, Fy_ParserError_ExpectedDifferentToken, NULL, "Const");
        if (allocated_values == 0)
            values = malloc((allocated_values = 4) * sizeof(uint8_t));
        else if (amount_values == allocated_values)
            values = realloc(values, (allocated_values += 4) * sizeof(uint8_t));
        values[amount_values++] = value;
    } while (Fy_Parser_match(parser, Fy_TokenType_Comma, true));

    if (!Fy_Parser_match(parser, Fy_TokenType_RightParen, true))
        Fy_Parser_error(parser, Fy_ParserError_ExpectedDifferentToken, NULL, "')'");

    // TODO: This can probably be optimized
    for (uint16_t i = 0; i < amount_duplicates; ++i) {
        for (uint16_t j = 0; j < amount_values; ++j) {
            Fy_Parser_addData8(parser, values[j]);
        }
    }

    free(values);

    return true;
}

static bool Fy_Parser_parseDupWord(Fy_Parser *parser) {
    Fy_ParserState backtrack;
    uint16_t amount_duplicates;
    uint16_t amount_values = 0, allocated_values = 0;
    uint16_t *values;

    Fy_Parser_dumpState(parser, &backtrack);

    if (!Fy_Parser_getConst16(parser, &amount_duplicates))
        return false;

    if (!Fy_Parser_match(parser, Fy_TokenType_Dup, true)) {
        Fy_Parser_loadState(parser, &backtrack);
        return false;
    }

    if (!Fy_Parser_match(parser, Fy_TokenType_LeftParen, true))
        Fy_Parser_error(parser, Fy_ParserError_ExpectedDifferentToken, NULL, "'('");

    do {
        uint16_t value;
        if (!Fy_Parser_getConst16(parser, &value))
            Fy_Parser_error(parser, Fy_ParserError_ExpectedDifferentToken, NULL, "Const");
        if (allocated_values == 0)
            values = malloc((allocated_values = 4) * sizeof(uint16_t));
        else if (amount_values == allocated_values)
            values = realloc(values, (allocated_values += 4) * sizeof(uint16_t));
        values[amount_values++] = value;
    } while (Fy_Parser_match(parser, Fy_TokenType_Comma, true));

    if (!Fy_Parser_match(parser, Fy_TokenType_RightParen, true))
        Fy_Parser_error(parser, Fy_ParserError_ExpectedDifferentToken, NULL, "')'");

    // TODO: This can probably be optimized
    for (uint16_t i = 0; i < amount_duplicates; ++i) {
        for (uint16_t j = 0; j < amount_values; ++j) {
            Fy_Parser_addData16(parser, values[j]);
        }
    }

    free(values);

    return true;
}

static bool Fy_Parser_parseByteString(Fy_Parser *parser) {
    size_t length;
    if (!Fy_Parser_match(parser, Fy_TokenType_String, true))
        return false;
    length = parser->token.length;
    for (size_t i = 0; i < length; ++i)
        Fy_Parser_addData8(parser, parser->token.start[i]);
    return true;
}

static bool Fy_Parser_parseWordString(Fy_Parser *parser) {
    size_t length;
    if (!Fy_Parser_match(parser, Fy_TokenType_String, true))
        return false;
    length = parser->token.length;
    for (size_t i = 0; i < length; ++i)
        Fy_Parser_addData8(parser, parser->token.start[i]);
    // If we don't align as bytes pad with 0
    if (length % 2 != 0)
        Fy_Parser_addData8(parser, 0);
    return true;
}

static void Fy_Parser_parseSetVariable(Fy_Parser *parser) {
    char *variable_name;
    uint16_t variable_offset;

    if (Fy_Parser_match(parser, Fy_TokenType_Symbol, true)) {
        variable_name = Fy_Token_toLowercaseCStr(&parser->token);
        // If there is a variable defined with that name
        if (Fy_Symbolmap_getEntry(&parser->symmap, variable_name))
            Fy_Parser_error(parser, Fy_ParserError_SymbolAlreadyDefined, NULL, "%s", variable_name);
    } else {
        variable_name = NULL;
    }

    variable_offset = parser->data_size;

    if (Fy_Parser_match(parser, Fy_TokenType_Eb, true)) {
        do {
            uint8_t value;
            if (Fy_Parser_parseByteString(parser)) {
                ;
            } else if (Fy_Parser_parseDupByte(parser)) {
                ;
            } else if (Fy_Parser_getConst8(parser, &value)) {
                Fy_Parser_addData8(parser, *(int8_t*)&value);
            } else {
                Fy_Parser_error(parser, Fy_ParserError_ExpectedDifferentToken, NULL, "Const");
            }
        } while (Fy_Parser_match(parser, Fy_TokenType_Comma, true));
    } else if (Fy_Parser_match(parser, Fy_TokenType_Ew, true)) {
        do {
            uint16_t value;
            if (Fy_Parser_parseWordString(parser)) {
                ;
            } else if (Fy_Parser_parseDupWord(parser)) {
                ;
            } else if (Fy_Parser_getConst16(parser, &value)) {
                Fy_Parser_addData16(parser, *(int16_t*)&value);
            } else {
                Fy_Parser_error(parser, Fy_ParserError_ExpectedDifferentToken, NULL, "Const");
            }
        } while (Fy_Parser_match(parser, Fy_TokenType_Comma, true));
    } else {
        Fy_Parser_error(parser, Fy_ParserError_ExpectedDifferentToken, NULL, "EB or EW");
        FY_UNREACHABLE();
    }

    if (variable_name)
        Fy_Symbolmap_addVariable(&parser->symmap, variable_name, variable_offset);

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
    if (Fy_Parser_lex(parser, true)) {
        Fy_Parser_loadState(parser, &backtrack);
        Fy_Parser_error(parser, Fy_ParserError_SyntaxError, NULL, NULL);
    }
}

/* Step 2: processing parsed instructions */
static void Fy_Parser_processInstructions(Fy_Parser *parser) {
    for (size_t i = 0; i < parser->amount_used; ++i) {
        Fy_Instruction *instruction = parser->instructions[i];
        Fy_Parser_loadState(parser, &instruction->start_state);
        switch (instruction->parse_rule->type) {
        case Fy_ParserParseRuleType_Custom:
            if (instruction->parse_rule->as_custom.process_func)
                instruction->parse_rule->as_custom.process_func(parser, instruction);
            break;
        case Fy_ParserParseRuleType_BinaryOperator: {
            Fy_Instruction_BinaryOperator *binary_instruction = (Fy_Instruction_BinaryOperator*)instruction;
            // Evaluate memory AST in instructions that reference memory
            switch (binary_instruction->type) {
            case Fy_BinaryOperatorArgsType_Reg16Const:
            case Fy_BinaryOperatorArgsType_Reg16Reg16:
            case Fy_BinaryOperatorArgsType_Reg8Const:
            case Fy_BinaryOperatorArgsType_Reg8Reg8:
                break;
            case Fy_BinaryOperatorArgsType_Memory16Const:
                Fy_AST_eval(binary_instruction->as_mem16const.ast, parser, &binary_instruction->as_mem16const.address);
                break;
            case Fy_BinaryOperatorArgsType_Reg16Memory16:
                Fy_AST_eval(binary_instruction->as_reg16mem16.ast, parser, &binary_instruction->as_reg16mem16.address);
                break;
            case Fy_BinaryOperatorArgsType_Reg8Memory8:
                Fy_AST_eval(binary_instruction->as_reg8mem8.ast, parser, &binary_instruction->as_reg8mem8.address);
                break;
            case Fy_BinaryOperatorArgsType_Memory16Reg16:
                Fy_AST_eval(binary_instruction->as_mem16reg16.ast, parser, &binary_instruction->as_mem16reg16.address);
                break;
            case Fy_BinaryOperatorArgsType_Memory8Const:
                Fy_AST_eval(binary_instruction->as_mem8const.ast, parser, &binary_instruction->as_mem8const.address);
                break;
            case Fy_BinaryOperatorArgsType_Memory8Reg8:
                Fy_AST_eval(binary_instruction->as_mem8reg8.ast, parser, &binary_instruction->as_mem8reg8.address);
                break;
            default:
                FY_UNREACHABLE();
            }
            break;
        }
        case Fy_ParserParseRuleType_UnaryOperator: {
            Fy_Instruction_UnaryOperator *unary_instruction = (Fy_Instruction_UnaryOperator*)instruction;
            // Evaluate memory AST in instructions that reference memory
            switch (unary_instruction->type) {
            case Fy_UnaryOperatorArgsType_Reg16:
            case Fy_UnaryOperatorArgsType_Reg8:
                break;
            case Fy_UnaryOperatorArgsType_Mem16:
                Fy_AST_eval(unary_instruction->as_mem16.ast, parser, &unary_instruction->as_mem16.address);
                break;
            case Fy_UnaryOperatorArgsType_Mem8:
                Fy_AST_eval(unary_instruction->as_mem8.ast, parser, &unary_instruction->as_mem8.address);
                break;
            default:
                FY_UNREACHABLE();
            }
            break;
        }
        case Fy_ParserParseRuleType_Jump:
            Fy_ProcessOpLabel(parser, (Fy_Instruction_OpLabel*)instruction);
            break;
        default:
            FY_UNREACHABLE();
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
        switch (instruction->parse_rule->type) {
        case Fy_ParserParseRuleType_Custom:
            if (instruction->parse_rule->as_custom.process_label_func)
                instruction->parse_rule->as_custom.process_label_func(instruction, parser);
            break;
        case Fy_ParserParseRuleType_BinaryOperator:
        case Fy_ParserParseRuleType_UnaryOperator:
            break;
        case Fy_ParserParseRuleType_Jump:
            Fy_ProcessLabelOpLabel(parser, (Fy_Instruction_OpLabel*)instruction);
            break;
        default:
            FY_UNREACHABLE();
        }
    }
}

void Fy_Parser_parseAll(Fy_Parser *parser) {
    // Advance newline if there is one
    Fy_Parser_expectNewline(parser, false);
    if (Fy_Parser_match(parser, Fy_TokenType_Data, true)) {
        Fy_Parser_expectNewline(parser, true);
        while (!Fy_Parser_match(parser, Fy_TokenType_Code, true)) {
            if (!Fy_Parser_parseMacroDef(parser))
                Fy_Parser_parseSetVariable(parser);
        }
    } else if (!Fy_Parser_match(parser, Fy_TokenType_Code, true)) {
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
void Fy_Parser_generateBytecode(Fy_Parser *parser, Fy_Generator *generator) {
    // Add size of data, code and stack to header
    Fy_Generator_addWord(generator, parser->data_size);
    Fy_Generator_addWord(generator, parser->code_size);
    Fy_Generator_addWord(generator, 0x100); // Stack size

    // TODO: Optimize this
    // Add all of the data bytes
    for (size_t i = 0; i < parser->data_size; ++i)
        Fy_Generator_addByte(generator, parser->data_part[i]);

    // Add all of the instructions
    for (size_t i = 0; i < parser->amount_used; ++i) {
        Fy_Instruction *instruction = parser->instructions[i];
        Fy_Generator_addInstruction(generator, instruction);
    }
}

void Fy_Parser_generateToFile(Fy_Parser *parser, char *filename, char *shebang_path) {
    FILE *file = fopen(filename, "w+");
    char *shebang_start = "#!";
    char *shebang_end = " -r \n";
    Fy_Generator generator;

    if (!file)
        Fy_Parser_error(parser, Fy_ParserError_CannotOpenFileForWrite, NULL, "%s", filename);

    Fy_Generator_Init(&generator);

    // If there's a shebang that needs to be added, add it
    if (shebang_path) {
        Fy_Generator_addString(&generator, shebang_start);
        Fy_Generator_addString(&generator, shebang_path);
        Fy_Generator_addString(&generator, shebang_end);
    }

    // Add FY to signal start of binary file
    Fy_Generator_addByte(&generator, 'F');
    Fy_Generator_addByte(&generator, 'Y');

    Fy_Parser_generateBytecode(parser, &generator);
    fwrite(generator.output, 1, generator.idx, file);
    Fy_Generator_Deallocate(&generator);

    fclose(file);
}
