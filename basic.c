#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lookup.h"


void add_line_break(int *first_line, FILE *hack_file) {
    if (!*first_line)
        fputc('\n', hack_file);
    *first_line = 0;
}
const char *get_c_instr_binary(char *instr, size_t map_size, struct asmb_binary_map *binary_map) {
    for (size_t i = 0; i < map_size; i++) {
        if (strcmp(binary_map[i].asmb, instr) == 0) {
            return binary_map[i].binary;
        };
    };

    return NULL;
}

char *finish_binary(char *result, size_t idx) {
    if (idx == 15)
        return result;

    size_t pad = 16 - idx;
    for (size_t i = 0; i < pad; i++) {
        result[i] = '0';
    }

    return result;
}

void prepend_char(char *s, char c) {
    size_t len = strlen(s);

    memmove(s + 1, s, len + 1);

    s[0] = c;
}

void get_binary_addr(char *addr_inv, int quotient, int remainder, int idx) {
    if (quotient == 0 && idx == 0) {
        addr_inv[0] = '0';
        addr_inv[1] = '\0';
        return;
    }
    
    if (quotient == 0) {
        addr_inv[idx] = '\0';
        char *end = addr_inv + strlen(addr_inv) - 1;
        while (addr_inv < end) {
            *addr_inv ^= *end;
            *end ^= *addr_inv;
            *addr_inv ^= *end;
            addr_inv++;
            end--;
        }
        return;
    }
    
    int new_quo = quotient / 2;
    int new_rem = quotient % 2;
    addr_inv[idx] = new_rem ? '1' : '0';
    idx++;
    get_binary_addr(addr_inv, new_quo, new_rem, idx);
}

void get_address(char *a_instr, char *binary_addr) {
    if (!binary_addr) {
        return;
    }
    size_t max_addr_len = 16;
    size_t zero_space = max_addr_len - strlen(binary_addr);

    a_instr[0] = '0';
    for (size_t i = 1; i < zero_space; i++) {
        a_instr[i] = '0';
    }
    for (size_t i = zero_space; i <= 16; i++) {
        a_instr[i] = binary_addr[i - zero_space];
    }

    a_instr[17] = '\0';
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
    int  tmp_dec_idx = 0;
    char tmp_dec[50];
    bool is_a = false;

    // C-Instruction
    int tmp_c_subset_idx = 0;
    char tmp_c_subset[50];
    char tmp_c[] = "1110000000000000";
    bool is_c = false;
    bool is_dest = false;
    bool is_comp = false;

    bool done = false;
    // Iterate over each char in the file
    while (!done) {
        ch = fgetc(fptr);
        if (done)
            break;
        unsigned char uch = (unsigned char)ch;
        done = (ch == EOF);
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
            tmp_dec[tmp_dec_idx + 1] = '\0';
            tmp_dec[tmp_dec_idx] = uch;
            size_t len = strlen(tmp_dec);
            if (newline && len <= 1) {
                fprintf(stderr, "Error: Unexpected end of line. Address missing in A-Instruction\n"); // TODO: Add Line Number info.
                return 1;
            }

            if (!newline && !done) {
                tmp_dec_idx++;
                continue;
            }
            
            int value = atoi(tmp_dec);
            char binary_addr[17] = "";
            get_binary_addr(binary_addr, value, 0, 0);
            char address[18] = "";
            get_address(address, binary_addr);

            if (tmp_dec_idx > 0) {
                add_line_break(&first_line, hack_file);
                fprintf(hack_file, "%s", address);
            }
                
            tmp_dec[0] = '\0'; // Reset and continue
            tmp_dec_idx = 0;
            is_a = false;
            continue;
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

        if (!newline && !done) {
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
