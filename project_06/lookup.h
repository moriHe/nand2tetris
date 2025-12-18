// tables.h
#ifndef LOOKUP_H
#define LOOKUP_H

#include <stdbool.h>
#include <stdio.h>

extern bool WHITESPACE[256];
extern bool NEWLINE[256];
extern bool A_INSTR[256];
extern bool COMMENT[256];
extern bool SEMICOLON[256];
extern bool EQUAL_SIGN[256];

struct asmb_binary_map {
    const char *asmb;
    const char *binary;
};

extern struct asmb_binary_map dest_map[];
extern const size_t dest_map_size;
extern struct asmb_binary_map jmp_map[];
extern const size_t jmp_map_size;
extern struct asmb_binary_map comp_map[];
extern const size_t comp_map_size;

#endif
