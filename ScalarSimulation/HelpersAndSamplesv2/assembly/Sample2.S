LI R0, 0, 0
LI R2, 0, a
LI R3, 0, 2
LI R4, 0, 0
LI R5, 0, 0

LI R10, 5, 0

SUB R15, R2, R0
BEQZ R15, 0, 6     // If R15 == 0 Goto HLT
LD R1, R0, 1
ADD R4, R4, R1
LD R1, R0, 0
ADD R5, R5, R1
ADD R0, R0, R3
JMP f, 8           // JMP to SUB instruction
ST R5, R10, 0
ST R4, R10, 1
HLT
