#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>
#include <stdbool.h>

// TODO: free CurrentInstr when used
typedef struct CurrentInstr {
    char *type;
    char *value;
} CurrentInstr;

typedef struct Node {
    char *key;
    struct Node *next;
} Node;

CurrentInstr advance_parser(FILE *jack_file);
Node *get_node(const char *key);
void insert_node(const char *key);
bool is_int(const char *str);

#endif