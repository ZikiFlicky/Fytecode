#ifndef FY_GENERATOR_H
#define FY_GENERATOR_H

#include <inttypes.h>
#include <stddef.h>

typedef struct Fy_ParserInstruction Fy_ParserInstruction;

typedef struct Fy_Generator {
    size_t idx, allocated;
    uint8_t *output;
} Fy_Generator;

void Fy_Generator_Init(Fy_Generator *out);

void Fy_Generator_addInstruction(Fy_Generator *generator, Fy_ParserInstruction *instuction);

#endif /* FY_GENERATOR_H */
