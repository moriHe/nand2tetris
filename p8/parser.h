#ifndef PARSER_H
#define PARSER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <ctype.h>

enum CMD_TYPE_ENUM {
    C_PUSH,
    C_POP,
    C_ARITHMETIC,
    C_LABEL,
    C_IF,
    C_INVALID,
    C_NONE
};

struct Parser {
    FILE *input_file;
    char current_commands[3][50];
    enum CMD_TYPE_ENUM current_command_type;
    int current_index; // needs to start at -1
};

enum CMD_TYPE_ENUM get_command_type(char *cmd);
enum CMD_TYPE_ENUM get_current_cmd_type(struct Parser *parser);
char *get_arg1(struct Parser *parser);
char *get_arg2(struct Parser *parser);
bool advance(struct Parser *parser);

void set_parser(struct Parser *parser, char *next);

#endif