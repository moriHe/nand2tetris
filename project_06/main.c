#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "lookup.h"
#include "instr_processing.h"
#include "../hash.h"

// TODO: Handle Variables like @i (LOOP) @LOOP

const char *ensure_capacity(size_t *capacity, size_t n, char ***asm_parsed) {
    if (n < *capacity) 
        return NULL;

    size_t newcap = *capacity == 0 ? 8 : *capacity * 2;
    char **tmp_asm_parsed = realloc(*asm_parsed, newcap * sizeof **asm_parsed);
    if (!tmp_asm_parsed) 
        return "Error: Realloc asm_parsed.";
    *asm_parsed = tmp_asm_parsed;
    *capacity = newcap;

    return NULL;
}

void remove_comments(char *line) {
    char *sep = strstr(line, "//");
    if (sep)
        *sep = '\0';
    char *w = line;
    for (char *p = line; *p; p++) {
        if (!isspace((unsigned char) *p)) {
            *w++ = *p;
        }
    }
    *w = '\0';
}


int main(int argc, char *argv[]) {
    // Input validation
    if (argc < 2) {
        fprintf(stderr, "Error: no filename provided.\n");
        return 1;
    }

    char *filename = argv[1];
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

    // Load input and output file
    FILE* fptr;
    fptr = fopen(filename, "r");
    if (fptr == NULL) {
        fprintf(stderr, "Error: file could not be opened.");
        return 1;
    }

    char **asm_parsed = NULL;
    char *line = NULL;
    size_t cap = 0;
    size_t n = 0;
    size_t capacity = 0;
    
    while (getline(&line, &cap, fptr) != -1) {
        const char *err = ensure_capacity(&capacity, n + 2, &asm_parsed);
        if (err) {
            fprintf(stderr, "%s\n", err);
            return 1;
        }
        
        remove_comments(line);

        // Empty line do nothing.
        if (line[0] == '\0')
            continue;

        // Persist asm line
        asm_parsed[n] = strdup(line);
        if (!asm_parsed[n]) {
            fprintf(stderr, "Error: Insert line asm_parsed.");
            return 1;
        }

        n++;
        asm_parsed[n] = NULL;
    }

    free(line);

    int write = 0;
    for (int read = 0; asm_parsed[read]; read++) {
        char *line = asm_parsed[read];
        if (line[0] == '(') {
            char *end_label = strchr(line + 1, ')');
            if (end_label) {
                *end_label = '\0';
                const char *err = ht_insert(line + 1, write);
                if (err) {
                    fprintf(stderr, "Error: Failed to ht_insert, err=%s\n", err);
                    return 1;
                }
            } else {
                fprintf(stderr, "Error: Label was not properly closed.");
                return 1;
            }
        } else {
            asm_parsed[write] = asm_parsed[read];
            write++;
        }
        
    }
    asm_parsed[write] = NULL;
    n = write;

    for (size_t i = 0; asm_parsed[i] != NULL; i++) {
        printf("i=%zu, asm_parsed=%s\n", i, asm_parsed[i]);
        char *instr = asm_parsed[i];
        if (instr[0] == '@') {
            if (instr[1] == '\0') {
                fprintf(stderr, "Error: Invalid A-Instruction.");
                return 1;
            }
            

        }
        // is a instruction? 
            // is decimal?
            // is label?
            // is variable (start at @16)
        // is c instruction?
    }


    for (size_t i = 0; i < n; i++) 
        free(asm_parsed[i]);
    free(asm_parsed);


    fclose(fptr);

    return 0;
}
