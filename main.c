#include "fy.h"

/*
 * Reads all of the file into memory and returns the pointer to that memory.
 * Returns a NULL on failure.
 */
static char *Fy_LoadTextFile(char *name) {
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
    stream[length] = '\0';

    return stream;
}

/*
 * Reads all of the binary file into `stream_out` and puts size in out_size.
 * Returns false on failure.
 */
bool Fy_LoadBinaryFile(char *name, uint8_t **stream_out, uint16_t *out_size) {
    FILE *file;
    uint16_t length;
    uint8_t *stream;

    file = fopen(name, "r");
    if (!file)
        return false;

    fseek(file, 0, SEEK_END);
    length = ftell(file);
    fseek(file, 0, SEEK_SET);

    stream = malloc(length * sizeof(uint8_t));
    fread(stream, sizeof(uint8_t), length, file);
    fclose(file);

    *stream_out = stream;
    *out_size = length;

    return true;
}

static void Fy_PrintHelp(void) {
    puts("Welcome to the Fytecode engine!");
    puts("usage: fy -h | -c source output | -r file [--warn-undefined]");
    puts("  -c source output: assembles file into bytecode");
    puts("  -r file:          runs bytecode on virtual machine");
    puts("  -h:               shows this help message");
}

int main(int argc, char **argv) {
    if (argc < 2) {
        Fy_PrintHelp();
        return 1;
    }

    if (strcmp(argv[1], "-c") == 0) {
        char *stream;
        Fy_Lexer lexer;
        Fy_Parser parser;

        if (argc != 4) {
            printf("Expected two arguments after '-c' switch\n");
            return 1;
        }

        stream = Fy_LoadTextFile(argv[2]);
        if (!stream) {
            printf("Couldn't open file '%s' for read\n", argv[2]);
            return 1;
    }

        Fy_Lexer_Init(stream, &lexer);
        Fy_Parser_Init(&lexer, &parser);
        Fy_Parser_parseAll(&parser);

        Fy_Parser_generateToFile(&parser, argv[3]);

        Fy_Parser_Destruct(&parser);

        free(stream);
    } else if (strcmp(argv[1], "-r") == 0) {
        uint8_t *stream;
        uint16_t length;
        Fy_VM vm;

        if (argc != 3) {
            printf("Expected one argument after '-r' switch\n");
            return 1;
        }

        if (!Fy_LoadBinaryFile(argv[2], &stream, &length)) {
            printf("Couldn't load binary file '%s' for read\n", argv[2]);
            return 1;
        }

        Fy_VM_Init(stream, length, 0x100, &vm);
        Fy_VM_runAll(&vm);

        free(stream);
    } else if (strcmp(argv[1], "-h")) {
        Fy_PrintHelp();
    } else {
        Fy_PrintHelp();
        return 1;
    }

    return 0;
}
