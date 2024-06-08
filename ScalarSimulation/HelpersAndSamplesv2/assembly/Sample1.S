LI R9, 2, 0 
LI R0, 0, 0
LI R1, 0, 4
LI R2, 0, 2
LI R3, e, e
SUB R8, R9, R0
BEQZ R8, 0, 6     // If R8 == 0 Goto HLT
ST R3, R0, 0
ST R4, R0, 2
ST R5, R0, 1
ST R6, R0, 3
ADD R0, R0, R1
JMP f, 8           // JMP to SUB instruction
HLT
