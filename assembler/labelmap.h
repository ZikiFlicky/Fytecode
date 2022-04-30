#ifndef FY_LABELMAP_H
#define FY_LABELMAP_H

#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>

#define FY_LABELMAP_AMOUNT_BUCKETS 8

typedef struct Fy_Labelmap Fy_Labelmap;
typedef struct Fy_BucketNode Fy_BucketNode;
typedef enum Fy_MapEntryType Fy_MapEntryType;

enum Fy_MapEntryType {
    Fy_MapEntryType_Variable = 1,
    Fy_MapEntryType_CodeLabel
};

struct Fy_BucketNode {
    Fy_BucketNode *next;
    Fy_MapEntryType type;
    char *name;
    union {
        uint16_t data_offset;
        size_t code_label;
    };
};

struct Fy_Labelmap {
    Fy_BucketNode *buckets[8];
};

void Fy_Labelmap_Init(Fy_Labelmap *out);
void Fy_Labelmap_Destruct(Fy_Labelmap *map);
bool Fy_Labelmap_addMemLabelDecl(Fy_Labelmap *map, char *name, size_t amount_prev_instructions);
bool Fy_Labelmap_addVariable(Fy_Labelmap *map, char *name, uint16_t offset);
Fy_BucketNode *Fy_Labelmap_getEntry(Fy_Labelmap *map, char *name);

#endif /* FY_LABELMAP_H */
