#ifndef FY_GENERATOR_H
#define FY_GENERATOR_H

#include <inttypes.h>
#include <stddef.h>

typedef struct Fy_Instruction Fy_Instruction;

typedef struct Fy_Generator {
    size_t idx, allocated;
    uint8_t *output;
} Fy_Generator;

void Fy_Generator_Init(Fy_Generator *out);
void Fy_Generator_Deallocate(Fy_Generator *generator);
void Fy_Generator_addByte(Fy_Generator *generator, uint8_t b);
void Fy_Generator_addWord(Fy_Generator *generator, uint16_t w);
void Fy_Generator_addInstruction(Fy_Generator *generator, Fy_Instruction *instuction);

#endif /* FY_GENERATOR_H */
