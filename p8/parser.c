#include "parser.h"
#include "writer_arth.h"

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