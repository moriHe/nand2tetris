#include "writer_push_pop.h"
#include "parser.h"
#include "stack_ptr.h"

void write_offset_a(FILE *optr, const char *start_val, const char *offset) {
    fprintf(optr, "@%s\nD=M\n@%s\nD=D+A\n", start_val, offset);
}

bool is_cmd(const char *curr_cmd, const char *comparator) {
    if (strcmp(curr_cmd, comparator) == 0)
        return true;
    return false;
}

// Hardware operation pop. vm command pop is decr_sp
void pop(FILE *optr, const char *start_val, const char *offset) {
    write_offset_a(optr, start_val, offset);
    fprintf(optr, "@R13\nM=D\n");
    decr_sp_load_a(optr);
    fprintf(optr, "D=M\n@R13\nA=M\nM=D\n");
}

void write_pop(FILE *optr, struct Parser *parser, const char *output_name) {
    const char *curr_cmd = get_arg1(parser);
    const char *offset = get_arg2(parser);

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
        decr_sp_load_a(optr);
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
        strcpy(addr, output_name);
        strcat(addr, ".");
        strcat(addr, offset);

        fprintf(optr, "@%s\nD=A\n@R13\nM=D\n", addr);
        decr_sp_load_a(optr);
        fprintf(optr, "D=M\n@R13\nA=M\nM=D\n");
    }
}

// Hardware operation push. vm command push is incr_sp
void push(FILE *optr, const char *start_val, const char *offset) {
    write_offset_a(optr, start_val, offset);
    fprintf(optr, "A=D\nD=M\n@SP\nA=M\nM=D\n");
    incr_sp(optr);
}

void write_push(FILE *optr, struct Parser *parser, const char *output_name) {
    const char *curr_cmd = get_arg1(parser);
    const char *offset = get_arg2(parser);

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
        strcpy(addr, output_name);
        strcat(addr, ".");
        strcat(addr, offset);
        push(optr, addr, "0");
    }
}