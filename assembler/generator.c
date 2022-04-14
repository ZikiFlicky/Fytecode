#include "fy.h"

void Fy_Generator_Init(Fy_Generator *out) {
    out->allocated = 0;
    out->idx = 0;
    out->output = NULL;
}

/* Make enough space for a string if needed, or do nothing */
void Fy_Generator_makeSpace(Fy_Generator *generator, size_t length) {
    if (generator->allocated == 0) {
        generator->allocated = length;
        generator->output = malloc(generator->allocated * sizeof(uint8_t));
    } else if (length > generator->allocated - generator->idx) {
        generator->allocated = generator->idx + length;
        generator->output = realloc(generator->output, generator->allocated * sizeof(uint8_t));
    }
}

// void Fy_Generator_addCString(Fy_Generator *generator, uint8_t *cstr) {
//     size_t len = strlen((char*)cstr); // FIXME: Should this be strlen? we need to work with uint8_t
//     Fy_Generator_makeSpace(generator, len);
//     memcpy(&generator->output[generator->idx], cstr, len * sizeof(uint8_t));
//     generator->idx += len;
// }

void Fy_Generator_addByte(Fy_Generator *generator, uint8_t b) {
    Fy_Generator_makeSpace(generator, 1);
    printf("%zu: %x\n", generator->idx, b);
    generator->output[generator->idx++] = b;
}

void Fy_Generator_addConst16(Fy_Generator *generator, uint16_t w) {
    Fy_Generator_makeSpace(generator, 2);
    // Little endianness
    printf("%zu: %x\n", generator->idx, w);
    generator->output[generator->idx] = w & 0xff;
    generator->output[generator->idx + 1] = w >> 8;
    generator->idx += 2;
}

void Fy_Generator_addInstruction(Fy_Generator *generator, Fy_Instruction *instruction) {
    // Add the instruction opcode
    Fy_Generator_addByte(generator, instruction->type->opcode);
    instruction->type->write_func(generator, instruction);
}
