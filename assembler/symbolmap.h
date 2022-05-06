#ifndef FY_SYMBOLMAP_H
#define FY_SYMBOLMAP_H

#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>

#define FY_SYMBOLMAP_AMOUNT_BUCKETS 8

typedef struct Fy_Macro Fy_Macro;
typedef struct Fy_MacroEvalInstance Fy_MacroEvalInstance;
typedef struct Fy_Symbolmap Fy_Symbolmap;
typedef struct Fy_BucketNode Fy_BucketNode;
typedef enum Fy_MapEntryType Fy_MapEntryType;

struct Fy_Macro {
    Fy_Token *tokens;
    size_t token_amount;
};

struct Fy_MacroEvalInstance {
    Fy_MacroEvalInstance *parent;
    Fy_Macro *macro;
    size_t macro_idx;
};

enum Fy_MapEntryType {
    Fy_MapEntryType_Variable = 1,
    Fy_MapEntryType_Label,
    Fy_MapEntryType_Macro
};

struct Fy_BucketNode {
    Fy_BucketNode *next;
    Fy_MapEntryType type;
    char *name;
    union {
        Fy_Macro macro;
        uint16_t data_offset;
        size_t code_label;
    };
};

struct Fy_Symbolmap {
    Fy_BucketNode *buckets[8];
};

void Fy_Symbolmap_Init(Fy_Symbolmap *out);
void Fy_Symbolmap_Destruct(Fy_Symbolmap *map);
bool Fy_Symbolmap_addLabelDecl(Fy_Symbolmap *map, char *name, size_t amount_prev_instructions);
bool Fy_Symbolmap_addVariable(Fy_Symbolmap *map, char *name, uint16_t offset);
bool Fy_Symbolmap_addMacro(Fy_Symbolmap *map, char *name, Fy_Macro macro);
Fy_BucketNode *Fy_Symbolmap_getEntry(Fy_Symbolmap *map, char *name);
Fy_Macro *Fy_Symbolmap_getMacro(Fy_Symbolmap *map, char *name);

#endif /* FY_SYMBOLMAP_H */
