#include <ctype.h>
#include <stdio.h>
#include <stdbool.h>

#include "parser.h"
#include "writer.h"
#include "utils.h"

int main(int argc, char *argv[]) {
    if (!main_args_validated(argc, argv))
        return 1;
    
    const char *iptr = argv[1];
    char *file_root = get_file_root(iptr);

    FILE *vm_ptr = fopen(iptr, "r");    
    struct Parser parser = {vm_ptr, {0}, C_NONE, -1, 0, NULL};

    char output_name[ strlen(file_root) + 4];
    sprintf(output_name, "%s.asm", file_root);
    FILE *optr = fopen(output_name, "w");

    while (advance(&parser)) {
        write(&parser, optr, file_root);
    }

    //TODO: Should make sure that the label keyword is reserved for function end loop
    fprintf(optr, "(FN_END)\n@FN_END\n0;JMP");

    fclose(optr);
    free(file_root);
    return 0;
}