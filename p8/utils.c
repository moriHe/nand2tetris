#include "utils.h"
#include "parser.h"
#include "writer.h"


char *get_file_root(const char *iptr) {
    // Start extract root file name for later use as static variable label
    char *filename_identifier = strrchr(iptr, '/');
    // If null, the arg is probably Basic.vm instead of ./Basic.vm. filename_identifier points to /. So we only need to advance if present.
    if (filename_identifier != NULL) {
        filename_identifier++;
    } else {
        filename_identifier = strdup(iptr);
    }

    size_t filename_identifier_len = strlen(filename_identifier);
    size_t cutoff = 3; // cutting off .vm
    size_t file_root_n = filename_identifier_len - cutoff + 1;    // + 1 for \0
    char *file_root = malloc(file_root_n * sizeof(char));
    strncpy(file_root, filename_identifier, filename_identifier_len - cutoff);
    file_root[file_root_n - 1] = '\0';
    return file_root;
}

bool main_args_validated(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Error: Expected 1 arg (file path), got %d instead.\n", argc);
        return false;
    }
    
    return true;
}

void process_single_file(const char *iptr, char *output_name, char *file_root) {

    FILE *vm_ptr = fopen(iptr, "r");    
    struct Parser parser = {vm_ptr, {0}, C_NONE, -1, 0, NULL};


    FILE *optr = fopen(output_name, "w");

    while (advance(&parser)) {
        write(&parser, optr, file_root);
    }

    //TODO: Should make sure that the label keyword is reserved for function end loop
    fprintf(optr, "(FN_END)\n@FN_END\n0;JMP");

    fclose(optr);
    free(file_root);
}