#include "stack_ptr.h"

void incr_sp(FILE *optr) {
    fprintf(optr, "@SP\nM=M+1\n");
}

void decr_sp_load_a(FILE *optr) {
    fprintf(optr, "@SP\nM=M-1\nA=M\n");
}
