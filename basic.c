#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lookup.h"
#include "instr_processing.h"

// TODO: Add END JMP
// TODO: Handle Variables
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

        // 2. A-Instruction
        // If @ set marker for A-Instruction, proceed.
        if (A_INSTR[uch]) {
            is_a = true;
            continue;
        }

        // Gather decimal value and compute A-Instruction. Write it into the hack file.
        if (is_a) {
            char *error = process_a(uch, &is_a, &first_line, hack_file);
            if (error == NULL)
                continue;
            else {
                fprintf(stderr, "Error: processing a instruction failed.");
                return 1;
            }
        }

        char *error = process_c(uch, &first_line, hack_file);
        if (error == NULL)
            continue;
        else {
                fprintf(stderr, "Error: processing c instruction failed.");
                return 1;
        }
        
    }

    fclose(hack_file);
    fclose(fptr);

    return 0;
}
