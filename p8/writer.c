#include <stdlib.h>
#include "writer.h"
#include "parser.h"
#include "writer_arth.h"
#include "writer_push_pop.h"
#include "stack_ptr.h"

void write(struct Parser *parser, FILE *optr, const char *output_name) {
    switch (get_current_cmd_type(parser))
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
    case C_GOTO:
        fprintf(optr, "@%s\n0;JMP\n", get_arg1(parser));
        break;
    case C_IF:
        decr_sp_load_a(optr);
        fprintf(optr, "D=M\n@%s\nD;JNE\n", get_arg1(parser));
        break;
    case C_LABEL:
        fprintf(optr, "(%s)\n", get_arg1(parser));
        break;
    case C_FUNCTION:
        fprintf(optr, "(%s)\n", get_arg1(parser));
        int arg2 = atoi(get_arg2(parser)); // TODO: Handle error
        for (size_t i = 0; i < arg2; i++) {
            fprintf(optr, "@0\nD=A\n@SP\nA=M\nM=D\n");
            incr_sp(optr);
        }
        break;
    case C_CALL:
        // Safe return address on the stack
        fprintf(optr, "@%s$ret.%d\nD=A\n@SP\nA=M\nM=D\n", parser->current_fn, parser->current_call_index);
        incr_sp(optr);

        // Safe old LCL, ARG, THIS, THAT
        fprintf(optr, "@LCL\nD=M\n@SP\nA=M\nM=D\n");
        incr_sp(optr);
        fprintf(optr, "@ARG\nD=M\n@SP\nA=M\nM=D\n");
        incr_sp(optr);
        fprintf(optr, "@THIS\nD=M\n@SP\nA=M\nM=D\n");
        incr_sp(optr);
        fprintf(optr, "@THAT\nD=M\n@SP\nA=M\nM=D\n");
        incr_sp(optr);

        // Safe new ARG and LCL
        fprintf(optr, "@SP\nD=M\n@5\nD=D-A\n@%s\nD=D-A\n@ARG\nM=D\n", get_arg2(parser));
        fprintf(optr, "@SP\nD=M\n@LCL\nM=D\n");

        // TODO: Outsource goto command. It is used here and in case C_GOTO
        fprintf(optr, "@%s\n0;JMP\n", get_arg1(parser));
        fprintf(optr, "(%s$ret.%d)\n", parser->current_fn, parser->current_call_index);
        break;
    case C_RETURN:
        // Temp store LCL
        fprintf(optr, "@LCL\nD=M\n@R14\nM=D\n");
        // Temp store return address
        fprintf(optr, "@LCL\nD=M\n@5\nD=D-A\nA=D\nD=M\n@R15\nM=D\n");
        // Pop function return value onto stack position of arg 0
        pop(optr, "ARG", "0");
        // Point stack to position of arg 0 + 1
        fprintf(optr, "@ARG\nD=M+1\n@SP\nM=D\n");
        
        fprintf(optr, "@R14\nD=M\n@1\nD=D-A\nA=D\nD=M\n@THAT\nM=D\n");
        fprintf(optr, "@R14\nD=M\n@2\nD=D-A\nA=D\nD=M\n@THIS\nM=D\n");
        fprintf(optr, "@R14\nD=M\n@3\nD=D-A\nA=D\nD=M\n@ARG\nM=D\n");
        fprintf(optr, "@R14\nD=M\n@4\nD=D-A\nA=D\nD=M\n@LCL\nM=D\n");
        fprintf(optr, "@R15\nA=M\n0;JMP\n");
        break;
    default:
        break;
    }
}