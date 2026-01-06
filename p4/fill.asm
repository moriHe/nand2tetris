// Fill
(LISTENKDB)
// init color to white
@color
M=0

/// init loop iterator
@SCREEN
D=A
@i
M=D

// Listen to Keyboard. If M > 0 (a key is pressed), color changes to black
@KBD
D=M
@LOOPCOLOR
D;JEQ
@color
M=-1

// Start coloring loop
(LOOPCOLOR)
// Read color
@color
D=M
// Change memory at SCREEN[i] to -1 or 0 based on color M
@i
A=M
M=D

// i++
@i
M=M+1
D=M
// Check if Keyboard Address - iterator is greater 0. If yes, start next iteration of the loop
// i starts as address integer of SCREEN[0]. With each i++ it gets closer to the KBD addresses which
// comes right after SCREEN[n]
@KBD
D=A-D

@LOOPCOLOR
D;JGT

// If coloring loop is finished, restart the process
@LISTENKDB
0;JMP