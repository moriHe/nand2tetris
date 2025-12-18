#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lookup.h"
#include "instr_processing.h"

// TODO: Handle Variables like @i (LOOP) @LOOP

int main(int argc, char *argv[]) {
    // Input validation Start
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
    // Input validation Start

    // Load input and output file Start
    FILE* fptr;
    fptr = fopen(filename, "r");
    if (fptr == NULL) {
        fprintf(stderr, "Error: file could not be opened.");
        return 1;
    }

    char filename_body[filename_length + 2];
    int prefix_len = filename_length - 3; // This includes the "." Cuts of "asm"
    memcpy(filename_body, filename, prefix_len);
    memcpy(filename_body + prefix_len, "hack", 5);
    
    FILE *hack_file = fopen(filename_body, "w");
    if (hack_file == NULL) {
        fprintf(stderr, "Error: Initializing hack file.");
        return 1;
    }
    // Load input and output file Start

    int ch;                     // Holds the current char from the input file.
    int first_line = 1;         // First line does not add a newline to the output file. 
    bool is_comment = false;    // Marks the beginning of a Comment (Identifier: /).
    bool is_a = false;          // Marks the beginning of an A-Instruction (Identifier: @).
    int line = 0;               // ++, when a new line was written to thhe hack file.
    while ((ch = fgetc(fptr)) != EOF) {

        unsigned char uch = (unsigned char)ch;        
        bool newline = NEWLINE[uch];
        if (COMMENT[uch]) {
            is_comment = true;
            continue;
        }

        if (is_comment && !newline) continue;

        // No continue, because there could be an instruction before the comment that needs to be processed.
        if (is_comment && newline) is_comment = false;

        if (A_INSTR[uch]) {
            is_a = true;
            continue;
        }

        // In each iteration, decides to add 'uch' as the next number of A's decimal value or write the A-Instruction into the output file.
        if (is_a) {
            char *error = process_loop_a(uch, &is_a, &first_line, hack_file, &line);
            if (error == NULL)
                continue;
            else {
                fprintf(stderr, "Error: processing a instruction failed.");
                return 1;
            }
        }

        // If is_a is false, a c instruction is processed
        // In each iteration, when newline writes to 
        char *error = process_loop_c(uch, &first_line, hack_file, &line);
        if (error == NULL)
            continue;
        else {
                fprintf(stderr, "Error: processing c instruction failed.");
                return 1;
        }
        
    }

    // Write last line of asm file into the hack file. 
    // Happens if the last line does not end with newline indicator (which is usually the case)
    if (is_a) {
        char *error = write_a(&is_a, &first_line, hack_file, &line);
        if (error != NULL) fprintf(stderr, "Error: processing a after loop");
    } else {
        char *error = write_c(&first_line, hack_file, &line);
        if (error != NULL) fprintf(stderr, "Error: processing c after loop");
    }

    char binary_addr[17] = "";
    get_binary_addr(binary_addr, line, 0, 0);
    char address[17] = "";
    get_address(address, binary_addr);
    add_line_break(&first_line, hack_file);
    fprintf(hack_file, "%s", address);
    add_line_break(&first_line, hack_file);
    fprintf(hack_file,"%s", "1110101010000111");
    printf("line=%d\n", line);
    fclose(hack_file);
    fclose(fptr);

    return 0;
}
