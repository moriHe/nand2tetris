#include <ctype.h>
#include <stdio.h>
#include <stdbool.h>

#include "push_pop.h"
#include "arithmetic_ops.h"
#include "parser.h"

void write(struct Parser *parser, FILE *optr, const char *output_name) {
    switch (parser->current_command_type)
    {
    case C_ARITHMETIC:
        write_arithmetic(optr, parser);
        break;
    case C_PUSH:
         write_push(optr, parser, output_name);
         break;
    case C_POP:
        write_pop(optr, parser, output_name);
        break;
    default:
        break;
    }
}

enum CMD_TYPE_ENUM get_command_type(char *cmd) {
    if (strcmp(cmd, "push") == 0)
        return C_PUSH;
    if (strcmp(cmd, "pop") == 0)
        return C_POP;
    for (size_t i = 0; i < arithmetic_cmds_len; i++) {
        if (strcmp(arithmetic_cmds[i], cmd) == 0)
            return C_ARITHMETIC;
    }

    fprintf(stderr, "Error: Could not find command type for cmd=%s.\n", cmd);
    return C_INVALID;
}

char *get_arg1(struct Parser *parser, char *command_type) {
    // if type is C_ARITHMETIC -> return commands[0] (add, sub, neg, eq, gt, lt, and, or, not)
    // if type is C_PUSH/C_POP, C_FUNCTION, C_CALL, C_LABEL, C_GOTO, C_If-GOTO -> return commands[1]
    return parser->current_commands[1];
}

char *get_arg2(struct Parser *parser, char *command_type) {
    // only return if C_PUSH, C_POP, C_FUNCTION, C_CALL (in this project only push/pop)
    return parser->current_commands[2];
}

bool advance(struct Parser *parser) {
    int BUFFER_SIZE = 4096;
    char raw[BUFFER_SIZE];
    char *next = NULL;
    while (next == NULL) {
        next = raw;
        if (fgets(raw, BUFFER_SIZE, parser->input_file) == NULL) {
            next = NULL;
            break;
        }
        char *comment_start = strstr(next, "//");
        if (comment_start)
            *comment_start = '\0';
        
        while (isspace(*next))
            next++;

        if (*next == 0) {
            next = NULL;
            continue;
        }

        char *end = next + strlen(next) - 1;
        while (end > next && isspace(*end)) {
            *end = '\0';
            end--;
        }

        if (strlen(next) == 0) {
            next = NULL;
            continue;
        }
    }
    
    bool has_more_lines = false;
    if (next != NULL) {
        size_t current_bucket = 0;
        size_t current_bucket_idx = 0;
        memset(parser->current_commands, 0, sizeof(parser->current_commands));
        for (size_t i = 0; i < strlen(next); i++) {
            if (isspace(next[i])) {
                current_bucket++;
                current_bucket_idx = 0;
                continue;
            }
            parser->current_commands[current_bucket][current_bucket_idx] = next[i];
            current_bucket_idx++;
        }
        parser->current_index++;
        parser->current_command_type = get_command_type(parser->current_commands[0]);
        return true;
    } 
    return has_more_lines;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Error: Expected 1 arg (file path), got %d instead.\n", argc);
        return 1;
    }
    
    const char *iptr = argv[1];
    char *ending = strrchr(iptr, '.');
    if (strcmp(ending, ".vm") != 0) {
        fprintf(stderr, "Error: Wrong input format. Expected .vm, got %s instead\n", ending);
        return 1;
    }

    // Start extract root file name for later use as static variable label
    char *filename_identifier = strrchr(iptr, '/');
    // If null, the arg is probably Basic.vm instead of ./Basic.vm. filename_identifier points to /. So we only need to advance if present.
    if (filename_identifier != NULL) {
        filename_identifier++;
    }
    
    size_t filename_identifier_len = strlen(filename_identifier);
    size_t cutoff = 3; // cutting off .vm
    size_t file_root_n = filename_identifier_len - cutoff + 1;    // + 1 for \0
    char file_root[file_root_n];
    strncpy(file_root, filename_identifier, filename_identifier_len - cutoff);
    file_root[file_root_n - 1] = '\0';
    // End extract root file name for later use as static variable label

    FILE *vm_ptr = fopen(iptr, "r");    
    struct Parser parser = {vm_ptr, {0}, C_NONE, -1};

    char output_name[file_root_n + 4];
    sprintf(output_name, "%s.asm", file_root);
    FILE *optr = fopen(output_name, "w");

    while (advance(&parser)) {
        write(&parser, optr, file_root);
    }

    fprintf(optr, "(END)\n@END\n0;JMP");

    fclose(optr);
    return 0;
}