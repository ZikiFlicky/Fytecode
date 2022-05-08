#ifndef FY_FY_H
#define FY_FY_H

/* Fytecode forward file */

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <assert.h>
#include <string.h>
#include <stdarg.h>

#include "../assembler/token.h"
#include "../assembler/lexer.h"
#include "../assembler/ast.h"
#include "../assembler/parser.h"
#include "../assembler/generator.h"
#include "../assembler/instruction.h"
#include "../assembler/symbolmap.h"

#include "../vm/vm.h"
#include "../vm/registers.h"
#include "../vm/interrupts.h"

#define FY_UNREACHABLE() assert(0)

#endif /* FY_FY_H */
