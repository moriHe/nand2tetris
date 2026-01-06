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

void dec_to_binary(int dec) {
    if (dec < 0 || dec > 32767) {
        // TODO: Handle Error
        return;
    }
    int binary[15] = {0};

    int i = 0;
    while (dec > 0) {
        binary[i] = dec % 2;
        dec = dec / 2;
        i++;
    }
    char binary_str[17];
    binary_str[16] = '\0';
    binary_str[0] = '0'; // Identifies row as A-Instruction
    for (int j = 15; j > 0; j--)
        binary_str[j] = (char)binary[j];
    
    printf("binary_str=%s\n", binary_str);
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
    dec_to_binary(20);
    printf("total_rows=%zu\n", total_rows);
    int reg_idx = 0;
    for (size_t i=0; i < total_rows; i++) {
        printf("row=%s -> ", raw_instr[i]);
        if (raw_instr[i][0] == '@') {
            printf("A-Address, value=%s, ", &raw_instr[i][1]);
            const char *instr = &raw_instr[i][1];
            struct entry *entry = ht_get(instr, &ht);
            char *end = NULL;
            if (entry != NULL) {
                printf("It s a label, key=%s, value=%d", entry->key, entry->value);
                // TODO: Compute binary Address from entry->value
            } else if (strtol(instr, &end, 10))  {
                printf("I'm a number.");
                // TODO: Compute binary Address from isntr
            } else {
                printf("I'm a variable");
                // TODO: Variable. Check if it is hashed. If yes, get the entry->value
                // If not: invoke ht_set. The value starts with the decimal 16 for the first Register in Nand2Tetris.
                // Use that decimal to compute Address. Increment decimal for the next occuring variable
            }
            printf("\n");
            
        } else {
            printf("C-Address\n");
        }
    }

    return 0;
}