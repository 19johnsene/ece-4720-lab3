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
	uint32_t rd;

	WB_decode_operands(MEM_WB.IR, &rd, &opcode, &funct);

	WB_populate_destination(MEM_WB.IR, rd, opcode, funct);
	
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
	EX_perform_operation(EX_MEM.IR, ID_EX.A, ID_EX.B, ID_EX.imm, &opcode, &shamt, &funct, &addr);

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

/**************************************************************/
/* Isolates portions of 32-bit binary instruction to determine register                                */
/* values (opcode, rs, rt, rd, shamt, funct, immediate, and address)                                   */
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

void EX_decode_operands(uint32_t instruction,
				uint32_t* opcode, 
				uint32_t* shamt, 
				uint32_t* funct,
				uint32_t* address)
		{
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
	*address = temp;		
}

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

void WB_decode_operands(uint32_t instruction,
				uint32_t* rd,
				uint32_t* opcode, 
				uint32_t* funct) {
	uint32_t temp;
	temp = instruction;
	temp >>= 26;
	*opcode = temp;
	
	temp = instruction;
	temp <<= 16;
	temp >>= 27;
	*rd = temp;

	temp = instruction;
	temp <<= 26;
	temp >>= 26;
	*funct = temp;
}

void EX_perform_operation(uint32_t instruction,
				uint32_t A,
				uint32_t B,
				uint32_t imm,
				uint32_t opcode,
				uint32_t shamt,
				uint32_t funct,
				uint32_t address) {

	// Operations can be:
	// 1. ALUOutput <= A + imm
	// 2. ALUOutput <= A op B
	// 3. ALUOutput <= A op imm

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

				case (0x18): { // mult
					EX_MEM.ALUOutput = (A * B);
				break;
				}

				case (0x19): { // multu
					EX_MEM.ALUOutput = (A * B);
				break;
				}

				case (0x1A): { // div
					EX_MEM.ALUOutput = (A / B);
				break;
				}

				case (0x1B): { // divu
					EX_MEM.ALUOutput = (A / B);
				break;
				}

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
					// WB only
				break;

				case (0x12): // mflo
					// WB only
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
			EX_MEM.ALUOutput = A + imm;
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

void MEM_access(uint32_t instruction,
				uint32_t opcode,
				uint32_t funct,
				uint32_t address) {

	MEM_WB.ALUOutput = EX_MEM.ALUOutput;
	
	switch (opcode) {
		// R-format
		case (0x0):
			// Does not use MEM
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
		break;

		case (0x9):  // addiu
		break;

		case (0xD):  // ori
		break;

		case (0xE):  // xori
		break;

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
			printf("Error[EX_perform_operation]: Invalid instruction\n");
		break;
	}

	return;
}

void WB_populate_destination(uint32_t instruction,
				uint32_t rd,
				uint32_t opcode,
				uint32_t funct) {
	switch (opcode) {
		// R-format
		case (0x0):
			switch (funct) {
				case (0x20): // add
					NEXT_STATE.REGS[rd] = MEM_WB.ALUOutput;
				break;

				case (0x21): // addu
					NEXT_STATE.REGS[rd] = MEM_WB.ALUOutput;
				break;

				case (0x22): // sub
					NEXT_STATE.REGS[rd] = MEM_WB.ALUOutput;
				break;

				case (0x23): // subu
					NEXT_STATE.REGS[rd] = MEM_WB.ALUOutput;
				break;

				case (0x24): // and
					NEXT_STATE.REGS[rd] = MEM_WB.ALUOutput;
				break;

				case (0x25): // or
					NEXT_STATE.REGS[rd] = MEM_WB.ALUOutput;
				break;

				case (0x26): // xor	
					NEXT_STATE.REGS[rd] = MEM_WB.ALUOutput;
				break;

				case (0x27): // nor	
					NEXT_STATE.REGS[rd] = MEM_WB.ALUOutput;
					break;

				case (0x18): // mult
					NEXT_STATE.REGS[rd] = MEM_WB.ALUOutput;
				break;

				case (0x19): // multu
					NEXT_STATE.REGS[rd] = MEM_WB.ALUOutput;
				break;

				case (0x1A): // div
					NEXT_STATE.REGS[rd] = MEM_WB.ALUOutput;
				break;

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
					NEXT_STATE.HI;
				break;

				case (0x13): // mtlo
					NEXT_STATE.LO;
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
			EX_MEM.ALUOutput = A + imm;
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
	/*IMPLEMENT THIS*/
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
