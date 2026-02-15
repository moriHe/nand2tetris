#ifndef WRITER_PUSH_POP_H
#define WRITER_PUSH_POP_H

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

struct Parser;

void write_offset_a(FILE *optr, const char *start_val, const char *offset);
bool is_cmd(const char *curr_cmd, const char *comparator);

void pop(FILE *optr, const char *start_val, const char *offset);
void write_pop(FILE *optr, struct Parser *parser, const char *output_name);

void push(FILE *optr, const char *start_val, const char *offset);
void write_push(FILE *optr, struct Parser *parser, const char *output_name);
#endif