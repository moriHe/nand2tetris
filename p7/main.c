#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdlib.h>
/*
Project 7 of Nand2Tetris.
* Goal: 
Build a basic VM translator. Should implement:
arithmetical-logical commands
- add, sub, neg
- eq, gt, lt
- and, or, not
push/pop commands
- push segment i
- pop segment i

The VMTranslator consists of a Parser and a CodeWriter

* Segments
- constant
- local
- argument
- this
- that
- static
- temp
- pointer (not used in this project)

* VM mapping / Hack RAM
0 -> Stack Pointer | SP
1 -> Local Segment Pointer | LCL
2 -> Argument Segment Pointer | ARG
3 -> THIS segment Pointer | THIS
4 -> THAT segment Pointer | THAT
5 - 12 -> Temp segment | temp
13 - 15 -> General Purpose
16 - 255 -> static variables  | Each static variable i in the fiel Xxx.vm is translated into the assembly symbol Xxx.i
256 - 2047 -> Stack

Parser Implementation
See PDF 91 - also here the methods:
- constructor - arg input file / stream - returns nothing
- hasMoreLines - arg nil - returns boolean 
- advance - arg nil - returns nothing
- commandType - arg nil - returns the type of the current command C_ARITHMETIC, C_PUSH or C_POP, C_LABEL, C_GOTO, C_IF, C_FUNCTION, C_RETURN, C_CALL
- arg1 - arg nil - returns first argument of current command as string
- arg2 - arg nil - returns second argument of the current command as int ONLY if C_PUSH or C_POP (or C_FUNCTION, C_CALL but that s project 8)

CodeWriter Implementation
See PDF 95 onward
- constructor - output file/stream
- writeArithmetic - arg command(string) - writes to output file the assembly code that implements the given arithmetic-logical command
- writePushPop - arg command C_PUSH or C_POP - writes push or pop command to out
- close Closes the output file

VMTranslator
Main - Drives the Parser and CodeWriter
*/

struct Writer {
    FILE *output_file;
};


char *arithmetic_cmds[] = {"add", "sub", "neg", "eq", "gt", "lt", "and", "or", "not"};
size_t arithmetic_cmds_len = sizeof arithmetic_cmds / sizeof arithmetic_cmds[0];

struct Parser {
    FILE *input_file;
    char current_commands[3][50];
    char current_command_type[50];
    int current_index; // needs to start at -1
};

void write_arithmetic(struct Writer *writer, struct Parser *parser) {
    char *instr = parser->current_commands[1];
        // below is the add statement
        // Next TODO: Add eq, lt, gt, sub, neg, and, or, not
        fprintf(writer->output_file, "@SP\nM=M-1\nA=M\nD=M\n@SP\nM=M-1\nA=M\nD=D+M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n"); // add
}


void write_push(struct Writer *writer, struct Parser *parser) {
    if (strcmp(parser->current_commands[1], "constant") == 0) {
        fprintf(writer->output_file, "@%s\nD=A\n@SP\nA=M\nM=D\n@SP\nM=M+1\n", parser->current_commands[2]);
    }
}

void write_pop() {
}

// TODO: Implement enum for the current_command_type and use switch instead of if/else in write
void write(struct Parser *parser, struct Writer *writer) {
    printf("command_type=%s\n", parser->current_command_type);
    if (strcmp(parser->current_command_type, "C_ARITHMETIC") == 0) {
        write_arithmetic(writer, parser);
    } else if (strcmp(parser->current_command_type, "C_PUSH") == 0) {
        write_push(writer, parser);
    } else if (strcmp(parser->current_command_type, "C_POP") == 0) {
        write_pop();
    } else {
        // TODO: Error Handling
    }
}

char *get_command_type(char *cmd) {
    if (strcmp(cmd, "push") == 0)
        return "C_PUSH";
    if (strcmp(cmd, "pop") == 0)
        return "C_POP";
    for (size_t i = 0; i < arithmetic_cmds_len; i++) {
        if (strcmp(arithmetic_cmds[i], cmd) == 0)
            return "C_ARITHMETIC";
    }

    fprintf(stderr, "Error: Could not find command type for cmd=%s.\n", cmd);
    return NULL;
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

// TODO: if next != null store current command_type. also: instead of current_command store arr[3] with the 3 possible commands in there
bool advance(struct Parser *parser) {
    int BUFFER_SIZE = 4096;
    char *raw = malloc(BUFFER_SIZE);
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
        parser->current_command_type[0] = '\0';
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
        strcpy(parser->current_command_type, get_command_type(parser->current_commands[0]));
        return true;
    } 
    free(raw);
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
    struct Parser parser = {vm_ptr, {0}, {0}, -1};

    char output_name[file_root_n + 4];
    sprintf(output_name, "%s.asm", file_root);
    FILE *outptr = fopen(output_name, "w");
    struct Writer writer = {outptr};

    while (advance(&parser)) {
        write(&parser, &writer);
    }

    fclose(writer.output_file);
    return 0;
}