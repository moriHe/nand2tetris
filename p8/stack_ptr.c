#include "stack_ptr.h"

void incr_sp(FILE *optr) {
    fprintf(optr, "@SP\nM=M+1\n");
}

void load_curr_stack_addr(FILE *optr) {
    fprintf(optr, "@SP\nM=M-1\nA=M\n");
}
