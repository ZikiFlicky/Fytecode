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
        Fy_BytecodeFileStream bc;
        Fy_VM vm;
        int exit_code;

        if (argc != 3) {
            printf("Expected one argument after '-r' switch\n");
            return 1;
        }

        if (!Fy_OpenBytecodeFile(argv[2], &bc)) {
            printf("Couldn't load binary file '%s' for read\n", argv[2]);
            return 1;
        }

        Fy_VM_Init(&bc, &vm);
        exit_code = Fy_VM_runAll(&vm);
        Fy_VM_Destruct(&vm);

        Fy_BytecodeFileStream_Destruct(&bc);
        return exit_code;
    } else if (strcmp(argv[1], "-h")) {
        Fy_PrintHelp();
    } else {
        Fy_PrintHelp();
        return 1;
    }

    return 0;
}
