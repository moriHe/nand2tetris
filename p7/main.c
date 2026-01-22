#include <stdio.h>
#include <string.h>
#include <stdbool.h>
/*
Project 7 of Nand2Tetris.
* Goal: 
Build a basic VM translator. Should implement:
arithmetical-logical commands
- add, sub, neg
- eq, gt, lt
- and, or, not
push/pop commands
- push segment i
- pop segment i

The VMTranslator consists of a Parser and a CodeWriter

* Segments
- constant
- local
- argument
- this
- that
- static
- temp
- pointer (not used in this project)

* VM mapping / Hack RAM
0 -> Stack Pointer | SP
1 -> Local Segment Pointer | LCL
2 -> Argument Segment Pointer | ARG
3 -> THIS segment Pointer | THIS
4 -> THAT segment Pointer | THAT
5 - 12 -> Temp segment | temp
13 - 15 -> General Purpose
16 - 255 -> static variables  | Each static variable i in the fiel Xxx.vm is translated into the assembly symbol Xxx.i
256 - 2047 -> Stack

Parser Implementation
See PDF 91 - also here the methods:
- constructor - arg input file / stream - returns nothing
- hasMoreLines - arg nil - returns boolean 
- advance - arg nil - returns nothing
- commandType - arg nil - returns the type of the current command C_ARITHMETIC, C_PUSH or C_POP, C_LABEL, C_GOTO, C_IF, C_FUNCTION, C_RETURN, C_CALL
- arg1 - arg nil - returns first argument of current command as string
- arg2 - arg nil - returns second argument of the current command as int ONLY if C_PUSH or C_POP (or C_FUNCTION, C_CALL but that s project 8)

CodeWriter Implementation
See PDF 95 onward
- constructor - output file/stream
- writeArithmetic - arg command(string) - writes to output file the assembly code that implements the given arithmetic-logical command
- writePushPop - arg command C_PUSH or C_POP - writes push or pop command to out
- close Closes the output file

VMTranslator
Main - Drives the Parser and CodeWriter
*/

struct Parser {
    FILE *input_file;
    char *current_command;
    int current_index;
};

void advance(struct Parser *parser) {
    char *next = malloc(4096);
    char *tmp = NULL;
    while (tmp == NULL) {
        fgets(next, sizeof next, parser->input_file);
        tmp = next;
        char *comment_start = strstr(tmp, "//");
        if (comment_start)
            *comment_start = '\0';

        // TODO: Trim whitespace left and right

        if (tmp[0] == '\0') {
            tmp = NULL;
            continue;
        }
    }
    printf("next=%s\n", next);
    parser->current_command = next;
    // Advance index
    parser->current_index++;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Error: Expected 1 arg (file path), got %d instead.\n", argc);
        return 1;
    }
    
    const char *iptr = argv[1];
    char *ending = strrchr(iptr, '.');
    if (strcmp(ending, ".vm") != 0) {
        fprintf(stderr, "Error: Wrong input format. Expected .vm, got %s instead\n", ending);
        return 1;
    }

    // Start extract root file name for later use as static variable label
    char *filename_identifier = strrchr(iptr, '/');
    // If null, the arg is probably Basic.vm instead of ./Basic.vm. filename_identifier points to /. So we only need to advance if present.
    if (filename_identifier != NULL) {
        filename_identifier++;
    }
    
    size_t filename_identifier_len = strlen(filename_identifier);
    size_t cutoff = 3; // cutting off .vm
    size_t file_root_n = filename_identifier_len - cutoff + 1;    // + 1 for \0
    char file_root[file_root_n];
    strncpy(file_root, filename_identifier, filename_identifier_len - cutoff);
    file_root[file_root_n - 1] = '\0';
    // End extract root file name for later use as static variable label
    printf("input pointer = %s\n", iptr);
    printf("file_identifier=%s\n", filename_identifier);
    printf("file_stem=%s\n", file_root);
    
    return 0;
}