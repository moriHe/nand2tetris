#include "lookup.h"

bool COMMENT[256] = {
    [47] = true,    // / marks the start of a comment
};

bool WHITESPACE[256] = {
    [9]  = true,  // tab
    [11] = true,  // vertical tab
    [12] = true,  // form feed
    [32] = true,  // space
};

bool NEWLINE[256] = {
    [10] = true,  // newline (LF)
    [13] = true,  // carriage return
};

bool SEMICOLON[256] = {
    [59] = true,  // ; marks the beginning of jump
};

bool EQUAL_SIGN[256] = {
    [61] = true,  // = marks the beginning of comp
};

bool A_INSTR[256] = {
    [64] = true  // @ marks the beginning of an A-Instruction
};

struct asmb_binary_map dest_map[] = {
    /// null can not be mapped (000)
    {"M", "001"},
    {"D", "010"},
    {"MD", "011"},
    {"A", "100"},
    {"AM", "101"},
    {"AD", "110"},
    {"AMD", "111"},
};
const size_t dest_map_size = sizeof(dest_map) / sizeof(*dest_map);

struct asmb_binary_map jmp_map[] = {
    // null can not be mapped (000)
    {"JGT", "001"},
    {"JEQ", "010"},
    {"JGE", "011"},
    {"JLT", "100"},
    {"JNE", "101"},
    {"JLE", "110"},
    {"JMP", "111"},
};
const size_t jmp_map_size = sizeof(jmp_map) / sizeof(*jmp_map);

struct asmb_binary_map comp_map[] = {
    // i[0]=a, i[1]..i[6]=c1..c6
    {"0", "0101010"},
    {"1", "0111111"},
    {"-1", "0111010"},
    {"D", "0001100"},
    {"A", "0110000"},
    {"M", "1110000"}, 
    {"!D", "0001101"},
    {"!A", "0110001"},
    {"!M", "1110001"},
    {"-D", "0001111"},
    {"-A", "0110011"},
    {"-M", "1110011"},
    {"D+1", "0011111"},
    {"A+1", "0110111"},
    {"M+1", "1110111"},
    {"D-1", "0001110"},
    {"A-1", "0110010"},
    {"M-1", "1110010"},
    {"D+A", "0000010"},
    {"D+M", "1000010"},
    {"D-A", "0010011"},
    {"D-M", "1010011"},
    {"A-D", "0000111"},
    {"M-D", "1000111"},
    {"D&A", "0000000"},
    {"D&M", "1000000"},
    {"D|A", "0010101"},
    {"D|M", "1010101"},
};
const size_t comp_map_size = sizeof(comp_map) / sizeof(*comp_map);