#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H
// nand2tetris Unit 5.10
#define HASH_SIZE 10000
typedef enum { K_STATIC, K_FIELD, K_VAR, K_ARG } Kind;
typedef struct Identifier {
    char *name; // eg x
    char *type; // eg int
    Kind kind; // STATIC or FIELD (class scope) | VAR or ARG (subroutine scope)
    int kind_index; // eg 0 (index increments separately for each kind)
    struct Identifier *next;
} Identifier;
extern Identifier *class_table[HASH_SIZE];
extern Identifier *subroutine_table[HASH_SIZE];
Identifier *get_ident(const char *name, Identifier **hash_table);
void insert_ident(const char *name, const char *type, int kind_index, Kind kind, Identifier **hash_table);
void reset_ident_table(Identifier **table);
// TODO: Hash Function und Hash Arrays initialisieren für class und subroutine

#endif