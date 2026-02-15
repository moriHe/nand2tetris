#include "writer.h"
#include "parser.h"
#include "writer_arth.h"
#include "writer_push_pop.h"

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