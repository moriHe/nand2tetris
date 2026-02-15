#ifndef WRITER_ARTH_H
#define WRITER_ARTH_H

#include <stdio.h>
#include <string.h>

struct Parser;

extern const char ADD[];
extern const char SUB[];
extern const char NEG[];
extern const char EQ[];
extern const char GT[];
extern const char LT[];
extern const char AND[];
extern const char OR[];
extern const char NOT[];

extern const char *arithmetic_cmds[];
extern size_t arithmetic_cmds_len;

void calc_latest_two(FILE *optr, char *calc);
void calc_latest_two_push_result_stack(FILE *optr, char *calc);
void set_m_bool(FILE *optr, int bool_asm_val);
void calc_bool_op(FILE *optr, int curr_idx, char *op);
void write_arithmetic(FILE *optr, struct Parser *parser);


#endif