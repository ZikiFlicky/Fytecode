#ifndef FY_LABELMAP_H
#define FY_LABELMAP_H

#include <inttypes.h>
#include <stdbool.h>

#define FY_LABELMAP_AMOUNT_BUCKETS 8

typedef struct Fy_Labelmap Fy_Labelmap;
typedef struct Fy_BucketNode Fy_BucketNode;

struct Fy_BucketNode {
    Fy_BucketNode *next;
    char *name;
    uint16_t rel_address;
};

struct Fy_Labelmap {
    Fy_BucketNode *buckets[8];
};

void Fy_Labelmap_Init(Fy_Labelmap *out);
bool Fy_Labelmap_addEntry(Fy_Labelmap *map, char *name, uint16_t address);
bool Fy_Labelmap_getEntry(Fy_Labelmap *map, char *name, uint16_t *address_out);

#endif /* FY_LABELMAP_H */
