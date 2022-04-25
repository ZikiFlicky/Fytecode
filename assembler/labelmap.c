#include "fy.h"

static Fy_BucketNode *Fy_BucketNode_New(char *name, Fy_MapEntryType type) {
    Fy_BucketNode *bucket = malloc(sizeof(Fy_BucketNode));
    bucket->name = name;
    bucket->type = type;
    bucket->next = NULL;
    return bucket;
}

static void Fy_BucketNode_Delete(Fy_BucketNode *node) {
    free(node->name);
    free(node);
}

void Fy_Labelmap_Init(Fy_Labelmap *out) {
    for (size_t i = 0; i < FY_LABELMAP_AMOUNT_BUCKETS; ++i)
        out->buckets[i] = NULL;
}

void Fy_Labelmap_Destruct(Fy_Labelmap *map) {
    for (size_t i = 0; i < FY_LABELMAP_AMOUNT_BUCKETS; ++i) {
        Fy_BucketNode *node = map->buckets[i];
        while (node) {
            Fy_BucketNode *next = node->next;
            Fy_BucketNode_Delete(node);
            node = next;
        }
    }
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

static Fy_BucketNode *Fy_Labelmap_addEntry(Fy_Labelmap *map, char *name, Fy_MapEntryType type) {
    size_t idx = Fy_HashLabelmapKey(name);
    Fy_BucketNode *base_node = map->buckets[idx];
    Fy_BucketNode *new_node;
    Fy_BucketNode **insert_ptr;

    if (!base_node) {
        insert_ptr = &map->buckets[idx];
    } else {
        Fy_BucketNode *prev;
        do {
            // If found a match, panic
            if (strcmp(base_node->name, name) == 0)
                return NULL;
            prev = base_node;
            base_node = prev->next;
        } while (base_node);
        insert_ptr = &prev->next;
    }
    new_node = Fy_BucketNode_New(name, type);
    *insert_ptr = new_node;
    return new_node;
}

bool Fy_Labelmap_addMemLabel(Fy_Labelmap *map, char *name, uint16_t address) {
    Fy_BucketNode *node = Fy_Labelmap_addEntry(map, name, Fy_MapEntryType_CodeLabel);
    if (!node)
        return false;
    node->code_offset = address;
    return true;
}

bool Fy_Labelmap_addVariable(Fy_Labelmap *map, char *name, uint16_t offset) {
    Fy_BucketNode *node = Fy_Labelmap_addEntry(map, name, Fy_MapEntryType_Variable);
    if (!node)
        return false;
    node->data_offset = offset;
    return true;
}

/* Returns whether we found the entry, and if we did, we put it in address_out */
Fy_BucketNode *Fy_Labelmap_getEntry(Fy_Labelmap *map, char *name) {
    size_t idx = Fy_HashLabelmapKey(name);
    Fy_BucketNode *node = map->buckets[idx];
    while (node) {
        if (strcmp(node->name, name) == 0)
            return node;
        node = node->next;
    }
    // Not found
    return NULL;
}
