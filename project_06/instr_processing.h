#ifndef INSTR_PROCESSING_H
#define INSTR_PROCESSING_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lookup.h"

extern void get_address(char *a_instr, char *binary_addr);
extern void get_binary_addr(char *addr_inv, int quotient, int remainder, int idx);
extern char *process_loop_a(unsigned char uch, bool *is_a, int *first_line, FILE *hack_file, int *line);
extern char *process_loop_c(unsigned char uch, int *first_line, FILE *hack_file, int *line);
extern void add_line_break(int *first_line, FILE *hack_file);
extern char *write_a(bool *is_a, int *first_line, FILE *hack_file, int *line);
extern char *write_c(int *first_line, FILE *hack_file, int *line);

#endif
