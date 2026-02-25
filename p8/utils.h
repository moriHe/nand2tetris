#ifndef UTILS_H
#define UTILS_H

#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

struct Parser;

char *get_file_root(const char *iptr);
bool main_args_validated(int argc, char *argv[]);
void process_single_file(const char *iptr, FILE *optr, char *file_root, bool is_dir, struct Parser *parser);
#endif