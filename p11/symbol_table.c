#include <stdlib.h>
#include <string.h>
#include "symbol_table.h"
#define HASH_SIZE 10000

Identifier *class_table[HASH_SIZE] = {NULL};
Identifier *subroutine_table[HASH_SIZE] = {NULL};

static unsigned long hash(const char *str) {
    unsigned long hash_num = 5381;
    int c;
    while (c = *str++) {
        hash_num = ((hash_num << 5) + hash_num) + c;
    }

    return hash_num;
}

Identifier *get_ident(const char *name, Identifier **hash_table) {
    unsigned long hash_num = hash(name);
    int index = hash_num % HASH_SIZE;

    Identifier *node = hash_table[index];
    while (node != NULL) {
        if (strcmp(node->name, name) == 0) {
            return node;
        }
        node = node->next;
    }

    return NULL;
}

void insert_ident(const char *name, const char *type, int kind_index, const char *kind_str, Identifier **hash_table)  {
    unsigned long hash_num = hash(name);
    int index = hash_num % HASH_SIZE;

    Identifier *new_node = malloc(sizeof(Identifier));

    new_node->name = strdup(name);
    new_node->type = strdup(type);
    if (strcmp(kind_str, "static") == 0) new_node->kind = K_STATIC;
    else if (strcmp(kind_str, "field") == 0) new_node->kind = K_FIELD;
    else if (strcmp(kind_str, "var") == 0) new_node->kind = K_VAR;
    else if (strcmp(kind_str, "arg") == 0) new_node->kind = K_ARG;
    new_node->kind_index = kind_index;

    new_node->next = hash_table[index];
    hash_table[index] = new_node;
}

void reset_ident_table(Identifier **table) {
    for (int i = 0; i < HASH_SIZE; i ++) {
        Identifier *current = table[i];
        while (current != NULL) {
            Identifier *temp = current;
            current = current->next;
            free(temp->name);
            free(temp->type);
            free(temp);
        }
        table[i] = NULL;
    }
}