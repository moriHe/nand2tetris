#ifndef HASH_H
#define HASH_H

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

extern const char *ht_insert(const char *str, int value);
extern HashResp ht_get(const char *str);
extern void ht_free_all(HashTable *ht);

#endif