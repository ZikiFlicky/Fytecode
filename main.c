#include "fy.h"

/*
 * Reads all of the file into memory and returns the pointer to that memory.
 * Returns a NULL on failure.
 */
char *Fy_LoadFile(char *name) {
    FILE *file;
    size_t length;
    char *stream;

    file = fopen(name, "r");
    if (!file)
        return NULL;

    fseek(file, 0, SEEK_END);
    length = ftell(file);
    fseek(file, 0, SEEK_SET);

    stream = malloc((length + 1) * sizeof(char));
    fread(stream, sizeof(char), length, file);
    fclose(file);

    return stream;
}

typedef struct Fy_VM {
    uint8_t ax[2];
    uint8_t bx[2];
} Fy_VM;

int main(int argc, char **argv) {
    char *stream;
    Fy_Lexer lexer;
    Fy_Parser parser;
    Fy_Generator gen;

    if (argc != 2) {
        printf("Expected 1 argument\n");
        return 1;
    }

    stream = Fy_LoadFile(argv[1]);
    if (!stream) {
        printf("Failed in opening %s\n", argv[1]);
        return 1;
    }

    Fy_Lexer_Init(&lexer, stream);
    Fy_Parser_Init(&lexer, &parser);
    Fy_Parser_parseAll(&parser);

    Fy_Parser_generateBytecode(&parser, &gen);
    // for (size_t i = 0; i < gen.idx; ++i) {
    //     switch (gen.output[i]) {
    //     case 
    //     }
    // }
    // Fy_Parser_logParsed(&parser);
    // Fy_Parser_generateToFile(&parser, "output");

    // printf("%zu\n", parser.amount_used);

    // while (Fy_Lexer_lex(&lexer))
    //     printf("%d\n", lexer.token.type);

    free(stream);

    return 0;
}
