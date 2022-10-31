# ece-4720-lab3

In this lab assignment, you will extend the instruction-level MIPS simulator that you have developed in
Lab 1. The MIPS simulator that you have developed in Lab 1 was unpipelined, namely, it could take one
instruction at a time and finish the execution within a (long) single cycle. In the pipelined version, there
are five stages where each stage takes one cycle (where the cycle time is about ~5x smaller than the
unpipelined version). As you may recall, these five pipeline stages are:
- Instruction Fetch (IF)
- Instruction Decode (ID) 
- Execute (EX)
- Memory Access (MEM) 
- Writeback (WB)

You will work on ALU and Load/Store instructions as a starting point, and exclude branch and jump
instructions from the scope of this lab assignment. You should build your pipelined instruction-level
MIPS simulator assuming that you do not have any branches and jumps in your code (these will reappear
for the next lab assignment, but for simplicity, we exclude them for now. However, if you finish your
lab earlier, you are welcome to add support for branches and jumps). The operations performed within
each of these five stages are explained below.

1. Instruction Fetch (IF): *************
IR <= Mem[PC] 
PC <= PC + 4

The instruction is fetched from memory into the instruction register (IR) by using the current program
counter (PC). The PC is then incremented by 4 to address the next instruction. IR is used to hold the
instruction (that is 32-bit) that will be needed in subsequent cycle during the instruction decode stage.

2. Instruction Decode (ID): *************
A <= REGS[rs]
B <= REGS[rt]
imm <= sign-extended immediate field of IR

In this stage, the instruction is decoded (i.e., opcode and operands are extracted), and the content
of the register file is read (rs and rt are the register specifiers that indicate which registers to
read from). The values read from register file are placed into two temporary registers called A and B.
The values stored in A and B will be used in upcoming cycles by other stages (e.g., EX, or MEM). The 
lower 16 bits of the IR are sign-extended to 32-bit and stored in temporary register called imm. The 
value stored in imm register will be used in the next stage (i.e., EX).

3. Execution (EX)
In this stage, we have an ALU that operates on the operands that were read in the previous stage. We 
can perform one of three functions depending on the instruction type.

i) Memory Reference (load/store): 
ALUOutput <= A + imm

ALU adds two operands to form the effective address and stores the result into register called ALUOutput.

ii) Register-register Operation ALUOutput <= A op B

ALU performs the operation specified by the instruction on the values stored in temporary registers A and
B and places the result into ALUOutput.

iii) Register-Immediate Operation ALUOutput <= A op imm

ALU performs the operation specified by the instruction on the value stored in temporary register A and
value in register imm and places the result into ALUOutput.

4. Memory Access (MEM):
for load: LMD <= MEM[ALUOutput] 
for store: MEM[ALUOutput] <= B

If the instruction is load, data is read from memory and stored in load memory data (LMD) register. If it
is store, then the value stored in register B is written into memory. The address of memory to be accessed
is the value computed in the previous stage and stored in ALUOutput register.

5. Writeback (WB)
for register-register instruction: REGS[rd] <= ALUOutput 
for register-immediate instruction: REGS[rt] <= ALUOutput 
for load instruction: REGS[rt] <= LMD

In this stage, we write the result back into the destination register in register file. The result may come
from LMD or ALUOutput depending on the instruction.

Notice that, the temporary registers that we used (i.e. IR, A, B, imm, ALUOpt, LMD) will be overwritten by 
the next instruction during the next cycle. Since these values have to be passed from one pipeline stage to
next, we utilize pipeline registers to do so. The pipeline registers carry both data and control signals from
one pipeline stage to another. Any value needed on a later pipeline stage should be placed in pipeline 
register and forwarded from one stage to another until it is no longer needed (i.e., not used in the upcoming
stages). Below is the pipelined datapath that you are going to build. For now, you can omit the branch related links and mux, and assume that PC is incremented by 4 at every cycle.

There are 4 pipeline registers and each of them has fields associated with proper temporary registers that
we mentioned above (i.e. IR, A, B, imm, ALUOpt, LMD). The pipeline register called IF/ID has the following
fields:

IF/ID.IR
IF/ID.PC

On the other hand, pipeline register called ID/EX has the following fields:
ID/EX.IR 
ID/EX.A 
ID/EX.B 
ID/EX.imm

Similarly, pipeline register called EX/MEM has the following fields:

EX/MEM.IR 
EX/MEM.A 
EX/MEM.B 
EX/MEM.ALUOutput

And finally, pipeline register called MEM/WB has the following fields:

MEM/WB.IR
MEM/WB.ALUOutput
MEM/WB.LMD
