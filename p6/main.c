#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

const int TABLE_SIZE = 10000;

struct entry {
    char *key;
    int value;
    struct entry *next;

};

struct hash_table {
    struct entry **entries;
};

struct hash_resp {
    char *err;
    unsigned long value;
};

struct hash_resp hash(const char *key) {
    struct hash_resp resp = {NULL, 0};
    if (key == NULL) {
        resp.err = "Error: No key passed to hash function.";
        return resp;
    }
    
    unsigned long value = 0;
    size_t key_len = strlen(key);

    for (size_t i = 0; i <key_len; i++) {
        value = value * 37 + key[i];
    }

    value = value % TABLE_SIZE;
    resp.value = value;
    return resp;
}

struct entry *ht_entry_set(const char *key, int value) {
    struct entry *entry =  malloc(sizeof(struct entry));
    size_t len = strlen(key);
    char *copy = malloc(len + 1);
    memcpy(copy, key, len + 1);
    entry->key = copy;
    entry->value = value;
    entry->next = NULL;
    return entry;
}

void ht_set(const char *key, int value, struct hash_table *ht) {
    struct hash_resp h_resp = hash(key);

    if (h_resp.err != NULL)
        return;


    if (ht->entries[h_resp.value] == NULL) {
        ht->entries[h_resp.value] = ht_entry_set(key, value);
        return;
    }

    struct entry *prev;
    struct entry *entry = ht->entries[h_resp.value];

    while (entry != NULL) {
        if (strcmp(entry->key, key) == 0) {
            entry->value = value;
            return;
        }

        prev = entry;
        entry = prev->next;
    }

    prev->next = ht_entry_set(key, value);
    return;
}

struct entry *ht_get(const char *key, struct hash_table *ht) {
    struct hash_resp h_resp = hash(key);
    if (h_resp.err != NULL) {
        printf("Error was not null.\n");
                return NULL;
    }
    struct entry *entry = ht->entries[h_resp.value];
    if (entry == NULL)
        return NULL;
    
    bool found = false;
    while (!found) {
        if (strcmp(entry->key, key) == 0) {
            found = true;
            break;
        }
        entry = entry->next;
    }

    return entry;
}

void dec_to_binary(int dec, char s[17]) {
    if (dec < 0 || dec > 32767) {
        // TODO: Handle Error
        return ;
    }
    int binary[15] = {0};

    int i = 0;
    while (dec > 0) {
        binary[i] = dec % 2;
        dec = dec / 2;
        i++;
    }

    s[16] = '\0';
    s[0] = '0'; // Identifies row as A-Instruction
    int k = 0;
    for (int j = 15; j > 0; j--) {
        s[j] = binary[15-j] == 0 ? '0' : '1';
        k++;
    }
    return;
}

void ht_dump(struct hash_table *ht) {
    for (size_t i = 0; i < TABLE_SIZE; i++) {
        printf("ht at i=%zu: ", i);
        struct entry *entry = ht->entries[i];
        if (!entry) {
            printf("(empty)\n"); 
            continue; 
        }

        while (entry) {
            printf("%s=%d", entry->key, entry->value);
            if (entry->next) 
                printf(" -> ");
            entry = entry->next;
        }
        printf("\n");
    }
}

struct c_instr {
    const char *key;
    const char *value;
};

static const struct c_instr comp[] = {
    {"0",   "0101010"},
    {"1",   "0111111"},
    {"-1",  "0111010"},
    {"D",   "0001100"},
    {"A",   "0110000"},
    {"!D",  "0001101"},
    {"!A",  "0110001"},
    {"-D",  "0001111"},
    {"-A",  "0110011"},
    {"D+1", "0011111"},
    {"A+1", "0110111"},
    {"D-1", "0001110"},
    {"A-1", "0110010"},
    {"D+A", "0000010"},
    {"D-A", "0010011"},
    {"A-D", "0000111"},
    {"D&A", "0000000"},
    {"D|A", "0010101"},

    // a = 1 (M memory based)
    {"M",   "1110000"},
    {"!M",  "1110001"},
    {"-M",  "1110011"},
    {"M+1", "1110111"},
    {"M-1", "1110010"},
    {"D+M", "1000010"},
    {"D-M", "1010011"},
    {"M-D", "1000111"},
    {"D&M", "1000000"},
    {"D|M", "1010101"},
};

int num_comp = sizeof(comp) / sizeof(comp[0]);

static const struct c_instr jump[] = {
    {"",    "000"},
    {"JGT", "001"},
    {"JEQ", "010"},
    {"JGE", "011"},
    {"JLT", "100"},
    {"JNE", "101"},
    {"JLE", "110"},
    {"JMP", "111"},
};

int num_jump = sizeof(jump) / sizeof(jump[0]);

static const struct c_instr dest[] = {
    {"",    "000"},
    {"M",   "001"},
    {"D",   "010"},
    {"MD",  "011"},
    {"A",   "100"},
    {"AM",  "101"},
    {"AD",  "110"},
    {"AMD", "111"},
};

int num_dest = sizeof(dest) / sizeof(dest[0]);

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

    struct hash_table ht = {.entries = calloc(TABLE_SIZE, sizeof(struct entry*))};
    if (ht.entries == NULL) {
        fprintf(stderr, "Error: Failed to alloc memory for hash table.");
        return 1;
    }
    // TODO: Make it more dynamic
    char raw_instr[50][50];
    size_t total_rows = 0;
    size_t raw_file_idx = -1;
    for (char buf[50]; fgets(buf, sizeof buf, asm_file); asm_file != NULL) {
        raw_file_idx++;
        if (strlen(buf) > 50) {
            fprintf(stderr, "Error: Max chars per line is 49.\n");
            return 1;
        }

        char *comment_start = strstr(buf, "//");

        if (comment_start)
            *comment_start = '\0';

            size_t buf_len = strlen(buf);
        char buf_no_whitespace[50] = {'\0'};
        int idx = 0;
        for (size_t j = 0; j < buf_len; j++) {
            if (isspace(buf[j]))
                continue;
            buf_no_whitespace[idx] = buf[j];
            idx++;
        }

        buf_no_whitespace[idx] = '\0';
        if (isspace(buf_no_whitespace[0]) || buf[0] == '\0')
            continue;

        if (strchr(buf_no_whitespace, '(')) {
            char *close_label = strrchr(buf_no_whitespace, ')');
            if (close_label == NULL) {
                fprintf(stderr, "Error: Label at index %ld does not clsoe the label.\n", raw_file_idx);
                return 1;
            }
            *close_label = '\0';
            ;
            // TODO: What about if two labels are declared in succession? The first would point to the second? How does this behave? Do I need to handle that?
            ht_set(&buf_no_whitespace[1], total_rows, &ht);
            continue;
        }
        strcpy(raw_instr[total_rows], buf_no_whitespace);
        total_rows++;
    }

    int reg_idx = 16;
    for (size_t i=0; i < total_rows; i++) {
        if (raw_instr[i][0] == '@') {
            const char *instr = &raw_instr[i][1];
            struct entry *entry = ht_get(instr, &ht);
            char a_str[17];
            int a_value;
            char *end = NULL;
            if (entry != NULL) {
                a_value = entry->value;
                // TODO: Compute binary Address from entry->value
            } else if (strtol(instr, &end, 10))  {
                a_value = strtol(instr, &end, 10);
            } else {
                ht_set(instr, reg_idx, &ht);
                a_value = reg_idx;
                // TODO: Check if failed befor reg_idx can be upped. Though if failed. The whole program should fail so needs to be implemented too?
                reg_idx++;
            }
            dec_to_binary(a_value, a_str);
            printf(" A Instr=%s\n", a_str);
            printf("\n");
            
        } else {
            // [dest=]comp[;jump] -> dest and jump are optional
            char c_str[17];
            char dest_val[4] = "000";
            char comp_val[8];
            char jmp_val[4] = "000";
            char *next = &raw_instr[i][0];
            char *equal_sign = strchr(next, '=');
            if (equal_sign) {
                // TODO: Look up dest
                // Store dest or write it in the final c instr already
                // next = equal_sign + 1 
                // use strncpy maybe?
                //     snprintf(out, 17, "111%.7s%.3s%.3s", mid, sfxA, sfxB);

                *equal_sign = '\0';
                for (int i = 0; i < num_dest; i++) {
                    if (strcmp(dest[i].key, next) == 0) {
                        strcpy(dest_val, dest[i].value);
                    }
                }
                next = equal_sign + 1;
            }
            char *semicolon = strchr(next, ';');
            if (semicolon) {
                *semicolon = '\0';
                for (int i = 0; i < num_comp; i++) {
                    // TODO: If none is a hit, it s a invalid c instruction
                    if (strcmp(comp[i].key, next) == 0) {
                        strcpy(comp_val, comp[i].value);
                    }
                }
                next = semicolon + 1;
                for (int i = 0; i < num_jump; i++) {
                    if (strcmp(jump[i].key, next) == 0) {
                        strcpy(jmp_val, jump[i].value);
                    }
                }
            } else {
                for (int i = 0; i < num_comp; i++) {
                    // TODO: If none is a hit, it s a invalid c instruction
                    if (strcmp(comp[i].key, next) == 0) {
                        strcpy(comp_val, comp[i].value);
                    }
                }
            }
            snprintf(c_str, sizeof(c_str), "111%s%s%s", comp_val, dest_val, jmp_val);
            printf("C Instr=%s\n", c_str);
        }
    }

    return 0;
}