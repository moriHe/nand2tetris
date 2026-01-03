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


struct hash_resp ht_set(const char *key, struct hash_table *ht) {
    struct hash_resp h_resp = hash(key);

    if (h_resp.err != NULL)
        return h_resp;
    struct entry *entry =  malloc(sizeof(struct entry));
    size_t len = strlen(key);
    char *copy = malloc(len + 1);
    memcpy(copy, key, len + 1);
    entry->key = copy;
    entry->value = 2;
    entry->next = NULL;
    ht->entries[h_resp.value] = entry;

    return h_resp;
}

struct entry *ht_get(const char *key, struct hash_table *ht) {
    struct hash_resp h_resp = hash(key);
    if (h_resp.err != NULL) {
        printf("Error was not null.\n");
                return NULL;
    }
    printf("h_respvalue=%ld\n", h_resp.value);
    struct entry *entry = ht->entries[h_resp.value];
    
    return entry;
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
    struct hash_table ht = {.entries = calloc(TABLE_SIZE, sizeof(struct entry*))};

    if (ht.entries == NULL) {
        fprintf(stderr, "Error: Failed to alloc memory for hash table.");
        return 1;
    }

    // TODO Whitespace vorher noch entfernen
    for (size_t i = 0; i < total_rows; i++) {
        if (strchr(raw_instr[i], '(')) {
            char *close_label = strrchr(raw_instr[i], ')');
            if (close_label == NULL) {
                fprintf(stderr, "Error: Row at index %ld does not clsoe the label.\n", i);
                return 1;
            }

            if (raw_instr[i][1] == '\0') {
                fprintf(stderr, "Error: Empty label declaration.");
                return 1;
            }
            ht_set(raw_instr[i], &ht);
            // TODO: Make this in one loop to skip putting in array
        }
    }

    for (size_t i = 0; i < total_rows; i++) {
        if (strchr(raw_instr[i], '(')) {
            if (raw_instr[i][1] == '\0') {
                fprintf(stderr, "Error: Empty label declaration.");
                return 1;
            }
                struct entry *entry = ht_get(raw_instr[i], &ht);
                if (entry != NULL) {
                    printf("not null\n");
                    printf("value=%d, key=%s\n", entry->value, entry->key);
                }
        }
    }
    return 0;
}