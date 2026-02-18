#include "writer_arth.h"
#include "parser.h"
#include "stack_ptr.h"

const char ADD[] = "add";
const char SUB[] = "sub";
const char NEG[] = "neg";
const char EQ[] = "eq";
const char GT[] = "gt";
const char LT[] = "lt";
const char AND[] = "and";
const char OR[] = "or";
const char NOT[] = "not";
const char *arithmetic_cmds[] = {ADD, SUB, NEG, EQ, GT, LT, AND, OR, NOT};
size_t arithmetic_cmds_len = sizeof arithmetic_cmds / sizeof arithmetic_cmds[0];

void calc_latest_two(FILE *optr, char *calc) {
    decr_sp_load_a(optr);
    fprintf(optr, "D=M\n");    
    decr_sp_load_a(optr);
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
        decr_sp_load_a(optr);
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
        decr_sp_load_a(optr);
        fprintf(optr, "M=!M\n");
        incr_sp(optr);
    }
}