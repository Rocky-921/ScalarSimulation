if [ $# -ne 2 ]
then
	echo "Usage: bash compile.sh file.S outICache.txt"
	exit
fi

# Note that the values of immediate operands must be in hexadecimal without 0x prefix

# Also immediate values must be given as separate hex digits
# Eg: if L1 is 0x9, it should be given as JMP 0, 9
# Eg: if L1 is 0x1e, it should be given as JMP 1, e
# Eg: for load immediate R0 with -15, you should use LI R0, f, 0
# all instructions must have newline at the end.

# instructions are of width 2 bytes and the first byte will have opcode and rd
# next byte will have rs1 and rs2 for arithmetic instructions, etc...

cat $1 | \
	sed '
	s/\/\/.*//;
	/^[ \t]*$/d;
	s/,/ /g;
	s/[ \t]\+/ /g;
	s/[rR]10/a/g;
	s/[rR]11/b/g;
	s/[rR]12/c/g;
	s/[rR]13/d/g;
	s/[rR]14/e/g;
	s/[rR]15/f/g;
	s/[rR]0/0/g;
	s/[rR]1/1/g;
	s/[rR]2/2/g;
	s/[rR]3/3/g;
	s/[rR]4/4/g;
	s/[rR]5/5/g;
	s/[rR]6/6/g;
	s/[rR]7/7/g;
	s/[rR]8/8/g;
	s/[rR]9/9/g;
	s/\(INC .\)/\1 0 0/g;
	s/\(NOT . .\)/\1 0/g;
	s/\(JMP . .\)/\1 0/g;
	s/\(HLT\)/\1 0 0 0/g;
	s/ADD/0/g;
	s/SUB/1/g;
	s/MUL/2/g;
	s/INC/3/g;
	s/AND/4/g;
	s/XOR/6/g;
	s/OR/5/g;
	s/NOT/7/g;
	s/SLLI/8/g;
	s/SRLI/9/g;
	s/LI/a/g;
	s/LD/b/g;
	s/ST/c/g;
	s/JMP/d/g;
	s/BEQZ/e/g;
	s/HLT/f/g;
	s/\(.\) \(.\) \(.\) \(.\) \?/\1\2\n\3\4/
	' > $2

lines=`wc -l $2 | cut -d" " -f1`
while [ $lines -lt 256 ]
do
	echo 00 >> $2
	lines=$(($lines+1))
done
