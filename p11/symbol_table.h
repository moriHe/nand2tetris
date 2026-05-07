#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H
// nand2tetris Unit 5.10
typedef struct Identifier {
    char *name; // eg x
    char *type; // eg int
    enum { STATIC, FIELD, VAR, ARG } kind; // STATIC or FIELD (class scope) | VAR or ARG (subroutine scope)
    int index; // eg 0 (index increments separately for each kind)
    struct Identifier *next;
} Identifier;

#endif