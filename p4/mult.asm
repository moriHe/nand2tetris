// Product of two numbers

// Reset R2
@R2
M=0
// Exit program if one of the args is a negative number
@R0
D=M
@INVALIDNUMBER
D;JLE

@R1
D=M
@INVALIDNUMBER
D;JLE

(LOOP)
@R1
M=M-1
// Add r0 to r2 for each iteration of the loop
@R0
D=M
@R2
M=D+M

// Check break condition. If iterator-r1 is smaller 0 reenter loop
@R1
D=M
@LOOP
D;JGT

(INVALIDNUMBER)

(END)
@END
0;JMP


