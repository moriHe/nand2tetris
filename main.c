#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

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

    // TODO: Make it more dynamic
    char raw_instr[50][50];
    size_t total_rows = 0;
    for (char buf[50]; fgets(buf, sizeof buf, asm_file); asm_file != NULL) {
        if (strlen(buf) > 50) {
            fprintf(stderr, "Error: Max chars per line is 49.\n");
            return 1;
        }

        char *comment_start = strstr(buf, "//");

        if (comment_start)
            *comment_start = '\0';

        bool is_empty_line = isspace(buf[0]);
        if (is_empty_line || buf[0] == '\0')
            continue;
        
        strcpy(raw_instr[total_rows], buf);
        total_rows++;
    }

    for (size_t i = 0; i < total_rows; i++) {
        if (strchr(raw_instr[i], '(')) {
            char *close_label = strrchr(raw_instr[i], ')');
            if (close_label == NULL) {
                fprintf(stderr, "Error: Row at index %ld does not clsoe the label.", i);
                return 1;
            }
            *close_label = '\0';
            // TODO: Save in Hash Map. Point to i+1
            printf("label = %s", raw_instr[i]);
        }
    }

    return 0;
}