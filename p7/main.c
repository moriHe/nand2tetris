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

const char ADD[] = "add";
const char SUB[] = "sub";
const char NEG[] = "neg";
const char EQ[] = "eq";
const char GT[] = "gt";
const char LT[] = "lt";
const char AND[] = "and";
const char OR[] = "or";
const char NOT[] = "not";

enum CMD_TYPE_ENUM {
    C_PUSH,
    C_POP,
    C_ARITHMETIC,
    C_INVALID,
    C_NONE
};

const char *arithmetic_cmds[] = {ADD, SUB, NEG, EQ, GT, LT, AND, OR, NOT};
size_t arithmetic_cmds_len = sizeof arithmetic_cmds / sizeof arithmetic_cmds[0];

struct Parser {
    FILE *input_file;
    char current_commands[3][50];
    enum CMD_TYPE_ENUM current_command_type;
    int current_index; // needs to start at -1
};

void incr_sp(FILE *optr) {
    fprintf(optr, "@SP\nM=M+1\n");
}

void load_curr_stack_addr(FILE *optr) {
    fprintf(optr, "@SP\nM=M-1\nA=M\n");
}

void calc_latest_two(FILE *optr, char *calc) {
    load_curr_stack_addr(optr);
    fprintf(optr, "D=M\n");    
    load_curr_stack_addr(optr);
    fprintf(optr, "%s\n", calc);
}

void calc_latest_two_push_result_stack(FILE *optr, char *calc) {
    calc_latest_two(optr, calc);
    fprintf(optr, "@SP\nA=M\nM=D\n");
    incr_sp(optr);
}

void set_m_bool(FILE *optr, int bool_asm_val) {
    fprintf(optr, "@SP\nA=M\nM=%d\n", bool_asm_val);
}

void calc_bool_op(FILE *optr, int curr_idx, char *op) {
    calc_latest_two(optr, "D=M-D");
    fprintf(optr, "@%s_%d\nD;J%s\n", op, curr_idx, op);
    set_m_bool(optr, 0);
    fprintf(optr, "@END_%s_%d\n0;JMP\n(%s_%d)\n", op, curr_idx, op, curr_idx);
    set_m_bool(optr, -1);
    fprintf(optr, "(END_%s_%d)\n", op, curr_idx);
    incr_sp(optr);
}

// Requiremens:
// * Nand2Tetris is an Empty Stack implementation.
// * At the end of each operation, RAM[SP] needs to point at en empty value
// * RAM[SP - 1] is the latest value
void write_arithmetic(FILE *optr, struct Parser *parser) {
    char *instr = parser->current_commands[0];
    int curr_idx = parser->current_index;

    if (strcmp(instr, ADD) == 0) {
        calc_latest_two_push_result_stack(optr, "D=D+M");
    }
    else if (strcmp(instr, SUB) == 0) {
        calc_latest_two_push_result_stack(optr, "D=M-D");
    } 
    else if (strcmp(instr, NEG) == 0) {
        load_curr_stack_addr(optr);
        fprintf(optr, "M=-M\n");
        incr_sp(optr);
    }
    else if (strcmp(instr, EQ) == 0) {
        calc_bool_op(optr, curr_idx, "EQ");
    }
    else if (strcmp(instr, GT) == 0) {
        calc_bool_op(optr, curr_idx, "GT");
    }
    else if (strcmp(instr, LT) == 0) {
        calc_bool_op(optr, curr_idx, "LT");
    }
    else if (strcmp(instr, AND) == 0) {
        calc_latest_two_push_result_stack(optr, "D=D&M");
    }
    else if (strcmp(instr, OR) == 0) {
        calc_latest_two_push_result_stack(optr, "D=D|M");
    }
    else if (strcmp(instr, NOT) == 0) {
        load_curr_stack_addr(optr);
        fprintf(optr, "M=!M\n");
        incr_sp(optr);
    }
}

bool is_cmd(const char *curr_cmd, const char *comparator) {
    if (strcmp(curr_cmd, comparator) == 0)
        return true;
    return false;
}

void write_offset_a(FILE *optr, const char *start_val, const char *offset) {
    fprintf(optr, "@%s\nD=M\n@%s\nD=D+A\n", start_val, offset);
}

void pop(FILE *optr, const char *start_val, const char *offset) {
    write_offset_a(optr, start_val, offset);
    fprintf(optr, "@R13\nM=D\n");
    load_curr_stack_addr(optr);
    fprintf(optr, "D=M\n@R13\nA=M\nM=D\n");
}

void push(FILE *optr, const char *start_val, const char *offset) {
    write_offset_a(optr, start_val, offset);
    fprintf(optr, "A=D\nD=M\n@SP\nA=M\nM=D\n");
    incr_sp(optr);
}

void write_push(FILE *optr, struct Parser *parser, const char *output_name) {
    const char *curr_cmd = parser->current_commands[1];
    const char *offset = parser->current_commands[2];

    if (is_cmd(curr_cmd, "constant")) {
        fprintf(optr, "@%s\nD=A\n@SP\nA=M\nM=D\n", offset);
        incr_sp(optr);
    }
    else if (is_cmd(curr_cmd, "local")) {
        push(optr, "LCL", offset);
    }
    else if (is_cmd(curr_cmd, "argument")) {
        push(optr, "ARG", offset);
    }
    else if (is_cmd(curr_cmd, "this")) {
       push(optr, "THIS", offset);
    }
    else if (is_cmd(curr_cmd, "that")) {
       push(optr, "THAT", offset);
    }
    else if (is_cmd(curr_cmd, "temp")) {
        fprintf(optr, "@5\nD=A\n@%s\nD=D+A\nA=D\nD=M\n@SP\nA=M\nM=D\n", offset);
        incr_sp(optr);
    } 
    else if (is_cmd(curr_cmd, "pointer")) {
        if (strcmp(offset, "0") == 0) {
            fprintf(optr, "@THIS\nD=M\n@SP\nA=M\nM=D\n");
        } else {
            fprintf(optr, "@THAT\nD=M\n@SP\nA=M\nM=D\n");
        }
        incr_sp(optr);
    } else if (is_cmd(curr_cmd, "static")) {
        char addr[256];
        char idx_str[20];
        snprintf(idx_str, sizeof(idx_str), "%d", parser->current_index);
        strcpy(addr, output_name);
        strcat(addr, ".");
        strcat(addr, idx_str);
        push(optr, addr, offset);
    }
}

void write_pop(FILE *optr, struct Parser *parser, const char *output_name) {
    const char *curr_cmd = parser->current_commands[1];
    const char *offset = parser->current_commands[2];

    if (is_cmd(curr_cmd, "local")) {
        pop(optr, "LCL", offset);
    }
    else if (is_cmd(curr_cmd, "argument")) {
        pop(optr, "ARG", offset);
    }
    else if (is_cmd(curr_cmd, "this")) {
        pop(optr, "THIS", offset);
    }
    else if (is_cmd(curr_cmd, "that")) {
        pop(optr, "THAT", offset);
    }
    else if (is_cmd(curr_cmd, "temp")) {
        fprintf(optr, "@5\nD=A\n@%s\nD=D+A\n@R13\nM=D\n", offset);
        load_curr_stack_addr(optr);
        fprintf(optr, "D=M\n@R13\nA=M\nM=D\n");
    }
    else if (is_cmd(curr_cmd, "pointer")) {
        if (strcmp(offset, "0") == 0) {
            fprintf(optr, "@SP\nAM=M-1\nD=M\n@THIS\nM=D\n");
        } else {
            fprintf(optr, "@SP\nAM=M-1\nD=M\n@THAT\nM=D\n");
        }
    } else if (is_cmd(curr_cmd, "static")) {
        char addr[256];
        char idx_str[20];
        snprintf(idx_str, sizeof(idx_str), "%d", parser->current_index);
        strcpy(addr, output_name);
        strcat(addr, ".");
        strcat(addr, idx_str);
        pop(optr, addr, offset);
    }
}

// TODO: Implement enum for the current_command_type and use switch instead of if/else in write
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
        write(&parser, optr, output_name);
    }

    fprintf(optr, "(END)\n@END\n0;JMP");

    fclose(optr);
    return 0;
}