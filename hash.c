#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

typedef struct {
    const char *key;
    int value;
    bool used;
} Entry;

typedef struct {
    Entry *slots;
    size_t capacity;
    size_t size;
} HashTable;

typedef struct {
    bool found;
    const Entry *slot;
    const char *error;
} HashResp;

static Entry slots[1024] = {0};
static HashTable ht = {slots, 1024, 0};

const char *ht_insert(const char *str, int value) {
    if (str == NULL)
        return "Error: String is NULL";
    size_t index = ht_hash_index(str);
    size_t counter = 0;

    while (true) {
    if (counter == ht.capacity) 
        return "Error: Hash Table is full. Can not add any more Labels.";
    if (ht.slots[index].used == true) {
        if (strcmp(str, ht.slots[index].key) == 0)
            return "Error: Duplicate Key detected.";
        index = (index + 1) % ht.capacity;
        counter++;
        continue;
    };

    char *copy = strdup(str);
    if (!copy) 
        return "Error: Out of memory";

    ht.slots[index].key = copy;
    ht.slots[index].value = value;
    ht.slots[index].used = true;
    ht.size = ht.size + 1;
    break;
    }

    return NULL;
}

static size_t ht_hash_index(const char *str) {
    uint32_t  h32 = UINT32_C(2166136261);
    uint32_t prime = UINT32_C(16777619);
    size_t str_len = strlen(str);
    for (size_t i = 0; i < str_len; i++) {
        unsigned char uch = (unsigned char) str[i];
        h32 ^= uch;
        h32 = h32 * prime;
    }

    return h32 % ht.capacity;
}

HashResp ht_get(const char *str) {
    HashResp hresp = {.found = false, .slot = NULL, .error = NULL};
    if (str == NULL) {
        hresp.error = "Error: String is NULL";
        return hresp;
    }

    size_t index = ht_hash_index(str);

    size_t counter = 0;
    while (true) {
        if (counter == ht.capacity) {
            break;
        }
        if (ht.slots[index].used == false) {
            break;
        }
        if (strcmp(str, ht.slots[index].key) == 0) {
            hresp.found = true;
            hresp.slot = &ht.slots[index];
            break;
        }
        counter++;
        index = (index + 1) % ht.capacity;
    }

    return hresp;
}

void ht_free_all(HashTable *ht) {
    if (!ht || !ht->slots) 
        return;

    for (size_t i = 0; i < ht->capacity; i++) {
        if (ht->slots[i].used) {
            free((void *)ht->slots[i].key);
            ht->slots[i].key = NULL;
            ht->slots[i].value = 0;
            ht->slots[i].used = false;
        }
    }

    ht->size = 0;
}
