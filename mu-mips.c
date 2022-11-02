#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

#include "mu-mips.h"

/***************************************************************/
/* Print out a list of commands available                                                                  */
/***************************************************************/
void help() {        
	printf("------------------------------------------------------------------\n\n");
	printf("\t**********MU-MIPS Help MENU**********\n\n");
	printf("sim\t-- simulate program to completion \n");
	printf("run <n>\t-- simulate program for <n> instructions\n");
	printf("rdump\t-- dump register values\n");
	printf("reset\t-- clears all registers/memory and re-loads the program\n");
	printf("input <reg> <val>\t-- set GPR <reg> to <val>\n");
	printf("mdump <start> <stop>\t-- dump memory from <start> to <stop> address\n");
	printf("high <val>\t-- set the HI register to <val>\n");
	printf("low <val>\t-- set the LO register to <val>\n");
	printf("print\t-- print the program loaded into memory\n");
	printf("show\t-- print the current content of the pipeline registers\n");
	printf("?\t-- display help menu\n");
	printf("quit\t-- exit the simulator\n\n");
	printf("------------------------------------------------------------------\n\n");
}

/***************************************************************/
/* Read a 32-bit word from memory                                                                            */
/***************************************************************/
uint32_t mem_read_32(uint32_t address)
{
	int i;
	for (i = 0; i < NUM_MEM_REGION; i++) {
		if ( (address >= MEM_REGIONS[i].begin) &&  ( address <= MEM_REGIONS[i].end) ) {
			uint32_t offset = address - MEM_REGIONS[i].begin;
			return (MEM_REGIONS[i].mem[offset+3] << 24) |
					(MEM_REGIONS[i].mem[offset+2] << 16) |
					(MEM_REGIONS[i].mem[offset+1] <<  8) |
					(MEM_REGIONS[i].mem[offset+0] <<  0);
		}
	}
	return 0;
}

/***************************************************************/
/* Write a 32-bit word to memory                                                                                */
/***************************************************************/
void mem_write_32(uint32_t address, uint32_t value)
{
	int i;
	uint32_t offset;
	for (i = 0; i < NUM_MEM_REGION; i++) {
		if ( (address >= MEM_REGIONS[i].begin) && (address <= MEM_REGIONS[i].end) ) {
			offset = address - MEM_REGIONS[i].begin;

			MEM_REGIONS[i].mem[offset+3] = (value >> 24) & 0xFF;
			MEM_REGIONS[i].mem[offset+2] = (value >> 16) & 0xFF;
			MEM_REGIONS[i].mem[offset+1] = (value >>  8) & 0xFF;
			MEM_REGIONS[i].mem[offset+0] = (value >>  0) & 0xFF;
		}
	}
}

/***************************************************************/
/* Execute one cycle                                                                                                              */
/***************************************************************/
void cycle() {                                                
	handle_pipeline();
	CURRENT_STATE = NEXT_STATE;
	CYCLE_COUNT++;
}

/***************************************************************/
/* Simulate MIPS for n cycles                                                                                       */
/***************************************************************/
void run(int num_cycles) {                                      
	
	if (RUN_FLAG == FALSE) {
		printf("Simulation Stopped\n\n");
		return;
	}

	printf("Running simulator for %d cycles...\n\n", num_cycles);
	int i;
	for (i = 0; i < num_cycles; i++) {
		if (RUN_FLAG == FALSE) {
			printf("Simulation Stopped.\n\n");
			break;
		}
		cycle();
	}
}

/***************************************************************/
/* simulate to completion                                                                                               */
/***************************************************************/
void runAll() {                                                     
	if (RUN_FLAG == FALSE) {
		printf("Simulation Stopped.\n\n");
		return;
	}

	printf("Simulation Started...\n\n");
	while (RUN_FLAG){
		cycle();
	}
	printf("Simulation Finished.\n\n");
}

/***************************************************************/ 
/* Dump a word-aligned region of memory to the terminal                              */
/***************************************************************/
void mdump(uint32_t start, uint32_t stop) {          
	uint32_t address;

	printf("-------------------------------------------------------------\n");
	printf("Memory content [0x%08x..0x%08x] :\n", start, stop);
	printf("-------------------------------------------------------------\n");
	printf("\t[Address in Hex (Dec) ]\t[Value]\n");
	for (address = start; address <= stop; address += 4){
		printf("\t0x%08x (%d) :\t0x%08x\n", address, address, mem_read_32(address));
	}
	printf("\n");
}

/***************************************************************/
/* Dump current values of registers to the teminal                                              */   
/***************************************************************/
void rdump() {                               
	int i; 
	printf("-------------------------------------\n");
	printf("Dumping Register Content\n");
	printf("-------------------------------------\n");
	printf("# Instructions Executed\t: %u\n", INSTRUCTION_COUNT);
	printf("# Cycles Executed\t: %u\n", CYCLE_COUNT);
	printf("PC\t: 0x%08x\n", CURRENT_STATE.PC);
	printf("-------------------------------------\n");
	printf("[Register]\t[Value]\n");
	printf("-------------------------------------\n");
	for (i = 0; i < MIPS_REGS; i++){
		printf("[R%d]\t: 0x%08x\n", i, CURRENT_STATE.REGS[i]);
	}
	printf("-------------------------------------\n");
	printf("[HI]\t: 0x%08x\n", CURRENT_STATE.HI);
	printf("[LO]\t: 0x%08x\n", CURRENT_STATE.LO);
	printf("-------------------------------------\n");
}

/***************************************************************/
/* Read a command from standard input.                                                               */  
/***************************************************************/
void handle_command() {                         
	char buffer[20];
	uint32_t start, stop, cycles;
	uint32_t register_no;
	int register_value;
	int hi_reg_value, lo_reg_value;

	printf("MU-MIPS SIM:> ");

	if (scanf("%s", buffer) == EOF){
		exit(0);
	}

	switch(buffer[0]) {
		case 'S':
		case 's':
			if (buffer[1] == 'h' || buffer[1] == 'H'){
				show_pipeline();
			}else {
				runAll(); 
			}
			break;
		case 'M':
		case 'm':
			if (scanf("%x %x", &start, &stop) != 2){
				break;
			}
			mdump(start, stop);
			break;
		case '?':
			help();
			break;
		case 'Q':
		case 'q':
			printf("**************************\n");
			printf("Exiting MU-MIPS! Good Bye...\n");
			printf("**************************\n");
			exit(0);
		case 'R':
		case 'r':
			if (buffer[1] == 'd' || buffer[1] == 'D'){
				rdump();
			}else if(buffer[1] == 'e' || buffer[1] == 'E'){
				reset();
			}
			else {
				if (scanf("%d", &cycles) != 1) {
					break;
				}
				run(cycles);
			}
			break;
		case 'I':
		case 'i':
			if (scanf("%u %i", &register_no, &register_value) != 2){
				break;
			}
			CURRENT_STATE.REGS[register_no] = register_value;
			NEXT_STATE.REGS[register_no] = register_value;
			break;
		case 'H':
		case 'h':
			if (scanf("%i", &hi_reg_value) != 1){
				break;
			}
			CURRENT_STATE.HI = hi_reg_value; 
			NEXT_STATE.HI = hi_reg_value; 
			break;
		case 'L':
		case 'l':
			if (scanf("%i", &lo_reg_value) != 1){
				break;
			}
			CURRENT_STATE.LO = lo_reg_value;
			NEXT_STATE.LO = lo_reg_value;
			break;
		case 'P':
		case 'p':
			print_program(); 
			break;
		default:
			printf("Invalid Command.\n");
			break;
	}
}

/***************************************************************/
/* reset registers/memory and reload program                                                    */
/***************************************************************/
void reset() {   
	int i;
	/*reset registers*/
	for (i = 0; i < MIPS_REGS; i++){
		CURRENT_STATE.REGS[i] = 0;
	}
	CURRENT_STATE.HI = 0;
	CURRENT_STATE.LO = 0;
	
	for (i = 0; i < NUM_MEM_REGION; i++) {
		uint32_t region_size = MEM_REGIONS[i].end - MEM_REGIONS[i].begin + 1;
		memset(MEM_REGIONS[i].mem, 0, region_size);
	}
	
	/*load program*/
	load_program();
	
	/*reset PC*/
	INSTRUCTION_COUNT = 0;
	CURRENT_STATE.PC =  MEM_TEXT_BEGIN;
	NEXT_STATE = CURRENT_STATE;
	RUN_FLAG = TRUE;
}

/***************************************************************/
/* Allocate and set memory to zero                                                                            */
/***************************************************************/
void init_memory() {                                           
	int i;
	for (i = 0; i < NUM_MEM_REGION; i++) {
		uint32_t region_size = MEM_REGIONS[i].end - MEM_REGIONS[i].begin + 1;
		MEM_REGIONS[i].mem = malloc(region_size);
		memset(MEM_REGIONS[i].mem, 0, region_size);
	}
}

/**************************************************************/
/* load program into memory                                                                                      */
/**************************************************************/
void load_program() {                   
	FILE * fp;
	int i, word;
	uint32_t address;

	/* Open program file. */
	fp = fopen(prog_file, "r");
	if (fp == NULL) {
		printf("Error: Can't open program file %s\n", prog_file);
		exit(-1);
	}

	/* Read in the program. */

	i = 0;
	while( fscanf(fp, "%x\n", &word) != EOF ) {
		address = MEM_TEXT_BEGIN + i;
		mem_write_32(address, word);
		printf("writing 0x%08x into address 0x%08x (%d)\n", word, address, address);
		i += 4;
	}
	PROGRAM_SIZE = i/4;
	printf("Program loaded into memory.\n%d words written into memory.\n\n", PROGRAM_SIZE);
	fclose(fp);
}

/************************************************************/
/* maintain the pipeline                                                                                           */ 
/************************************************************/
void handle_pipeline()
{
	/*INSTRUCTION_COUNT should be incremented when instruction is done*/
	/*Since we do not have branch/jump instructions, INSTRUCTION_COUNT should be incremented in WB stage */
	
	WB();
	MEM();
	EX();
	ID();
	IF();
}

/************************************************************/
/* writeback (WB) pipeline stage:                                                                          */ 
/************************************************************/
void WB()
{
	uint32_t opcode;
	uint32_t funct;	
	uint32_t rt;
	uint32_t rd;

	WB_decode_operands(MEM_WB.IR, &rt, &rd, &opcode, &funct);

	WB_populate_destination(MEM_WB.IR, rt, rd, opcode, funct);
	
	INSTRUCTION_COUNT++;
}

/************************************************************/
/* memory access (MEM) pipeline stage:                                                          */ 
/************************************************************/
void MEM()
{
	uint32_t opcode;
	uint32_t funct;
	uint32_t addr;
	
	// Get the current instruction
	MEM_WB.IR = EX_MEM.IR;

	MEM_decode_operands(ID_EX.IR, &opcode, &funct, &addr);

	// Perform the current memory operation
	MEM_access(MEM_WB.IR, opcode, funct, addr);
}

/************************************************************/
/* execution (EX) pipeline stage:                                                                          */ 
/************************************************************/
void EX()
{
	// Local vars
	uint32_t opcode;
	uint32_t shamt;
	uint32_t funct;
	uint32_t addr;
	
	// Get the current instruction 
	EX_MEM.IR = ID_EX.IR;

	EX_decode_operands(ID_EX.IR, &opcode, &shamt, &funct, &addr);

	// Perform the current operation and store the values
	EX_perform_operation(EX_MEM.IR, ID_EX.A, ID_EX.B, ID_EX.imm, opcode, shamt, funct, addr);

	return;
}

/************************************************************/
/* instruction decode (ID) pipeline stage:                                                         */ 
/************************************************************/
void ID()
{
	uint32_t rs;
	uint32_t rt;
	uint32_t immediate;
	
	// Get the current instruction
	ID_EX.IR = IF_ID.IR;

	// Decode the registers in the instruction
	ID_decode_operands(ID_EX.IR, &rs, &rt, &immediate);

	// ID/EX.A <= REGS[ IF/ID.IR[rs] ]
	ID_EX.A = CURRENT_STATE.REGS[rs];

	// ID/EX.B <= REGS[ IF/ID.IR[rt] ]
	ID_EX.B = CURRENT_STATE.REGS[rt];

	// ID/EX.imm <= sign-extend( IF/ID.IR[imm. Field] )
	ID_EX.imm = immediate;

	return;
}

/************************************************************/
/* instruction fetch (IF) pipeline stage:                                                              */ 
/************************************************************/
void IF()
{
	// IR <= Mem[PC] 
	IF_ID.IR = mem_read_32(CURRENT_STATE.PC);

	// PC <= PC + 4
	IF_ID.PC = CURRENT_STATE.PC + 4; // next state or current?
}

void decode_all_operands(uint32_t instruction,
				uint32_t* opcode,
				uint32_t* rs, 
				uint32_t* rt, 
				uint32_t* rd, 
				uint32_t* shamt, 
				uint32_t* funct, 
				uint32_t* immediate,
				uint32_t* address) {
	uint32_t temp;
	temp = instruction;
	temp >>= 26;
	*opcode = temp;
	
	temp = instruction;
	temp <<= 6;
	temp >>= 27;
	*rs = temp;

	temp = instruction;
	temp <<= 11;
	temp >>= 27;
	*rt = temp;

	temp = instruction;
	temp <<= 16;
	temp >>= 27;
	*rd = temp;

	temp = instruction;
	temp <<= 21;
	temp >>= 27;
	*shamt = temp;	

	temp = instruction;
	temp <<= 26;
	temp >>= 26;
	*funct = temp;

	temp = instruction;
	temp <<= 16;
	temp >>= 16;
	*immediate = temp;	

	temp = instruction;
	temp <<= 6;
	temp >>= 6;
	*address = temp;
}

/**************************************************************/
/* Isolates portions of 32-bit binary instruction to determine operands                                  */
/* needed for Decode stage of pipeline       								                             */
/**************************************************************/
void ID_decode_operands(uint32_t instruction,
				uint32_t* rs, 
				uint32_t* rt, 
				uint32_t* immediate)
		{
	uint32_t temp;
	temp = instruction;
	temp <<= 6;
	temp >>= 27;
	*rs = temp;

	temp = instruction;
	temp <<= 11;
	temp >>= 27;
	*rt = temp;

	temp = instruction;
	temp <<= 16;
	temp >>= 16;
	*immediate = temp;		
}

/**************************************************************/
/* Isolates portions of 32-bit binary instruction to determine operands                                  */
/* needed for Execute stage of pipeline       								                             */
/**************************************************************/
void EX_decode_operands(uint32_t instruction,
				uint32_t* opcode, 
				uint32_t* shamt, 
				uint32_t* funct,
				uint32_t* addr) {
	uint32_t temp;
	temp = instruction;
	temp >>= 26;
	*opcode = temp;

	temp = instruction;
	temp <<= 21;
	temp >>= 27;
	*shamt = temp;	

	temp = instruction;
	temp <<= 26;
	temp >>= 26;
	*funct = temp;

	temp = instruction;
	temp <<= 6;
	temp >>= 6;
	*addr = temp;		
}

/**************************************************************/
/* Isolates portions of 32-bit binary instruction to determine operands                                  */
/* needed for Memory stage of pipeline       								                             */
/**************************************************************/
void MEM_decode_operands(uint32_t instruction,
				uint32_t* opcode, 
				uint32_t* funct,
				uint32_t* address) {
	uint32_t temp;
	temp = instruction;
	temp >>= 26;
	*opcode = temp;

	temp = instruction;
	temp <<= 26;
	temp >>= 26;
	*funct = temp;

	temp = instruction;
	temp <<= 6;
	temp >>= 6;
	*address = temp;
}

/**************************************************************/
/* Isolates portions of 32-bit binary instruction to determine operands                                  */
/* needed for Write Back stage of pipeline       								                         */
/**************************************************************/
void WB_decode_operands(uint32_t instruction,
				uint32_t* rt,
				uint32_t* rd,
				uint32_t* opcode, 
				uint32_t* funct) {
	uint32_t temp;
	temp = instruction;
	temp >>= 26;
	*opcode = temp;

	temp = instruction;
	temp <<= 11;
	temp >>= 27;
	*rt = temp;
	
	temp = instruction;
	temp <<= 16;
	temp >>= 27;
	*rd = temp;

	temp = instruction;
	temp <<= 26;
	temp >>= 26;
	*funct = temp;
}

void decode_machine_register(uint32_t reg, char* buffer) {
	switch (reg) {
		// Constant value zero
		case(0):
			strcpy(buffer, "$zero");
		break;
		
		// Assembler temporary
		case(1):
			strcpy(buffer, "$at");
		break;

		// Function results and expression eval 2-3
		case(2):
			strcpy(buffer, "$v0");
		break;

		case(3):
			strcpy(buffer, "$v1");
		break;	

		// Arguments 4-7
		case(4):
			strcpy(buffer, "$a0");
		break;

		case(5):
			strcpy(buffer, "$a1");
		break;

		case(6):
			strcpy(buffer, "$a2");
		break;

		case(7):
			strcpy(buffer, "$a3");
		break;

		// Temporaries 8-15
		case(8):
			strcpy(buffer, "$t0");
		break;

		case(9):
			strcpy(buffer, "$t1");
		break;	

		case(10):
			strcpy(buffer, "$t2");
		break;	

		case(11):
			strcpy(buffer, "$t3");
		break;	

		case(12):
			strcpy(buffer, "$t4");
		break;	

		case(13):
			strcpy(buffer, "$t5");
		break;	

		case(14):
			strcpy(buffer, "$t6");
		break;	

		case(15):
			strcpy(buffer, "$t7");
		break;	

		// Saved temporaries 16-23
		case(16):
			strcpy(buffer, "$s0");
		break;	

		case(17):
			strcpy(buffer, "$s1");
		break;	

		case(18):
			strcpy(buffer, "$s2");
		break;	

		case(19):
			strcpy(buffer, "$s3");
		break;	

		case(20):
			strcpy(buffer, "$s4");
		break;	

		case(21):
			strcpy(buffer, "$s5");
		break;	

		case(22):
			strcpy(buffer, "$s6");
		break;	

		case(23):
			strcpy(buffer, "$s7");
		break;	

		// Temporaries 24-25
		case(24):
			strcpy(buffer, "$t8");
		break;	

		case(25):
			strcpy(buffer, "$t9");
		break;	

		// Reserved for OS kernel
		case(26):
			strcpy(buffer, "$k0");
		break;	

		case(27):
			strcpy(buffer, "$k1");
		break;	

		// Global pointer
		case(28):
			strcpy(buffer, "$gp");
		break;	

		// Stack pointer
		case(29):
			strcpy(buffer, "$sp");
		break;	

		// Frame pointer
		case(30):
			strcpy(buffer, "$fp");
		break;	
		
		// Return address
		case(31):
			strcpy(buffer, "$ra");
		break;

		default:
			printf("Error[decode_machine_register]: Register not found\n");
	}
	return;
}

/**************************************************************/
/* Performs the operation and stores result to be used in WB/MEM stage                                */
/**************************************************************/
void EX_perform_operation(uint32_t instruction,
				uint32_t A,
				uint32_t B,
				uint32_t imm,
				uint32_t opcode,
				uint32_t shamt,
				uint32_t funct,
				uint32_t address) {

	switch (opcode) {
		// R-format
		case (0x0):
			switch (funct) {
				case (0x20): // add
					EX_MEM.ALUOutput = A + B;
				break;

				case (0x21): // addu
					EX_MEM.ALUOutput = A + B;
				break;

				case (0x22): // sub
					EX_MEM.ALUOutput = A - B;
				break;

				case (0x23): // subu
					EX_MEM.ALUOutput = A - B;
				break;

				case (0x24): // and
					EX_MEM.ALUOutput = (A && B);
				break;

				case (0x25): // or
					EX_MEM.ALUOutput = (A || B);
				break;

				case (0x26): // xor	
					EX_MEM.ALUOutput = (A ^ B);
				break;

				case (0x27): // nor	
					EX_MEM.ALUOutput = ~(A | B);
					break;

				case (0x18): // mult
				case (0x19): { // multu
					uint64_t result = (A * B);
					NEXT_STATE.HI = (result >> 32);
					NEXT_STATE.LO = ((result << 32) >> 32);
				break; }

				case (0x1A): { // div
				case (0x1B): // divu
					NEXT_STATE.LO = (A / B);
					NEXT_STATE.HI = (A % B);
				break; }

				case (0x2A): // slt
					EX_MEM.ALUOutput = (A < B) ? 1 : 0;
				break;

				case (0x00): // sll
					EX_MEM.ALUOutput = B << shamt;
				break;

				case (0x02): // srl
					EX_MEM.ALUOutput = B >> shamt;
				break;

				case (0x3):  // sra
					EX_MEM.ALUOutput = B >> shamt;
				break;

				case (0x9):  // jalr	
					// Do this later
				break;

				case (0x10): // mfhi
				case (0x12): // mflo
				break;

				case (0x11): // mthi
					EX_MEM.ALUOutput = A;
				break;

				case (0x13): // mtlo
					EX_MEM.ALUOutput = A;
				break;

				case (0x8):  // jr
					// Do this later
				break;

				case (0xC):  // syscall
				break;

				default:
					printf("ERROR[EX_perform_operation]: Invalid instruction (R)\n");
				break;
			}
		break;


		// J-format
		case (0x2): // j
			// Do this later
		break;

		case (0x3): // jal
			// Do this later
		break;

		
		// I-format
		case (0x8):  // addi
			EX_MEM.ALUOutput = A + imm;
		break;

		case (0x9):  // addiu
			EX_MEM.ALUOutput = A + imm;
		break;

		case (0xD):  // ori
			EX_MEM.ALUOutput = (A || imm);
		break;

		case (0xE):  // xori
			EX_MEM.ALUOutput = (A ^ imm);
		break;

		case (0xA):  // slti
			EX_MEM.ALUOutput = (A < imm) ? 1 : 0;
		break;

		case (0x23): // lw
			EX_MEM.ALUOutput = A + imm;
			EX_MEM.B = ID_EX.B;
		break;

		case (0x32): // lb
			EX_MEM.ALUOutput = A + imm;
			EX_MEM.B = ID_EX.B;
		break;

		case (0x36): // lh		
			EX_MEM.ALUOutput = A + imm;
			EX_MEM.B = ID_EX.B;
		break;

		case (0xF):  // lui
			EX_MEM.ALUOutput = (imm << 16);
			EX_MEM.B = ID_EX.B;
		break;

		case (0x2B): // sw
			EX_MEM.ALUOutput = A + imm;
			EX_MEM.B = ID_EX.B;
		break;

		case (0x28): // sb
			EX_MEM.ALUOutput = A + imm;
			EX_MEM.B = ID_EX.B;
		break;

		case (0x29): // sh 
			EX_MEM.ALUOutput = A + imm;
			EX_MEM.B = ID_EX.B;
		break;

		case (0x1):  // bltz or bgez
			// Do this later
		break;

		case (0x4):  // beq
			// Do this later
		break;

		case (0x5):  // bne
			// Do this later
		break;

		case (0x6):  // blez
			// Do this later
		break;

		case (0x7):  // bgtz
			// Do this later
		break;

		default:
			printf("Error[EX_perform_operation]: Invalid instruction\n");
		break;
	}

	return;
}

/**************************************************************/
/* Stores computed result in memory                    				                                */
/**************************************************************/
void MEM_access(uint32_t instruction,
				uint32_t opcode,
				uint32_t funct,
				uint32_t address) {

	MEM_WB.ALUOutput = EX_MEM.ALUOutput;
	
	switch (opcode) {
		// R-format
		case (0x0):
		break;


		// J-format
		case (0x2): // j
			// Do this later
		break;

		case (0x3): // jal
			// Do this later
		break;

		
		// I-format
		case (0x8):  // addi
		case (0x9):  // addiu
		case (0xD):  // ori
		case (0xE):  // xori
		case (0xA):  // slti
		break;

		case (0x23): // lw
		case (0x32): // lb
		case (0x36): // lh		
			MEM_WB.LMD = mem_read_32(EX_MEM.ALUOutput);
		break;

		case (0xF):  // lui
		break;

		case (0x2B): // sw
		case (0x28): // sb
		case (0x29): // sh 
			mem_write_32(EX_MEM.ALUOutput, EX_MEM.B);
		break;

		case (0x1):  // bltz or bgez
			// Do this later
		break;

		case (0x4):  // beq
			// Do this later
		break;

		case (0x5):  // bne
			// Do this later
		break;

		case (0x6):  // blez
			// Do this later
		break;

		case (0x7):  // bgtz
			// Do this later
		break;

		default:
			printf("Error[MEM_access]: Invalid instruction\n");
		break;
	}

	return;
}

/**************************************************************/
/* Writes computed result to destination register              		                                */
/**************************************************************/
void WB_populate_destination(uint32_t instruction,
				uint32_t rt,
				uint32_t rd,
				uint32_t opcode,
				uint32_t funct) {
	switch (opcode) {
		// R-format
		case (0x0):
			switch (funct) {
				case (0x20): // add
				case (0x21): // addu
				case (0x22): // sub
				case (0x23): // subu
				case (0x24): // and
				case (0x25): // or
				case (0x26): // xor	
				case (0x27): // nor	
					NEXT_STATE.REGS[rd] = MEM_WB.ALUOutput;
				break;
				
				case (0x18): // mult
				case (0x19): // multu
				case (0x1A): // div
				case (0x1B): // divu
					NEXT_STATE.REGS[rd] = MEM_WB.ALUOutput;
				break;

				case (0x2A): // slt
					NEXT_STATE.REGS[rd] = MEM_WB.ALUOutput;
				break;

				case (0x00): // sll
					NEXT_STATE.REGS[rd] = MEM_WB.ALUOutput;
				break;

				case (0x02): // srl
					NEXT_STATE.REGS[rd] = MEM_WB.ALUOutput;
				break;

				case (0x3):  // sra
					NEXT_STATE.REGS[rd] = MEM_WB.ALUOutput;
				break;

				case (0x9):  // jalr	
					// Do this later
				break;

				case (0x10): // mfhi
					NEXT_STATE.REGS[rd] = CURRENT_STATE.HI;
				break;

				case (0x12): // mflo
					NEXT_STATE.REGS[rd] = CURRENT_STATE.LO;
				break;

				case (0x11): // mthi
					NEXT_STATE.HI = MEM_WB.ALUOutput;
				break;

				case (0x13): // mtlo
					NEXT_STATE.LO = MEM_WB.ALUOutput;
				break;

				case (0x8):  // jr
					// Do this later
				break;

				case (0xC): // syscall
				break;

				default:
					printf("ERROR[WB_populate_destination]: Invalid instruction (R)\n");
				break;
			}
		break;


		// J-format
		case (0x2): // j
		case (0x3): // jal
		break;

		
		// I-format
		case (0x8):  // addi
		case (0x9):  // addiu
		case (0xD):  // ori
		case (0xE):  // xori
		case (0xA):  // slti
			NEXT_STATE.REGS[rt] = MEM_WB.ALUOutput;
		break;

		case (0x23): // lw
			NEXT_STATE.REGS[rt] = MEM_WB.LMD;
		break;

		case (0x32): // lb
			NEXT_STATE.REGS[rt] = MEM_WB.LMD;
		break;

		case (0x36): // lh		
			NEXT_STATE.REGS[rt] = MEM_WB.LMD;
		break;

		case (0xF):  // lui
			NEXT_STATE.REGS[rt] = MEM_WB.ALUOutput;
		break;

		case (0x2B): // sw
		case (0x28): // sb
		case (0x29): // sh 
		break;

		case (0x1):  // bltz or bgez
		case (0x4):  // beq
		case (0x5):  // bne
		case (0x6):  // blez
		case (0x7):  // bgtz
		break;

		default:
			printf("Error[WB_populate_destination]: Invalid instruction\n");
		break;
	}

	return;
}

/************************************************************/
/* Initialize Memory                                                                                                    */ 
/************************************************************/
void initialize() { 
	init_memory();
	CURRENT_STATE.PC = MEM_TEXT_BEGIN;
	NEXT_STATE = CURRENT_STATE;
	RUN_FLAG = TRUE;
}

/************************************************************/
/* Print the program loaded into memory (in MIPS assembly format)    */ 
/************************************************************/
void print_program(){
	int i;
	uint32_t addr;
	
	for(i=0; i<PROGRAM_SIZE; i++){
		addr = MEM_TEXT_BEGIN + (i*4);
		printf("[0x%x]\t", addr);
		print_instruction(addr);
	}
}

void print_instruction(uint32_t addr){
	uint32_t instruction = 0;
	uint32_t opcode;
	uint32_t rs;
	uint32_t rt;
	uint32_t rd;
	uint32_t shamt;
	uint32_t funct;
	uint32_t immediate;
	uint32_t address;
	uint32_t temp;
	char reg_str[5];

	// Step 1: Read in the instruction at the given memory addr
	instruction = mem_read_32(addr);

	// Step 2: Isolate instruction
	decode_all_operands(instruction, &opcode, &rs, &rt, &rd, &shamt, &funct, &immediate, &address);

	// Step 3: Enter switch statements for R, J, and I respectively
	switch(opcode) {
	
		case (0x0): {  // R format
			// [ op - 6][ rs - 5 ][ rt - 5 ][ rd - 5 ][ shamt - 5 ][ funct - 6 ]
			switch(funct) {
				case (0x20): // add
					printf("add ");
					decode_machine_register(rd, reg_str);
					printf("%s, ", reg_str);
					decode_machine_register(rs, reg_str);
					printf("%s, ", reg_str);
					decode_machine_register(rt, reg_str);
					printf("%s\n", reg_str);
				break;

				case (0x21): // addu
					printf("addu ");
					decode_machine_register(rd, reg_str);
					printf("%s, ", reg_str);
					decode_machine_register(rs, reg_str);
					printf("%s, ", reg_str);
					decode_machine_register(rt, reg_str);
					printf("%s\n", reg_str);
				break;

				case (0x22): // sub
					printf("sub ");
					decode_machine_register(rd, reg_str);
					printf("%s, ", reg_str);
					decode_machine_register(rs, reg_str);
					printf("%s, ", reg_str);
					decode_machine_register(rt, reg_str);
					printf("%s\n", reg_str);
				break;

				case (0x23): // subu
					printf("subu ");
					decode_machine_register(rd, reg_str);
					printf("%s, ", reg_str);
					decode_machine_register(rs, reg_str);
					printf("%s, ", reg_str);
					decode_machine_register(rt, reg_str);
					printf("%s\n", reg_str);
				break;

				case (0x24): // and
					printf("and ");
					decode_machine_register(rd, reg_str);
					printf("%s, ", reg_str);
					decode_machine_register(rs, reg_str);
					printf("%s, ", reg_str);
					decode_machine_register(rt, reg_str);
					printf("%s\n", reg_str);
				break;

				case (0x25): // or
					printf("or ");
					decode_machine_register(rd, reg_str);
					printf("%s, ", reg_str);
					decode_machine_register(rs, reg_str);
					printf("%s, ", reg_str);
					decode_machine_register(rt, reg_str);
					printf("%s\n", reg_str);
				break;

				case (0x26): // xor	
					printf("xor ");
					decode_machine_register(rd, reg_str);
					printf("%s, ", reg_str);
					decode_machine_register(rs, reg_str);
					printf("%s, ", reg_str);
					decode_machine_register(rt, reg_str);
					printf("%s\n", reg_str);
					break;

				case (0x27): // nor	
					printf("nor ");
					decode_machine_register(rd, reg_str);
					printf("%s, ", reg_str);
					decode_machine_register(rs, reg_str);
					printf("%s, ", reg_str);
					decode_machine_register(rt, reg_str);
					printf("%s\n", reg_str);
				break;

				case (0x18): // mult
					printf("mult ");
					decode_machine_register(rs, reg_str);
					printf("%s, ", reg_str);
					decode_machine_register(rt, reg_str);
					printf("%s\n", reg_str);
				break;

				case (0x19): // multu
					printf("multu ");
					decode_machine_register(rs, reg_str);
					printf("%s, ", reg_str);
					decode_machine_register(rt, reg_str);
					printf("%s\n", reg_str);
				break;

				case (0x1A): // div
					printf("div ");
					decode_machine_register(rs, reg_str);
					printf("%s, ", reg_str);
					decode_machine_register(rt, reg_str);
					printf("%s\n", reg_str);
				break;

				case (0x1B): // divu
					printf("divu ");
					decode_machine_register(rs, reg_str);
					printf("%s, ", reg_str);
					decode_machine_register(rt, reg_str);
					printf("%s\n", reg_str);
				break;

				case (0x2A): // slt
					printf("slt ");
					decode_machine_register(rd, reg_str);
					printf("%s, ", reg_str);
					decode_machine_register(rs, reg_str);
					printf("%s, ", reg_str);
					decode_machine_register(rt, reg_str);
					printf("%s\n", reg_str);
				break;

				case (0x00): // sll
					printf("sll ");
					decode_machine_register(rd, reg_str);
					printf("%s, ", reg_str);
					decode_machine_register(rs, reg_str);
					printf("%s, %d\n", reg_str, shamt);
				break;

				case (0x02): // srl
					printf("srl ");
					decode_machine_register(rd, reg_str);
					printf("%s, ", reg_str);
					decode_machine_register(rs, reg_str);
					printf("%s, %d\n", reg_str, shamt);
				break;

				case (0x3):  // sra
					printf("sra ");
					decode_machine_register(rd, reg_str);
					printf("%s, ", reg_str);
					decode_machine_register(rs, reg_str);
					printf("%s, %d\n", reg_str, shamt);
				break;

				case (0x10): // mfhi
					printf("mfhi ");
					decode_machine_register(rd, reg_str);
					printf("%s\n", reg_str);
				break;

				case (0x12): // mflo
					printf("mflo ");
					decode_machine_register(rd, reg_str);
					printf("%s\n", reg_str);
				break;

				case (0x11): // mthi
					printf("mthi ");
					decode_machine_register(rs, reg_str);
					printf("%s\n", reg_str);
				break;

				case (0x13): // mtlo
					printf("mtlo ");
					decode_machine_register(rs, reg_str);
					printf("%s\n", reg_str);
				break;

				case (0x8):  // jr
					printf("jr ");
					decode_machine_register(rs, reg_str);
					printf("%s\n", reg_str);
				break;

				case (0x9):  // jalr	
					printf("jalr ");
					if (rd == 31) {
						// format is rs
						decode_machine_register(rs, reg_str);
						printf("%s\n", reg_str);
					} else {
						// format is rd, rs
						decode_machine_register(rd, reg_str);
						printf("%s, ", reg_str);
						decode_machine_register(rs, reg_str);
						printf("%s\n", reg_str);
					}
				break;

				case (0xC):  // syscall
					printf("syscall\n");
				break;

				default:
					printf("Error: Invalid instruction at memory location (R-format)\n");
				break;
			}
		break;
		}

		// J-format
		// [ op - 6][        const/address - 26        ]
		case (0x2): // j
			printf("j %d\n", (address << 2));
		break;

		case (0x3): // jal
			printf("j %d\n", (address << 2));
		break;

		// I-format
		// [ op - 6][ rs - 5 ][ rt - 5 ][    immmediate - 16    ]
		case (0x8):  // addi
			printf("addi ");
			decode_machine_register(rt, reg_str);
			printf("%s, ", reg_str);
			decode_machine_register(rs, reg_str);
			printf("%s, %d\n", reg_str, immediate);	
		break;

		case (0x9):  // addiu
			printf("addiu ");
			decode_machine_register(rt, reg_str);
			printf("%s, ", reg_str);
			decode_machine_register(rs, reg_str);
			printf("%s, %d\n", reg_str, immediate);	
		break;

		case (0xC):  // andi
			printf("andi ");
			decode_machine_register(rt, reg_str);
			printf("%s, ", reg_str);
			decode_machine_register(rs, reg_str);
			printf("%s, %d\n", reg_str, immediate);	
		break;

		case (0xD):  // ori
			printf("ori ");
			decode_machine_register(rt, reg_str);
			printf("%s, ", reg_str);
			decode_machine_register(rs, reg_str);
			printf("%s, %d\n", reg_str, immediate);	
		break;

		case (0xE):  // xori
			printf("xori ");
			decode_machine_register(rt, reg_str);
			printf("%s, ", reg_str);
			decode_machine_register(rs, reg_str);
			printf("%s, %d\n", reg_str, immediate);	
		break;

		case (0xA):  // slti
			printf("slti ");
			decode_machine_register(rt, reg_str);
			printf("%s, ", reg_str);
			decode_machine_register(rs, reg_str);
			printf("%s, %d\n", reg_str, immediate);	
		break;

		case (0x23): // lw
			printf("lw ");
			decode_machine_register(rt, reg_str);
			printf("%s, ", reg_str);
			decode_machine_register(rs, reg_str);
			printf("%d(%s)\n", immediate, reg_str);	
		break;

		case (0x32): // lb
			printf("lb ");
			decode_machine_register(rt, reg_str);
			printf("%s, ", reg_str);
			decode_machine_register(rs, reg_str);
			printf("%d(%s)\n", immediate, reg_str);	
		break;

		case (0x36): // lh
			printf("lh ");
			decode_machine_register(rt, reg_str);
			printf("%s, ", reg_str);
			decode_machine_register(rs, reg_str);
			printf("%d(%s)\n", immediate, reg_str);	
		break;

		case (0xF):  // lui
			printf("lui ");
			decode_machine_register(rt, reg_str);
			printf("%s, %d\n", reg_str, immediate);
		break;

		case (0x2B): // sw
			printf("sw ");
			decode_machine_register(rt, reg_str);
			printf("%s, ", reg_str);
			decode_machine_register(rs, reg_str);
			printf("%d(%s)\n", immediate, reg_str);	
		break;

		case (0x28): // sb
			printf("sb ");
			decode_machine_register(rt, reg_str);
			printf("%s, ", reg_str);
			decode_machine_register(rs, reg_str);
			printf("%d(%s)\n", immediate, reg_str);	
		break;

		case (0x29): // sh 
			printf("sh ");
			decode_machine_register(rt, reg_str);
			printf("%s, ", reg_str);
			decode_machine_register(rs, reg_str);
			printf("%d(%s)\n", immediate, reg_str);	
		break;

		case (0x1): { // bltz or bgez
			if ( rt == 1 ) { // bgez
				printf("bgez ");

			} else if ( rt == 0 ) { // bltz
				printf("bltz ");
			} 
			decode_machine_register(rt, reg_str);
			printf("%s, 0x%x\n", reg_str, immediate);
		break;
		}

		case (0x4): { // beq
			printf("beq ");
			decode_machine_register(rs, reg_str);
			printf("%s, ", reg_str);
			decode_machine_register(rt, reg_str);
			printf("%s, 0x%x\n", reg_str, immediate);
		break;
		}

		case (0x5): { // bne
			printf("bne ");
			decode_machine_register(rs, reg_str);
			printf("%s, ", reg_str);
			decode_machine_register(rt, reg_str);
			printf("%s, 0x%x\n", reg_str, immediate);
		break;
		}

		case (0x6): { // blez
			printf("blez ");
			decode_machine_register(rs, reg_str);
			printf("%s, 0x%x\n", reg_str, immediate);
		break;
		}

		case (0x7): { // bgtz
			printf("bgtz ");
			decode_machine_register(rs, reg_str);
			printf("%s, 0x%x\n", reg_str, immediate);
		break;
		}

		default:
			printf("Error[print_instruction]: Invalid instruction at memory location\n");
		break;
	}
}

/************************************************************/
/* Print the current pipeline                                                                                    */ 
/************************************************************/
void show_pipeline(){
	/*IMPLEMENT THIS*/
}

/***************************************************************/
/* main                                                                                                                                   */
/***************************************************************/
int main(int argc, char *argv[]) {                              
	printf("\n**************************\n");
	printf("Welcome to MU-MIPS SIM...\n");
	printf("**************************\n\n");
	
	if (argc < 2) {
		printf("Error: You should provide input file.\nUsage: %s <input program> \n\n",  argv[0]);
		exit(1);
	}

	strcpy(prog_file, argv[1]);
	initialize();
	load_program();
	help();
	while (1){
		handle_command();
	}
	return 0;
}
