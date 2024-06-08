#include <iostream>
#include <fstream>
#include <vector>

using namespace std;

enum opcodeslist{
    ADD,    // 0
    SUB,    // 1
    MUL,    // 2
    INC,    // 3
    AND,    // 4
    OR,     // 5
    XOR,    // 6
    NOT,    // 7
    SLLI,   // 8
    SRLI,   // 9
    LI,     // a
    LD,     // b
    ST,     // c
    JMP,    // d
    BEQZ,   // e
    HLT     // f
};

int main(){
    ifstream file_icache("input/ICache.txt");
    ifstream file_idcache("input/DCache.txt");
    ifstream file_rf("input/rf.txt");
    ofstream file_odcache("output/DCache.txt");
    ofstream file_output("output/Output.txt");

    vector<int> I$(256),RF(16),D$(256),reg_in_use(16,0);
    // Instruction Cache, Register file, Data Cache
    // reg_in_use[i] specifies how many instructions are currently writing to register[i]

    // take input
    int i=0;
    while(file_icache >> hex >> I$[i++]);
    i=0;
    while(file_idcache >> hex >> D$[i++]);
    i=0;
    while(file_rf >> hex >> RF[i++]);

    // variables for writing to output.txt
    int ins_exec=0, ins_arith=0, ins_log=0, ins_shift=0, ins_mem=0, ins_li=0, ins_cont=0, ins_halt=0;
    int stalls_ttl=0, stalls_data=0, stalls_cont=0;

    // PC(initialised to 0), A, B, ALUOutput, LMD as mentioned in PDF
    // B2, ALUOutput2 for the extended use after 2 steps
    int PC=0,A,B,B2,ALUOutput,ALUOutput2,LMD;
    int cycles=0;   // number of cycles
    int ins=1,IF=-1,ID=-1,EX=-1,MEM=-1,stall=0,branch=-1;
    // ins(bool) => new instruction to execute or not
    // below variables are defaulted to -1 specifying no instruction
    // IF => Instruction fetched
    // ID => Instruction Decoded
    // EX => Instruction Executed
    // MEM => Instruction whose Memory cycle is done

    // stall(bool) => whether some instruction is being stalled (data stall only).
    // branch => where to branch to for JMP or BEQZ statement, defaulted to -1.
    while(1){
        cycles++;
        stalls_data+=stall; // if stall = 1, cycle is stalled
        // Write-Back Block
        if(MEM!=-1){
            if((MEM>>12)<=LD){
                ins_exec++;
                int rd=((0x0f00)&(MEM));
                rd=(rd>>8);
                if((MEM>>12)<=LI) RF[rd]=((ALUOutput2)&(0xff));
                else if((MEM>>12)==LD) RF[rd]=LMD;
                reg_in_use[rd]--;   // instruction done writing to register[rd]
                // pass value into respective register
            }
        }else if(ins==0 && EX==-1 && ID==-1 && IF==-1) break;   // no instruction left in any block, break

        // Memory Access Block
        if(EX!=-1){
            ALUOutput2=ALUOutput;   // ALUOutput will be changed by next instruction, have to use in next step as well
            // Perform LD, ST instructions as stated in PDF
            if((EX>>12)==LD) LMD=D$[ALUOutput2];
            else if((EX>>12)==ST){
                ins_exec++;
                D$[ALUOutput2]=B2;
            }else if((EX>>12)==BEQZ || (EX>>12)==JMP){
                // for control instructions, pass the ALUOutput2 to PC
                ins_exec++;
                ins_cont++;
                PC=ALUOutput2;
                ins=1;
                // start instruction flow
            }
            MEM=EX; // pass instruction whose memory access is done
        }else MEM=-1;   // no instruction executed, none passed to memory

        // Instruction Execute Block
        if(ID!=-1){
            if((ID>>12)==LI){
                ALUOutput=((0x00ff)&(ID));  // imm(8)
                ins_li++;
            }else if((ID>>12)==LD || (ID>>12)==ST){
                ALUOutput=((0x000f)&(ID))+A;    // A + imm(4)
                ins_mem++;
            }else{
                switch((ID>>12)){
                    case ADD:
                    ins_arith++;
                    ALUOutput=A+B;
                    break;

                    case SUB:
                    ins_arith++;
                    ALUOutput=A-B;
                    break;

                    case MUL:
                    ins_arith++;
                    ALUOutput=A*B;
                    break;

                    case INC:
                    ins_arith++;
                    ALUOutput=A+1;
                    break;

                    case AND:
                    ins_log++;
                    ALUOutput=(A&B);
                    break;

                    case OR:
                    ins_log++;
                    ALUOutput=(A|B);
                    break;
                    
                    case XOR:
                    ins_log++;
                    ALUOutput=(A^B);
                    break;

                    case NOT:
                    ins_log++;
                    ALUOutput=(~A);
                    break;

                    case SLLI:
                    ins_shift++;
                    ALUOutput=(A<<((0x000f)&(ID)));
                    break;

                    case SRLI:
                    ins_shift++;
                    ALUOutput=(A>>((0x000f)&(ID)));
                    break;

                    case JMP:
                    ALUOutput=PC+2*(((0x0ff0)&(ID))>>4);
                    stalls_cont++;  // stall yet not lifted, instruction below will be stalled
                    break;

                    case BEQZ:
                    if(A==0) ALUOutput=PC+2*((0x00ff)&(ID));
                    else ALUOutput=PC;
                    stalls_cont++;  // stall yet not lifted, instruction below will be stalled
                    break;
                }
            }
            ALUOutput=((ALUOutput)&(0x00ff));
            // now ALUOutput has corresponding value for each instruction as specified in PDF
            if((ID>>12)!=BEQZ && (ID>>12)!=JMP && (ALUOutput>>7)==1) ALUOutput=((ALUOutput)|(0xffffff00));
            // PC is unsigned, so ALUOutput is unsigned for BEQZ and JMP statement but signed for others
            // ALUOutput is 32 bit here, but should be 8 bits, if signed bit is 1, all bits to it's left should be 1
            // now ALUOutput mimicks as of 8 bits
            B2=B;   // value of B, will be changed by next instruction, hence created another variable
            EX=ID; // pass instruction executed
        }else EX=-1;    // no instruction decoded, none is executed

        // Instruction Decode Block
        if(IF!=-1){
            int Opcode=(IF>>12);        // get Opcode for fetched instruction
            stall=0;        // if the instruction has to be data stalled
            // A = rs1 or r(for INC and NOT) or rs(for BEQZ), B = rs2 or rd(for ST) for given instruction set
            if(Opcode==INC || Opcode==BEQZ){    // INC or BEQZ
                A=((0x0f00)&(IF));
                A=A>>8;
                if(reg_in_use[A]) stall=1;    // if register is in use, stall
                A=RF[A];
            }else if(Opcode<=ST && Opcode!=LI){
                A=((0x00f0)&(IF));
                A=(A>>4);
                if(reg_in_use[A]) stall=1;    // if register is in use, stall
                A=RF[A];
                if(Opcode<=XOR){
                    B=((0x000f)&(IF));
                    if(reg_in_use[B]) stall=1;    // if register is in use, stall
                    B=RF[B];
                }else if(Opcode==ST){
                    B=((0x0f00)&(IF));
                    B=(B>>8);
                    if(reg_in_use[B]) stall=1;    // if register is in use, stall
                    B=RF[B];
                }
            }
            if(stall) ID=-1;    // if a locked register was being accessed, instruction not decoded successfully.
            else{
                if((IF>>12)<=LD){
                    reg_in_use[(((IF)&(0x0f00))>>8)]++;  // lock the register being written, if necessary
                }
                // if Instruction decoded successfully pass instruction decoded
                ID=IF;
                // now A and B has register values, if necessary
                if(Opcode==BEQZ || Opcode==JMP){    // control stall the pipeline by fetching no more instructions
                    ins=0;
                    stalls_cont++;  // instruction below is stalled
                }
            }

        }else ID=-1;    // if no instruction is fetched, none is decoded

        // Instruction Fetch Block
        if(ins){
            if(!stall){
                IF=(I$[PC]<<8)+(I$[PC+1]);  // fetch new instruction
                PC=PC+2;
                PC=((PC)&(0xff));
                if((IF>>12)==HLT){  // if halt, ins=0, no more instructions
                    ins_halt++;
                    ins_exec++;
                    ins=0;
                }
            }   // if pipeline is currently data stalled, we pass Instruction fetched earlier only for decoding
        }else IF=-1;    // if no instruction, pass default value to IF
    }
    cycles--;
    // write to file DCache.txt
    for(int i=0;i<256;i++) file_odcache << hex << (D$[i]>>4) << (D$[i]&0x0f) << "\n";
    stalls_ttl=stalls_data+stalls_cont;
    // write to file Output.txt
    file_output << "Total number of instructions executed        : " << ins_exec << "\n";
    file_output << "Number of instructions in each class     \n";
    file_output << "Arithmetic instructions                      : " << ins_arith << "\n";
    file_output << "Logical instructions                         : " << ins_log << "\n";
    file_output << "Shift instructions                           : " << ins_shift << "\n";
    file_output << "Memory instructions                          : " << ins_mem << "\n";
    file_output << "Load immediate instructions                  : " << ins_li << "\n";
    file_output << "Control instructions                         : " << ins_cont << "\n";
    file_output << "Halt instructions                            : " << ins_halt << "\n";
    file_output << "Cycles Per Instruction                       : " << double((1.0*cycles)/ins_exec) << "\n";
    file_output << "Total number of stalls                       : " << stalls_ttl << "\n";
    file_output << "Data stalls (RAW)                            : " << stalls_data << "\n";
    file_output << "Control stalls                               : "  << stalls_cont << "\n";
}