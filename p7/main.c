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
int main(int argc, char *argv[]) {

    return 0;
}