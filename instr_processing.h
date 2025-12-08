#ifndef INSTR_PROCESSING_H
#define INSTR_PROCESSING_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lookup.h"

extern char *process_a(unsigned char uch, bool *is_a, int *first_line, FILE *hack_file);
extern void add_line_break(int *first_line, FILE *hack_file);

#endif
