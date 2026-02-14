#ifndef PARSER_H
#define PARSER_H

enum CMD_TYPE_ENUM {
    C_PUSH,
    C_POP,
    C_ARITHMETIC,
    C_INVALID,
    C_NONE
};

struct Parser {
    FILE *input_file;
    char current_commands[3][50];
    enum CMD_TYPE_ENUM current_command_type;
    int current_index; // needs to start at -1
};

#endif