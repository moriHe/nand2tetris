#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lookup.h"
#include "instr_processing.h"

// TODO: Add END JMP
// TODO: Handle Variables
const char *get_c_instr_binary(char *instr, size_t map_size, struct asmb_binary_map *binary_map) {
    for (size_t i = 0; i < map_size; i++) {
        if (strcmp(binary_map[i].asmb, instr) == 0) {
            return binary_map[i].binary;
        };
    };

    return NULL;
}

int main(int argc, char *argv[]) {
    // Check validity args
    if (argc < 2) {
        fprintf(stderr, "Error: no filename provided.\n");
        return 1;
    }

    const char *filename = argv[1];
    size_t filename_length = strlen(filename);
    if (filename_length < 5)
    {
        fprintf(stderr, "Error: filename is too short.\n");
        return 1;
    }
    const int not_asm = strcmp(filename + filename_length - 4, ".asm");
    if (not_asm) {
        fprintf(stderr, "Error: wrong file format. File needs to end on .asm.\n");
        return 1;
    }

    // Load asm file (read from)
    FILE* fptr;
    fptr = fopen(filename, "r");
    if (fptr == NULL) {
        fprintf(stderr, "Error: file could not be opened.");
        return 1;
    }

    // Load hack file (written to)
    // TODO: Take name of incoming file and name outgoing file the same with .hack ending
    FILE *hack_file = fopen("test.hack", "w");
    if (hack_file == NULL) {
        fprintf(stderr, "Error: Initializing hack file.");
        return 1;
    }
    
    int ch;

    // Decides whether to write \n or not after newline
    int first_line = 1;
    // While true skips ch processing until newline
    bool is_comment = false;

    // A-Instruction helpers
    bool is_a = false;

    // C-Instruction
    int tmp_c_subset_idx = 0;
    char tmp_c_subset[50];
    char tmp_c[] = "1110000000000000";
    bool is_c = false;
    bool is_dest = false;
    bool is_comp = false;

    // Iterate over each char in the file
    while ((ch = fgetc(fptr)) != EOF) {

        unsigned char uch = (unsigned char)ch;
        bool newline = NEWLINE[uch];
        // 1. comment and whitespace logic for skipping
        if (COMMENT[uch]) {
            is_comment = true;
            continue;
        }

        if (is_comment && !newline) {
            continue;
        }

        if (is_comment && newline) {
            is_comment = false;
        }

        if (newline && strlen(tmp_c_subset) == 0 && !is_a) {
            continue;
        }

        // 2. A-Instruction
        // If @ set marker for A-Instruction, proceed.
        if (A_INSTR[uch]) {
            is_a = true;
            continue;
        }

        // Gather decimal value and compute A-Instruction. Write it into the hack file.
        if (is_a) {
            char *error = process_a(uch, &is_a, &first_line, hack_file);
            if (error == NULL) {
                continue;
            }
            else {
                fprintf(stderr, "Error: processing a instruction failed.");
                return 1;
            }
        }

        if (!is_dest && EQUAL_SIGN[uch]) {
            is_dest = true;
            const char *binary_str = get_c_instr_binary(tmp_c_subset, dest_map_size, dest_map);
            if (binary_str != NULL) {
                tmp_c[10] = binary_str[0];
                tmp_c[11] = binary_str[1];
                tmp_c[12] = binary_str[2];
            }
            tmp_c_subset[0] = '\0';
            tmp_c_subset_idx = 0;
            continue;
        }

        if (!is_comp && SEMICOLON[uch]) {
            is_comp = true;
            const char *binary_str = get_c_instr_binary(tmp_c_subset, comp_map_size, comp_map);
            if (binary_str != NULL) {
                tmp_c[3] = binary_str[0];
                tmp_c[4] = binary_str[1];
                tmp_c[5] = binary_str[2];
                tmp_c[6] = binary_str[3];
                tmp_c[7] = binary_str[4];
                tmp_c[8] = binary_str[5];
                tmp_c[9] = binary_str[6];
            }
            tmp_c_subset[0] = '\0';
            tmp_c_subset_idx = 0;
            continue;
        }

        if (!newline) {
            tmp_c_subset[tmp_c_subset_idx] = uch;
            tmp_c_subset[tmp_c_subset_idx + 1] = '\0';
            tmp_c_subset_idx++;
            continue;
        }
        
        if (!is_comp) {
            const char *binary_str = get_c_instr_binary(tmp_c_subset, comp_map_size, comp_map);
            if (binary_str != NULL) {
                tmp_c[3] = binary_str[0];
                tmp_c[4] = binary_str[1];
                tmp_c[5] = binary_str[2];
                tmp_c[6] = binary_str[3];
                tmp_c[7] = binary_str[4];
                tmp_c[8] = binary_str[5];
                tmp_c[9] = binary_str[6];
            }
        } else {
            const char *binary_str = get_c_instr_binary(tmp_c_subset, jmp_map_size, jmp_map);
            if (binary_str != NULL) {
                tmp_c[13] = binary_str[0];
                tmp_c[14] = binary_str[1];
                tmp_c[15] = binary_str[2];
            }
        }

        if (tmp_c_subset_idx > 0) {
            add_line_break(&first_line, hack_file);
            fprintf(hack_file, "%s", tmp_c);
        }

        strcpy(tmp_c, "1110000000000000");
        tmp_c_subset[0] = '\0';
        tmp_c_subset_idx = 0;
        is_dest = false;
        is_comp = false;
        continue;
    }

    fclose(hack_file);
    fclose(fptr);

    return 0;
}
