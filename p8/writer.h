#ifndef WRITER_H
#define WRITER_H

#include <stdio.h>

struct Parser;

void write(struct Parser *parser, FILE *optr, const char *output_name);

#endif