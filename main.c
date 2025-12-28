#include <stdio.h>
#include <string.h>

int  main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Error: Only one arg allowed -> The path to the asm file.\n");
        return 1;
    }
    
    const char *file_path = argv[1];
    const char *file_sep = strrchr(file_path, '.');
    if (strcmp(".asm", file_sep) != 0) {
        fprintf(stderr, "Error: Wrong file identifier.\n");
        return 1;
    }

    FILE *asm_file = fopen(file_path, "r");
    if (asm_file == NULL) {
        fprintf(stderr, "Error: Could not open the file. Is the path correct?");
    }

    for (char buf[50]; fgets(buf, sizeof buf, asm_file); asm_file != NULL) {
        if (strlen(buf) > 50) {
            fprintf(stderr, "Error: Max chars per line is 49.\n");
            return 1;
        }
        char *comment_start = strchr(buf, '/');
        
    }

    return 0;
}