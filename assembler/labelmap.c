#include "fy.h"

static Fy_BucketNode *Fy_BucketNode_New(char *name, uint16_t address) {
    Fy_BucketNode *bucket = malloc(sizeof(Fy_BucketNode));
    bucket->name = name;
    bucket->rel_address = address;
    return bucket;
}

void Fy_Labelmap_Init(Fy_Labelmap *out) {
    for (size_t i = 0; i < FY_LABELMAP_AMOUNT_BUCKETS; ++i)
        out->buckets[i] = NULL;
}

static size_t Fy_HashLabelmapKey(char *key) {
    size_t value = 0;

    for (char *cur_idx = key; *cur_idx; ++cur_idx) {
        value *= 256;
        value += *cur_idx;
        value %= FY_LABELMAP_AMOUNT_BUCKETS;
    }

    return value;
}

bool Fy_Labelmap_addEntry(Fy_Labelmap *map, char *name, uint16_t address) {
    size_t idx = Fy_HashLabelmapKey(name);
    Fy_BucketNode *base_node = map->buckets[idx];
    if (!base_node) {
        map->buckets[idx] = Fy_BucketNode_New(name, address);
    } else {
        Fy_BucketNode *prev;
        do {
            // If found a match, panic
            if (strcmp(base_node->name, name) == 0)
                return false;
            prev = base_node;
            base_node = prev->next;
        } while (base_node);
        prev->next = Fy_BucketNode_New(name, address);
    }
    return true;
}

/* Returns whether we found the entry, and if we did, we put it in address_out */
bool Fy_Labelmap_getEntry(Fy_Labelmap *map, char *name, uint16_t *address_out) {
    size_t idx = Fy_HashLabelmapKey(name);
    Fy_BucketNode *node = map->buckets[idx];
    while (node) {
        if (strcmp(node->name, name) == 0) {
            *address_out = node->rel_address;
            return true;
        }
    }
    // Not found
    return false;
}
