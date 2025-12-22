#include "instr_processing.h"

// General helper
void add_line_break(int *first_line, FILE *hack_file) {
    if (!*first_line)
        fputc('\n', hack_file);
    *first_line = 0;
}


// A-Instruction helper
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

// A-Instruction helper
void get_address(char *a_instr, char *binary_addr) {
    if (!binary_addr) {
        return;
    }

    for (size_t i = 0; i < 17; i++) {
        a_instr[i] = '0';
    }

    size_t start = 16 - strlen(binary_addr);
    for (size_t i = 0; i < strlen(binary_addr); i++) {
        a_instr[start + i] = binary_addr[i];
    }

    a_instr[16] = '\0';
}


int  tmp_dec_idx = 0;
char tmp_dec[50];
// A-Instructon writer
char *process_loop_a(unsigned char uch, bool *is_a, int *first_line, FILE *hack_file, int *total_index) {
    bool newline = NEWLINE[uch];
    tmp_dec[tmp_dec_idx + 1] = '\0';
    tmp_dec[tmp_dec_idx] = uch;
    size_t len = strlen(tmp_dec);

    if (!newline) {
        tmp_dec_idx++;
        return NULL;
    }
    
    return write_a(is_a, first_line, hack_file, total_index);
}

char *write_a(bool *is_a, int *first_line, FILE *hack_file, int *total_index) {
    // TODO Error handling. Eg when there is no decimal value or no value at all after @  
    int value = atoi(tmp_dec);
    char binary_addr[17] = "";
    get_binary_addr(binary_addr, value, 0, 0);
    char address[17] = "";
    get_address(address, binary_addr);

    if (tmp_dec_idx > 0) {
        add_line_break(first_line, hack_file);
        fprintf(hack_file, "%s", address);
    }

    tmp_dec[0] = '\0'; // Reset and continue
    tmp_dec_idx = 0;
    *is_a = false;
    *total_index = *total_index + 1;
    return NULL;
}

// C-Instruction helper
const char *get_c_instr_binary(char *instr, size_t map_size, struct asmb_binary_map *binary_map) {
    for (size_t i = 0; i < map_size; i++) {
        if (strcmp(binary_map[i].asmb, instr) == 0) {
            return binary_map[i].binary;
        };
    };

    return NULL;
}

int tmp_c_subset_idx = 0;
char tmp_c_subset[50];
char tmp_c[] = "1110000000000000";
bool is_c = false;
bool is_dest = false;
bool is_comp = false;
bool c_pending = false;
// C-Instruction
char *process_loop_c(unsigned char uch, int *first_line, FILE *hack_file, int *total_index) {
    bool newline = NEWLINE[uch];
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
        c_pending = true;
        return NULL;
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
        c_pending = true;
        return NULL;
    }

    if (!newline) {
        tmp_c_subset[tmp_c_subset_idx] = uch;
        tmp_c_subset[tmp_c_subset_idx + 1] = '\0';
        tmp_c_subset_idx++;
        c_pending = true;
        return NULL;
    }

    return write_c(first_line, hack_file, total_index);
}

// Force write outside the loop
char *write_c(int *first_line, FILE *hack_file, int *total_index) {
    if (!c_pending) return NULL;

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

    add_line_break(first_line, hack_file);
    fprintf(hack_file, "%s", tmp_c);


    strcpy(tmp_c, "1110000000000000");
    tmp_c_subset[0] = '\0';
    tmp_c_subset_idx = 0;
    is_dest = false;
    is_comp = false;
    c_pending = false;
    *total_index = *total_index + 1;
    
    return NULL;
}