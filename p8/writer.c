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
    case C_IF:
        decr_sp(optr);
        fprintf(optr, "D=M\n@%s\nD;JNE\n", get_arg1(parser));
        break;
    case C_LABEL:
        fprintf(optr, "(%s)\n", get_arg1(parser));
        break;
    default:
        break;
    }
}