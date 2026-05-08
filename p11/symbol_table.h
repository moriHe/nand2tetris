#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H
// nand2tetris Unit 5.10

typedef enum { K_STATIC, K_FIELD, K_VAR, K_ARG } Kind;
typedef struct Identifier {
    char *name; // eg x
    char *type; // eg int
    Kind kind; // STATIC or FIELD (class scope) | VAR or ARG (subroutine scope)
    int kind_index; // eg 0 (index increments separately for each kind)
    struct Identifier *next;
} Identifier;
// TODO: Hash Function und Hash Arrays initialisieren für class und subroutine

#endif