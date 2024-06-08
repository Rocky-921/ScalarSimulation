LI R2, 0, 0 //#This is the borrow
LI R3, 0, 4 //#This is the number of bytes in a
LI R4, 0, 0
LI R5, 0, 4
LI R11, 0, 8
LI R13, f, f
LD R6, R4, 0 //# Load operands   // START_LOOP
LD R7, R5, 0   
NOT R8, R6 //# Move a_byte into result
NOT R8, R8
SUB R8, R8, R7 //# subtract b_byte from result
SUB R8, R8, R2 //# subtract borrow from result
SUB R12, R6, R7
BEQZ R12, 1// # if a is not equal to b jump to mayover
JMP 0, 8 // mayover
ST R8, R11, 0 //# Store the result  // BACK
INC R4 //# Increment addresses to get next byte
INC R5
INC R11
ADD R3, R3, R13 //#Decrement loop variable
BEQZ R3, 1// # loop if s3 is not zero
JMP f, 0 // start_loop
HLT
SUB R14, R6, R7     // MAYOVER
SRLI R14, R14, 7
BEQZ R14, 1 //# if a<b jump to overflow
JMP 0, 2 // overflow
LI R2, 0, 0          //  # else borrow=0
JMP f, 2 // back
LI R2, 0, 1            // # borrow=1   OVERFLOW
JMP f, 0 //    back;             //# jump back
