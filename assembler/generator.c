#include "fy.h"

void Fy_Generator_Init(Fy_Generator *out) {
    out->allocated = 0;
    out->idx = 0;
    out->output = NULL;
}

void Fy_Generator_Deallocate(Fy_Generator *generator) {
    if (generator->idx > 0)
        free(generator->output);
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

void Fy_Generator_addByte(Fy_Generator *generator, uint8_t b) {
    Fy_Generator_makeSpace(generator, 1);
    generator->output[generator->idx++] = b;
}

void Fy_Generator_addWord(Fy_Generator *generator, uint16_t w) {
    Fy_Generator_makeSpace(generator, 2);
    // Little endianness
    generator->output[generator->idx] = w & 0xff;
    generator->output[generator->idx + 1] = w >> 8;
    generator->idx += 2;
}

void Fy_Generator_addString(Fy_Generator *generator, char *string) {
    for (size_t i = 0; string[i] != '\0'; ++i)
        Fy_Generator_addByte(generator, (uint8_t)string[i]);
}

void Fy_Generator_addMemory(Fy_Generator *generator, Fy_InlineValue *mem) {
    uint8_t mapping;
    Fy_InlineValue_getMapping(mem, &mapping);
    // Add mapping
    Fy_Generator_addByte(generator, mapping);
    if (mapping & FY_INLINEVAL_MAPPING_HASVAR)
        Fy_Generator_addWord(generator, mem->variable_offset);
    if (mapping & FY_INLINEVAL_MAPPING_HASBP)
        Fy_Generator_addWord(generator, mem->times_bp);
    if (mapping & FY_INLINEVAL_MAPPING_HASBX)
        Fy_Generator_addWord(generator, mem->times_bx);
    if (mapping & FY_INLINEVAL_MAPPING_HASNUM)
        Fy_Generator_addWord(generator, mem->numeric);
}

void Fy_Generator_addInstruction(Fy_Generator *generator, Fy_Instruction *instruction) {
    Fy_InstructionWriteFunc func;
    // Add the instruction opcode
    Fy_Generator_addByte(generator, instruction->type->opcode);
    func = instruction->type->write_func;
    // Write only if function is defined
    if (func)
        func(generator, instruction);
}
