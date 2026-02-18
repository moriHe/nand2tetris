#include "parser.h"
#include "writer_arth.h"

enum CMD_TYPE_ENUM get_command_type(char *cmd) {
    if (strcmp(cmd, "push") == 0)
        return C_PUSH;
    if (strcmp(cmd, "pop") == 0)
        return C_POP;
    if (strcmp(cmd, "label") == 0)
        return C_LABEL;
    if (strcmp(cmd, "if-goto") == 0)
        return C_IF;
    if (strcmp(cmd, "goto") == 0)
        return C_GOTO;
    for (size_t i = 0; i < arithmetic_cmds_len; i++) {
        if (strcmp(arithmetic_cmds[i], cmd) == 0)
            return C_ARITHMETIC;
    }

    fprintf(stderr, "Error: Could not find command type for cmd=%s.\n", cmd);
    return C_INVALID;
}

enum CMD_TYPE_ENUM get_current_cmd_type(struct Parser *parser) {
    return parser->current_command_type;
}

char *get_arg1(struct Parser *parser) {
    return parser->current_commands[1];
}

char *get_arg2(struct Parser *parser) {
    return parser->current_commands[2];
}


void set_parser(struct Parser *parser, char *next) {
    size_t current_bucket = 0;
    size_t current_bucket_idx = 0;
    memset(parser->current_commands, 0, sizeof(parser->current_commands));
    for (size_t i = 0; i < strlen(next); i++)
    {
        if (isspace(next[i]))
        {
            current_bucket++;
            current_bucket_idx = 0;
            continue;
        }
        parser->current_commands[current_bucket][current_bucket_idx] = next[i];
        current_bucket_idx++;
    }
    parser->current_index++;
    parser->current_command_type = get_command_type(parser->current_commands[0]);
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
        set_parser(parser, next);
        has_more_lines = true;
    } 
    return has_more_lines;
}
