#include "instr_processing.h"

void add_line_break(int *first_line, FILE *hack_file) {
    if (!*first_line)
        fputc('\n', hack_file);
    *first_line = 0;
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
char *process_a(unsigned char uch, bool *is_a, int *first_line, FILE *hack_file) {
    bool newline = NEWLINE[uch];
    tmp_dec[tmp_dec_idx + 1] = '\0';
    tmp_dec[tmp_dec_idx] = uch;
    size_t len = strlen(tmp_dec);

    if (!newline) {
        tmp_dec_idx++;
        return NULL;
    }
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
    return NULL;
}