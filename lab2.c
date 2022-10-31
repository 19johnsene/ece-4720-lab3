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
	printf("print\t-- print the program loaded into memory to terminal\n");
	printf("file\t-- print the program loaded into memory to file\n");
	printf("convert\t-- convert a mips file to machine code file\n");
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
	handle_machine_instruction();
	CURRENT_STATE = NEXT_STATE;
	INSTRUCTION_COUNT++;
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
/* Simulate program to completion                                                  */
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
			runAll(); 
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
		case 'F':
		case 'f':
			write_file(); 
			break;
		case 'C':
		case 'c':
			convert_mips();
			break;
		default:
			printf("Invalid Command.\n");
			break;
	}
}

/***************************************************************/
/* Reset registers/memory and reload program                                                    */
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
/* Load program into memory                                                                                      */
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

/**************************************************************/
/* Isolates portions of 32-bit binary instruction to determine register             */
/* values (opcode, rs, rt, rd, shamt, funct, immediate, and address)                */
/**************************************************************/
void isolate_vars(uint32_t instruction_code,
				uint32_t* opcode,
				uint32_t* rs, 
				uint32_t* rt, 
				uint32_t* rd, 
				uint32_t* shamt,
				uint32_t* funct,
				uint32_t* immediate,
				uint32_t* address)
		{
		
		uint32_t temp;
		
		temp = instruction_code;
		temp >>= 26;
		*opcode = temp;

		temp = instruction_code;
		temp <<= 6;
		temp >>= 27;
		*rs = temp;

		temp = instruction_code;
		temp <<= 11;
		temp >>= 27;
		*rt = temp;

		temp = instruction_code;
		temp <<= 16;
		temp >>= 27;
		*rd = temp;

		temp = instruction_code;
		temp <<= 21;
		temp >>= 27;
		*shamt = temp;	

		temp = instruction_code;
		temp <<= 26;
		temp >>= 26;
		*funct = temp;

		temp = instruction_code;
		temp <<= 16;
		temp >>= 16;
		*immediate = temp;	

		temp = instruction_code;
		temp <<= 6;
		temp >>= 6;
		*address = temp;

		return;			
}

/************************************************************/
/* Simulates the current machine instruction loaded in memory  */
/************************************************************/
void handle_machine_instruction()
{
	uint32_t instruction;
	uint32_t opcode;
	uint32_t rs;
	uint32_t rt;
	uint32_t rd;
	uint32_t shamt;
	uint32_t funct;
	uint32_t immediate;
	uint32_t address;
	uint32_t high, low;
	uint32_t temp = 0;

	// Get the current instruction
	instruction = mem_read_32(CURRENT_STATE.PC);

	// Isolate the registers to be used in the operation
	isolate_vars(instruction, &opcode, &rs, &rt, &rd, &shamt, &funct, &immediate, &address);

	// Enter switch statements for R, J, and I respectively
	switch(opcode) {
		
		case (0x0): {  // R type
			// [ op - 6][ rs - 5 ][ rt - 5 ][ rd - 5 ][ shamt - 5 ][ funct - 6 ]
			switch(funct) {
				case (0x20): // add
					NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rs] + CURRENT_STATE.REGS[rt];
					NEXT_STATE.PC += 4;
					break;

				case (0x21): // addu
					NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rs] + CURRENT_STATE.REGS[rt];
					NEXT_STATE.PC += 4;
					break;

				case (0x22): // sub
					NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rs] - CURRENT_STATE.REGS[rt];
					NEXT_STATE.PC += 4;
					break;

				case (0x23): // subu
					NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rs] - CURRENT_STATE.REGS[rt];
					NEXT_STATE.PC += 4;
					break;

				case (0x24): // and
					NEXT_STATE.REGS[rd] = (CURRENT_STATE.REGS[rs] && CURRENT_STATE.REGS[rt]);
					NEXT_STATE.PC += 4;
					break;

				case (0x25): // or
					NEXT_STATE.REGS[rd] = (CURRENT_STATE.REGS[rs] || CURRENT_STATE.REGS[rt]);
					NEXT_STATE.PC += 4;
					break;

				case (0x26): // xor	
					NEXT_STATE.REGS[rd] = (CURRENT_STATE.REGS[rs] ^ CURRENT_STATE.REGS[rt]);
					NEXT_STATE.PC += 4;
					break;

				case (0x27): // nor	
					NEXT_STATE.REGS[rd] = !(CURRENT_STATE.REGS[rs] || CURRENT_STATE.REGS[rt]);
					NEXT_STATE.PC += 4;
					break;

				case (0x18): { // mult
					uint64_t result = (CURRENT_STATE.REGS[rs] * CURRENT_STATE.REGS[rt]);
					high = (result >> 32);
					low = ((result >> 32) >> 32);
					NEXT_STATE.REGS[2] = high;
					NEXT_STATE.REGS[3] = low;
					NEXT_STATE.PC += 4;
					break;
				}

				case (0x19): { // multu
					uint64_t result = (CURRENT_STATE.REGS[rs] * CURRENT_STATE.REGS[rt]);
					high = (result >> 32);
					low = ((result >> 32) >> 32);
					NEXT_STATE.REGS[2] = high;
					NEXT_STATE.REGS[3] = low;
					NEXT_STATE.PC += 4;
					break;
				}

				case (0x1A): { // div
					uint64_t result = (CURRENT_STATE.REGS[rs] / CURRENT_STATE.REGS[rt]);
					low = ((result >> 32) >> 32);
					result = (CURRENT_STATE.REGS[rs] % CURRENT_STATE.REGS[rt]);
					high = (result >> 32);
					NEXT_STATE.REGS[2] = high;
					NEXT_STATE.REGS[3] = low;
					NEXT_STATE.PC += 4;
					break;
				}

				case (0x1B): { // divu
					uint64_t result = (CURRENT_STATE.REGS[rs] / CURRENT_STATE.REGS[rt]);
					low = ((result >> 32) >> 32);
					result = (CURRENT_STATE.REGS[rs] % CURRENT_STATE.REGS[rt]);
					high = (result >> 32);
					NEXT_STATE.REGS[2] = high;
					NEXT_STATE.REGS[3] = low;
					NEXT_STATE.PC += 4;
					break;
				}

				case (0x2A): // slt
					NEXT_STATE.REGS[rd] = (CURRENT_STATE.REGS[rs] < CURRENT_STATE.REGS[rt]) ? 1 : 0;
					NEXT_STATE.PC += 4;
					break;

				case (0x00): // sll
					NEXT_STATE.REGS[rd] = (CURRENT_STATE.REGS[rt] << shamt);
					NEXT_STATE.PC += 4;
					break;

				case (0x02): // srl
					NEXT_STATE.REGS[rd] = (CURRENT_STATE.REGS[rt] >> shamt);
					NEXT_STATE.PC += 4;
					break;

				case (0x3):  // sra
					NEXT_STATE.REGS[rd] = (CURRENT_STATE.REGS[rt] >> shamt);
					NEXT_STATE.PC += 4;
					break;

				case (0x9):  // jalr	
					// if only rs (rd asumed to be 31)
					if (CURRENT_STATE.REGS[rd] == 31) {
						NEXT_STATE.REGS[31] = (CURRENT_STATE.PC + 8);
						NEXT_STATE.PC = CURRENT_STATE.REGS[rs];
					} else {
						NEXT_STATE.REGS[rd] = (CURRENT_STATE.PC + 8);
						NEXT_STATE.PC = CURRENT_STATE.REGS[rs]; 
					}
					break;

				case (0x10): // mfhi
					NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[2];
					NEXT_STATE.PC += 4;
					break;

				case (0x12): // mflo
					NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[3];
					break;

				case (0x11): // mthi
					NEXT_STATE.REGS[2] = CURRENT_STATE.REGS[rd];
					NEXT_STATE.PC += 4;
					break;

				case (0x13): // mtlo
					NEXT_STATE.REGS[3] = CURRENT_STATE.REGS[rd];
					NEXT_STATE.PC += 4;
					break;

				case (0x8):  // jr
					NEXT_STATE.PC = CURRENT_STATE.REGS[rs];
					break;

				case (0xC):  // syscall
					NEXT_STATE.REGS[2] = 0xA;
					NEXT_STATE.PC += 4;
					break;

				default:
					printf("ERROR[handle_machine_instruction]: Invalid instruction (R)\n");
					break;
			}
			break;
		}
		
		// J-format
		// [ op - 6][        const/address - 26        ]
		case (0x2): // j
			temp = CURRENT_STATE.PC;
			temp <<= 28;
			temp >>= 28;
			NEXT_STATE.PC = temp + (address << 2);
			break;

		case (0x3): // jal
			temp = CURRENT_STATE.PC;
			temp <<= 28;
			temp >>= 28;
			NEXT_STATE.PC = temp + (address << 2);			
			NEXT_STATE.REGS[31] = (CURRENT_STATE.PC + 8);
			NEXT_STATE.PC = address;
			break;

		// I-format
		// [ op - 6][ rs - 5 ][ rt - 5 ][    immmediate - 16    ]
		case (0x8):  // addi
			NEXT_STATE.REGS[rt] = CURRENT_STATE.REGS[rs] + immediate;
			NEXT_STATE.PC += 4;
			break;

		case (0x9):  // addiu
			NEXT_STATE.REGS[rt] = CURRENT_STATE.REGS[rs] + immediate;
			NEXT_STATE.PC += 4;
			break;

		case (0xD):  // ori
			NEXT_STATE.REGS[rt] = (CURRENT_STATE.REGS[rs] || immediate);
			NEXT_STATE.PC += 4;
			break;

		case (0xE):  // xori
			NEXT_STATE.REGS[rt] = (CURRENT_STATE.REGS[rs] ^ immediate);
			NEXT_STATE.PC += 4;
			break;

		case (0xA):  // slti
			NEXT_STATE.REGS[rd] = (CURRENT_STATE.REGS[rs] < immediate) ? 1 : 0;
			NEXT_STATE.PC += 4;
			break;

		case (0x23): // lw
			NEXT_STATE.REGS[rt] = *(MEM_REGIONS[1].mem + immediate);
			NEXT_STATE.PC += 4;
			break;

		case (0x32): // lb
			temp = CURRENT_STATE.REGS[rs];
			temp <<= 24;
			temp >>= 24;
			NEXT_STATE.REGS[rt] = *(MEM_REGIONS[1].mem + temp);
			NEXT_STATE.PC += 4;
			break;

		case (0x36): // lh
			temp = CURRENT_STATE.REGS[rs];
			temp <<= 16;
			temp >>= 16;	
			NEXT_STATE.REGS[rt] = *(MEM_REGIONS[1].mem + temp);
			NEXT_STATE.PC += 4;					
			break;

		case (0xF):  // lui
			temp = immediate;
			temp <<= 16;
			NEXT_STATE.REGS[rt] = temp;
			NEXT_STATE.PC += 4;
			break;

		case (0x2B): // sw
			*(MEM_REGIONS[1].mem + immediate) = CURRENT_STATE.REGS[rt];
			NEXT_STATE.PC += 4;
			break;

		case (0x28): // sb
			temp = CURRENT_STATE.REGS[rt];
			temp <<= 24;
			temp >>= 24;
			*(MEM_REGIONS[1].mem + immediate) = temp;
			NEXT_STATE.PC += 4;
			break;

		case (0x29): // sh 
			temp = CURRENT_STATE.REGS[rt];
			temp <<= 16;
			temp >>= 16;
			*(MEM_REGIONS[1].mem + immediate) = temp;
			NEXT_STATE.PC += 4;
			break;

		case (0x1):  // bltz or bgez
			if ( rt == 1 ) { // bgez
				if (CURRENT_STATE.REGS[rs] >= 0) {
					temp = immediate;
					for (int i = 16; i < 32; i++) {
						temp += (1 << i);
					}
				} else {
					NEXT_STATE.PC += (temp << 2);
				}
			} else if ( rt == 0 ) { // bltz
				if (CURRENT_STATE.REGS[rs] < 0) {
					temp = immediate;
					for (int i = 16; i < 32; i++) {
						temp += (1 << i);
					}
				} else {
					NEXT_STATE.PC += (temp << 2);
				}
			} 
			break;

		case (0x4):  // beq
			if (CURRENT_STATE.REGS[rs] == CURRENT_STATE.REGS[rt]) {
				temp = immediate;
				for (int i = 16; i < 32; i++) {
					temp += (1 << i);
				}
			} else {
				NEXT_STATE.PC += (temp << 2);
			}
			break;

		case (0x5):  // bne
			if (CURRENT_STATE.REGS[rs] != CURRENT_STATE.REGS[rt]) {
				temp = immediate;
				for (int i = 16; i < 32; i++) {
					temp += (1 << i);
				}
			} else {
				NEXT_STATE.PC += temp << 2;
			}
			break;

		case (0x6):  // blez
			if (CURRENT_STATE.REGS[rs] <= 0) {
				temp = immediate;
				for (int i = 16; i < 32; i++) {
					temp += (1 << i);
				}
			} else {
				NEXT_STATE.PC += temp << 2;		
			}
			break;

		case (0x7):  // bgtz
			if (CURRENT_STATE.REGS[rs] > 0) {
				temp = immediate;
				for (int i = 16; i < 32; i++) {
					temp += (1 << i);
				}
			} else {
				NEXT_STATE.PC += (temp << 2);	
			}
			break;

		default:
			printf("Error[handle_machine_instruction]: Invalid instruction\n");
			break;
	}
}

/************************************************************/
/* Accepts an input file written in MIPS assembly and              */ 
/* generates output file containing corresponding machine code     */
/************************************************************/
void convert_mips() {
	FILE * fp_i; 
	FILE * fp_o; 
	char input_file[20]; 				// file containing MIPS (input)
	char output_file[20]; 				// file containing machine (output)
	int buffer_length = 40;
	char buffer[buffer_length]; 	 // mips instruction
	char* instr;	   				 // instruction name
	char* regs; 					 // instruction arguments (registers)
	uint32_t machine; 			     // machine instruction
	char* delim = " ";

	// Open program file containing MIPS assembly
	printf("\nPlease enter the name of the file you want to convert:  ");
	if (scanf("%19s", input_file) == EOF) {
		printf("\nError[convert_mips]: Could not get mips filename\n");
	}

	fp_i = fopen(input_file, "r");
	if (fp_i == NULL) {
		printf("\nError[convert_mips]: Can't open input file %s\n", input_file);
		exit(-1);
	}

	// Create output file for machine code
	printf("\nPlease enter the name of the machine file you want to generate:  ");
	if (scanf("%19s", output_file) == EOF) {
		printf("Error[convert_mips]: Could not get machine filename\n");
	}

	fp_o = fopen(output_file, "w");
	if (fp_i == NULL) {
		printf("Error[convert_mips]: Cannot write to output file %s\n", output_file);
		exit(-1);
	}

	printf("\nConverting MIPS to machine...\n");
	// Read in MIPS instructions line by line
	while(fgets(buffer, buffer_length, fp_i)) {

		char* temp = buffer;

		// Get the instruction name
		instr = strtok(temp, delim);

		// Get the remainder of the instruction for use later
		regs = buffer;
		regs = regs + strlen(instr) + 1;

		// Determine which instruction we have character by character
		switch(instr[0]) {
			// ADDU, ADDIU, ADDI, ADD, ANDI, AND
			case('a'):
			case('A'):
				// ADDU, ADDIU, ADDI, ADD
				if (instr[1] == 'd' || instr[1] == 'D') {
					if (instr[3] == 'u' || instr[3] == 'U') { // ADDU
						machine = 0x0; // opcode
						mips_regs_to_machine(0, regs, &machine);
						machine += 0x21; // funct
					} else if (instr[3] == 'i' || instr[3] == 'I') {
						if (instr[4] == 'u' || instr[4] == 'U') { // ADDIU
							machine = 0x9; // opcode
							machine <<= 26;
							mips_regs_to_machine(1, regs, &machine);
						} else { // ADDI
							machine = 0x8; // opcode
							machine <<= 26;
							mips_regs_to_machine(1, regs, &machine);
						}
					} else { // ADD
						machine = 0x0; // opcode
						mips_regs_to_machine(0, regs, &machine);
						machine += 0x20; // funct
					}
				// ANDI, AND
				} else if (instr[1] == 'n' || instr[1] == 'N') { 
					if (instr[3] == 'i' || instr[3] == 'I') { // ANDI
						machine = 0xC; // opcode
						machine <<= 26;
						mips_regs_to_machine(1, regs, &machine);
					} else {  // AND
						machine = 0x0; // opcode
						mips_regs_to_machine(0, regs, &machine);
						machine += 0x24; // funct
					}
				}
			break;

			// BLEZ, BLTZ, BGEZ, BGTZ, BEQ, BNE
			case('b'):
			case('B'):
				// BLEZ, BLTZ
				// RT == 0
				if (instr[1] == 'l' || instr[1] == 'L') {
					uint32_t temp_rt = 0x0;
					uint32_t temp_imm = 0x0;
					uint32_t hold = 0x0;
					if (instr[2] == 'e' || instr[2] == 'E') { // BLEZ
						machine = 0x6; // opcode
						machine <<= 26;
						mips_regs_to_machine(1, regs, &machine);
					} else { // BLTZ
						machine = 0x1; // opcode
						machine <<= 26;
						mips_regs_to_machine(1, regs, &machine);
					}
					// We need to set rt but maintain our other operands
					temp_imm = machine;
					temp_imm <<= 16;
					temp_imm >>= 16;

					hold = machine;
					hold >>= 16;
					hold += temp_rt;
					hold <<= 16;
					hold += temp_imm;
					machine = hold;

				// BGEZ, BGTZ
				// RT == 1
				} else if (instr[1] == 'g' || instr[1] == 'G') {
					uint32_t temp_rt = 0x1;
					uint32_t temp_imm = 0x0;
					uint32_t hold = 0x0;
					if (instr[2] == 'e' || instr[2] == 'E') { // BGEZ
						machine = 0x1; // opcode
						machine <<= 26;
						mips_regs_to_machine(1, regs, &machine);
					} else { // BGTZ
						machine = 0x7; // opcode
						machine <<= 26;
						mips_regs_to_machine(1, regs, &machine);
					}
					// We need to set rt but maintain our other operands
					temp_imm = machine;
					temp_imm <<= 16;
					temp_imm >>= 16;

					hold = machine;
					hold >>= 16;
					hold += temp_rt;
					hold <<= 16;
					hold += temp_imm;
					machine = hold;
				
				} else if (instr[1] == 'e' || instr[1] == 'E') { // BEQ
					machine = 0x4; // opcode
					machine <<= 26;
					mips_regs_to_machine(3, regs, &machine);
				} else { // BNE
					machine = 0x5; // opcode
					machine <<= 26;
					mips_regs_to_machine(3, regs, &machine);
				}
			break;

			// DIVU, DIV
			case('d'):
			case('D'):
				if (instr[3] == 'u' || instr[3] == 'U') { // DIVU
					machine = 0x0; // opcode
					mips_regs_to_machine(0, regs, &machine);
					machine += 0x1B; // funct
				} else { // DIV
					machine = 0x0; // opcode
					mips_regs_to_machine(0, regs, &machine);
					machine += 0x1A; // funct
				}
			break;

			// JR, JALR, JAL, J
			case('j'):
			case('J'):
				if (instr[1] == 'r' || instr[1] == 'R') { // JR
					machine = 0x0; // opcode
					mips_regs_to_machine(0, regs, &machine);
					machine += 0x08; // funct
				// JALR, JAL
				} else if (instr[2] == 'l' || instr[2] == 'L') {
					if (instr[3] == 'r' || instr[3] == 'R') { // JALR
						machine = 0x0; // opcode
						mips_regs_to_machine(0, regs, &machine);
						machine += 0x9; // funct
					} else { // JAL
						machine = 0x3; // opcode
						machine <<= 26;
						mips_regs_to_machine(2, regs, &machine);
					}
				} else { // J
					machine = 0x2; // opcode
					machine <<= 26;
					mips_regs_to_machine(2, regs, &machine);
				}
			break;

			// LW, LB, LH, LUI
			case('l'):
			case('L'):
				if (instr[1] == 'w' || instr[1] == 'W') { // LW
					machine = 0x23; // opcode
					machine <<= 26;
					mips_regs_to_machine(3, regs, &machine);
				} else if (instr[1] == 'b' || instr[1] == 'B') { // LB
					machine = 0x32; // opcode
					machine <<= 26;
					mips_regs_to_machine(3, regs, &machine);
				} else if (instr[1] == 'h' || instr[1] == 'H') { // LH
					machine = 0x36; // opcode
					machine <<= 26;
					mips_regs_to_machine(3, regs, &machine);
				} else if (instr[1] == 'u' || instr[1] == 'U') { // LUI
					machine = 0xF; // opcode
					machine <<= 26;
					mips_regs_to_machine(3, regs, &machine);
				}
			break;

			// MULT, MULTU, MFHI, MFLO, MTHI, MTLO
			case('m'):
			case('M'):
				// MULTU, MULT
				if (instr[1] == 'u' || instr[1] == 'U') {
					if (instr[4] == 'u' || instr[4] == 'U') { // MULTU
						machine = 0x0; // opcode
						mips_regs_to_machine(0, regs, &machine);
						machine += 0x19; // funct
					} else { // MULT
						machine = 0x0; // opcode
						mips_regs_to_machine(0, regs, &machine);
						machine += 0x18; // funct
					}
				 // MFHI, MFLO
				} else if (instr[1] == 'f' || instr[1] == 'F') {
					if (instr[2] == 'h' || instr[2] == 'H') { // MFHI
						machine = 0x0; // opcode
						mips_regs_to_machine(0, regs, &machine);
						machine += 0x10; // funct
					} else if (instr[2] == 'l' || instr[2] == 'L') { // MFLO
						machine = 0x0; // opcode
						mips_regs_to_machine(0, regs, &machine);
						machine += 0x12; // funct
					}
				// MTHI, MTLO
				} else if (instr[1] == 't' || instr[1] == 'T') { 
					if (instr[2] == 'h' || instr[2] == 'H') { // MTHI
						machine = 0x0; // opcode
						mips_regs_to_machine(0, regs, &machine);
						machine += 0x11; // funct
					} else if (instr[2] == 'l' || instr[2] == 'L') { // MTLO
						machine = 0x0; // opcode
						mips_regs_to_machine(0, regs, &machine);
						machine += 0x13; // funct
					}
				}
			break;
			
			// NOR
			case('n'):
			case('N'):
				machine = 0x0; // opcode
				mips_regs_to_machine(0, regs, &machine);
				machine += 0x27; // funct
			break;
			
			// ORI, OR
			case('o'):
			case('O'):
				if (instr[2] == 'i' || instr[2] == 'I') { // ORI
					machine = 0xD; // opcode
					machine <<= 26;
					mips_regs_to_machine(1, regs, &machine);
				} else { // OR
					machine = 0x0; // opcode
					mips_regs_to_machine(0, regs, &machine);
					machine += 0x25; // funct
				}
			break;
			
			// SUBU, SUB, SLL, SLTI, SLT, SRL, SRA, SW, SB, SH, SYSCALL
			case('s'):
			case('S'): 
				// SUBU, SUB
				if (instr[1] == 'u' || instr[1] == 'U') {
					if (instr[3] == 'u' || instr[3] == 'U') { // SUBU
						machine = 0x0; // opcode
						mips_regs_to_machine(0, regs, &machine);
						machine += 0x23; // funct
					} else { // SUB
						machine = 0x0; // opcode
						mips_regs_to_machine(0, regs, &machine);
						machine += 0x22; // funct
					}
				// SLL, SLTI, SLT
				} else if (instr[1] == 'l' || instr[1] == 'L') {
					if (instr[2] == 'l' || instr[2] == 'L') { // SLL
						machine = 0x0; // opcode
						mips_regs_to_machine(0, regs, &machine);
						machine += 0x00; // funct
					} else if (instr[2] == 't' || instr[2] == 'T') {
						if (instr[3] == 'i' || instr[3] == 'I') { // SLTI
							machine = 0xA; // opcode
							machine <<= 26;
							mips_regs_to_machine(1, regs, &machine);
						} else { // SLT
							machine = 0x0; // opcode
							mips_regs_to_machine(0, regs, &machine);
							machine += 0x2A; // funct
						}
					}
				// SRL, SRA
				} else if (instr[1] == 'r' || instr[1] == 'R') {
					if (instr[2] == 'l' || instr[2] == 'L') { // SRL
						machine = 0x0; // opcode
						mips_regs_to_machine(0, regs, &machine);
						machine += 0x02; // funct
					} else if (instr[2] == 'a' || instr[2] == 'A') { // SRA
						machine = 0x0; // opcode
						mips_regs_to_machine(0, regs, &machine);
						machine += 0x3; // funct
					}
				} else if (instr[1] == 'w' || instr[1] == 'W') { // SW
					machine = 0x2B; // opcode
					machine <<= 26;
					mips_regs_to_machine(3, regs, &machine);
				} else if (instr[1] == 'b' || instr[1] == 'B') { // SB
					machine = 0x28; // opcode
					machine <<= 26;
					mips_regs_to_machine(3, regs, &machine);
				} else if (instr[1] == 'h' || instr[1] == 'H') { // SH
					machine = 0x29; // opcode
					machine <<= 26;
					mips_regs_to_machine(3, regs, &machine);
				} else if (instr[1] == 'y' || instr[1] == 'Y') { // SYSCALL
					machine = 0x0; // opcode
					machine += 0xC; // funct
				}
			break;

			// XORI, XOR
			case('x'):
			case('X'):
				if (instr[3] == 'i' || instr[2] == 'I') { // XORI
					machine = 0xE; // opcode
					machine <<= 26;
					mips_regs_to_machine(1, regs, &machine);
				} else { // XOR
					machine = 0x0; // opcode
					mips_regs_to_machine(0, regs, &machine);
					machine += 0x26; // funct
				}
			break;

			default:
				printf("Error[convert_mips]: Invalid MIPS instruction\n");
			break;
		}
		// Our machine instruction is now populated
		// Write the current instruction to the output file
		fprintf(fp_o, "%x\n", machine);
	}
	fclose(fp_i);
	fclose(fp_o);
	printf("Success: File generation complete.\n\n");
	return;
}

/************************************************************/
/* Determines mips register name using machine code register number           */ 
/************************************************************/
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

/************************************************************/
/* Determines machine code register number using mips register name      */ 
/************************************************************/
uint32_t decode_mips_register(char* reg_str) {
	uint32_t buffer = 0;
	char reg = reg_str[0];
	uint32_t reg_no = atoi(reg_str+1);

	switch(reg) {
		case ('z'):
		case ('Z'):
			buffer = 0x0;
		break;

		case ('a'):
		case ('A'):
			if (reg_no == 0) { // a0
				buffer = 0x4;
			} else if (reg_no == 1) { // a1
				buffer = 0x5;
			} else if (reg_no == 2) { // a2
				buffer = 0x6;
			} else if (reg_no == 3) { // a3
				buffer = 0x7;
			} else { // at
				buffer = 0x1;
			}
		break;

		case ('v'):
		case ('V'):
			if (reg_no == 0) { // v0
				buffer = 0x2;
			} else if (reg_no == 1) { // v1
				buffer = 0x3;
			} 
		break;

		case ('t'):
		case ('T'):
			if (reg_no == 0) { // t0
				buffer = 0x8;
			} else if (reg_no == 1) { // t1
				buffer = 0x9;
			} else if (reg_no == 2) { // t2
				buffer = 0xA;
			} else if (reg_no == 3) { // t3
				buffer = 0xB;
			} else if (reg_no == 4) { // t4
				buffer = 0xC;
			} else if (reg_no == 5) { // t5
				buffer = 0xD;
			} else if (reg_no == 6) { // t6
				buffer = 0xE;
			} else if (reg_no == 7) { // t7
				buffer = 0xF;
			} else if (reg_no == 8) { // t8
				buffer = 0x18;
			} else if (reg_no == 9) { // t9
				buffer = 0x19;
			}
		break;

		case ('s'):
		case ('S'):
			if (reg_no == 0) { // s0
				buffer = 0x10;
			} else if (reg_no == 1) { // s1
				buffer = 0x11;
			} else if (reg_no == 2) { // s2
				buffer = 0x12;
			} else if (reg_no == 3) { // s3
				buffer = 0x13;
			} else if (reg_no == 4) { // s4
				buffer = 0x14;
			} else if (reg_no == 5) { // s5
				buffer = 0x15;
			} else if (reg_no == 6) { // s6
				buffer = 0x16;
			} else if (reg_no == 7) { // s7
				buffer = 0x17;
			} else { // sp
				buffer = 0x1D;
			}
		break;

		case ('k'):
		case ('K'):
			if (reg_no == 0) { // k0
				buffer = 0x1A;
			} else if (reg_no == 1) { // k1
				buffer = 0x1B;
			}
		break;

		case ('g'):
		case ('G'):
			buffer = 0x1C; // gp
		break;

		case ('f'):
		case ('F'):
			buffer = 0x1E; // fp
		break;

		case ('r'):
		case ('R'):
			buffer = 0x1F; // ra
		break;

		default:
			printf("Error[decode_mips_register]: Register not found\n");
		break;
	}
	return buffer;
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

/************************************************************/
/* Generate output file with program loaded into memory             */ 
/************************************************************/
void write_file(){
	int i;
	char buffer[20];
	uint32_t addr;
	FILE* fp;

	printf("\nPlease enter a name for the file to be generated:  ");
	if (scanf("%s", buffer) == EOF){
		exit(0);
	}

	if ((fp = fopen(buffer, "w")) == NULL) {
		printf("\nError[write_file]: Could not write program to file\n");
		exit(0);
	}
	
	printf("\nWriting MIPS program to file...\n");
	for(i=0; i<PROGRAM_SIZE; i++){
		addr = MEM_TEXT_BEGIN + (i*4);
		write_instruction(fp, addr);
	}
	printf("Success: File generation complete.\n\n");

	fclose(fp);
}

/************************************************************/
/* Print the instruction at given memory address (in MIPS) to stdout */
/************************************************************/
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

	// Step 1: Need to read in the instruction at the given memory addr
	instruction = mem_read_32(addr);

	// Step 2: isolate instruction
	isolate_vars(instruction, &opcode, &rs, &rt, &rd, &shamt, &funct, &immediate, &address);

	// Step 4: Enter switch statements for R, J, and I respectively
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
			printf("j %x\n", (address << 2));
		break;

		case (0x3): // jal
			printf("j %x\n", (address << 2));
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
			temp = immediate;
			for (int j = 16; j < 32; j++) {
				temp += (1 << j);
			}
			temp <<= 2;
			temp += CURRENT_STATE.PC;
			
			if ( rt == 1 ) { // bgez
				printf("bgez ");

			} else if ( rt == 0 ) { // bltz
				printf("bltz ");
			} 
			decode_machine_register(rt, reg_str);
			printf("%s, %x\n", reg_str, temp);
		break;
		}

		case (0x4): { // beq
			temp = immediate;
			for (int j = 16; j < 32; j++) {
				temp += (1 << j);
			}
			temp <<= 2;
			temp += CURRENT_STATE.PC;

			printf("beq ");
			decode_machine_register(rs, reg_str);
			printf("%s, ", reg_str);
			decode_machine_register(rt, reg_str);
			printf("%s, %x\n", reg_str, temp);
		break;
		}

		case (0x5): { // bne
			temp = immediate;
			for (int j = 16; j < 32; j++) {
				temp += (1 << j);
			}
			temp <<= 2;
			temp += CURRENT_STATE.PC;
			
			printf("bne ");
			decode_machine_register(rs, reg_str);
			printf("%s, ", reg_str);
			decode_machine_register(rt, reg_str);
			printf("%s, %x\n", reg_str, temp);
		break;
		}

		case (0x6): { // blez
			temp = immediate;
			for (int j = 16; j < 32; j++) {
				temp += (1 << j);
			}
			temp <<= 2;
			temp += CURRENT_STATE.PC;			
			
			printf("blez ");
			decode_machine_register(rs, reg_str);
			printf("%s, %x\n", reg_str, temp);
		break;
		}

		case (0x7): { // bgtz
			temp = immediate;
			for (int j = 16; j < 32; j++) {
				temp += (1 << j);
			}
			temp <<= 2;
			temp += CURRENT_STATE.PC;

			printf("bgtz ");
			decode_machine_register(rs, reg_str);
			printf("%s, %x\n", reg_str, temp);
		break;
		}

		default:
			printf("Error[print_instruction]: Invalid instruction at memory location\n");
		break;
	}
}

/************************************************************/
/* Print the instruction at given memory address (in MIPS) to file    */
/************************************************************/
void write_instruction(FILE* fp, uint32_t addr) {
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

	// Step 1: Need to read in the instruction at the given memory addr
	instruction = mem_read_32(addr);

	// Step 2: isolate instruction
	isolate_vars(instruction, &opcode, &rs, &rt, &rd, &shamt, &funct, &immediate, &address);

	// Step 4: Enter switch statements for R, J, and I respectively
	switch(opcode) {
	
		case (0x0): {  // R type
			// [ op - 6][ rs - 5 ][ rt - 5 ][ rd - 5 ][ shamt - 5 ][ funct - 6 ]
			switch(funct) {
				case (0x20): // add
					fprintf(fp, "add ");
					decode_machine_register(rd, reg_str);
					fprintf(fp, "%s, ", reg_str);
					decode_machine_register(rs, reg_str);
					fprintf(fp, "%s, ", reg_str);
					decode_machine_register(rt, reg_str);
					fprintf(fp, "%s\n", reg_str);
				break;

				case (0x21): // addu
					fprintf(fp, "addu ");
					decode_machine_register(rd, reg_str);
					fprintf(fp, "%s, ", reg_str);
					decode_machine_register(rs, reg_str);
					fprintf(fp, "%s, ", reg_str);
					decode_machine_register(rt, reg_str);
					fprintf(fp, "%s\n", reg_str);
				break;

				case (0x22): // sub
					fprintf(fp, "sub ");
					decode_machine_register(rd, reg_str);
					fprintf(fp, "%s, ", reg_str);
					decode_machine_register(rs, reg_str);
					fprintf(fp, "%s, ", reg_str);
					decode_machine_register(rt, reg_str);
					fprintf(fp, "%s\n", reg_str);
				break;

				case (0x23): // subu
					fprintf(fp, "subu ");
					decode_machine_register(rd, reg_str);
					fprintf(fp, "%s, ", reg_str);
					decode_machine_register(rs, reg_str);
					fprintf(fp, "%s, ", reg_str);
					decode_machine_register(rt, reg_str);
					fprintf(fp, "%s\n", reg_str);
				break;

				case (0x24): // and
					fprintf(fp, "and ");
					decode_machine_register(rd, reg_str);
					fprintf(fp, "%s, ", reg_str);
					decode_machine_register(rs, reg_str);
					fprintf(fp, "%s, ", reg_str);
					decode_machine_register(rt, reg_str);
					fprintf(fp, "%s\n", reg_str);
				break;

				case (0x25): // or
					fprintf(fp, "or ");
					decode_machine_register(rd, reg_str);
					fprintf(fp, "%s, ", reg_str);
					decode_machine_register(rs, reg_str);
					fprintf(fp, "%s, ", reg_str);
					decode_machine_register(rt, reg_str);
					fprintf(fp, "%s\n", reg_str);
				break;

				case (0x26): // xor	
					fprintf(fp, "xor ");
					decode_machine_register(rd, reg_str);
					fprintf(fp, "%s, ", reg_str);
					decode_machine_register(rs, reg_str);
					fprintf(fp, "%s, ", reg_str);
					decode_machine_register(rt, reg_str);
					fprintf(fp, "%s\n", reg_str);
				break;

				case (0x27): // nor	
					fprintf(fp, "nor ");
					decode_machine_register(rd, reg_str);
					fprintf(fp, "%s, ", reg_str);
					decode_machine_register(rs, reg_str);
					fprintf(fp, "%s, ", reg_str);
					decode_machine_register(rt, reg_str);
					fprintf(fp, "%s\n", reg_str);
				break;

				case (0x18): // mult
					fprintf(fp, "mult ");
					decode_machine_register(rs, reg_str);
					fprintf(fp, "%s, ", reg_str);
					decode_machine_register(rt, reg_str);
					fprintf(fp, "%s\n", reg_str);
				break;

				case (0x19): // multu
					fprintf(fp, "multu ");
					decode_machine_register(rs, reg_str);
					fprintf(fp, "%s, ", reg_str);
					decode_machine_register(rt, reg_str);
					fprintf(fp, "%s\n", reg_str);
				break;

				case (0x1A): // div
					fprintf(fp, "div ");
					decode_machine_register(rs, reg_str);
					fprintf(fp, "%s, ", reg_str);
					decode_machine_register(rt, reg_str);
					fprintf(fp, "%s\n", reg_str);
				break;

				case (0x1B): // divu
					fprintf(fp, "divu ");
					decode_machine_register(rs, reg_str);
					fprintf(fp, "%s, ", reg_str);
					decode_machine_register(rt, reg_str);
					fprintf(fp, "%s\n", reg_str);
				break;

				case (0x2A): // slt
					fprintf(fp, "slt ");
					decode_machine_register(rd, reg_str);
					fprintf(fp, "%s, ", reg_str);
					decode_machine_register(rs, reg_str);
					fprintf(fp, "%s, ", reg_str);
					decode_machine_register(rt, reg_str);
					fprintf(fp, "%s\n", reg_str);
				break;

				case (0x00): // sll
					fprintf(fp, "sll ");
					decode_machine_register(rd, reg_str);
					fprintf(fp, "%s, ", reg_str);
					decode_machine_register(rt, reg_str);
					fprintf(fp, "%s, %d\n", reg_str, shamt);
				break;

				case (0x02): // srl
					fprintf(fp, "srl ");
					decode_machine_register(rd, reg_str);
					fprintf(fp, "%s, ", reg_str);
					decode_machine_register(rt, reg_str);
					fprintf(fp, "%s, %d\n", reg_str, shamt);
				break;

				case (0x3):  // sra
					fprintf(fp, "sra ");
					decode_machine_register(rd, reg_str);
					fprintf(fp, "%s, ", reg_str);
					decode_machine_register(rt, reg_str);
					fprintf(fp, "%s, %d\n", reg_str, shamt);
				break;

				case (0x10): // mfhi
					fprintf(fp, "mfhi ");
					decode_machine_register(rd, reg_str);
					fprintf(fp, "%s\n", reg_str);
				break;

				case (0x12): // mflo
					fprintf(fp, "mflo ");
					decode_machine_register(rd, reg_str);
					fprintf(fp, "%s\n", reg_str);
				break;

				case (0x11): // mthi
					fprintf(fp, "mthi ");
					decode_machine_register(rs, reg_str);
					fprintf(fp, "%s\n", reg_str);
				break;

				case (0x13): // mtlo
					fprintf(fp, "mtlo ");
					decode_machine_register(rs, reg_str);
					fprintf(fp, "%s\n", reg_str);
				break;

				case (0x8):  // jr
					fprintf(fp, "jr ");
					decode_machine_register(rs, reg_str);
					fprintf(fp, "%s\n", reg_str);
				break;

				case (0x9):  // jalr	
					fprintf(fp, "jalr ");
					if (rd == 31) {
						// format is rs
						decode_machine_register(rs, reg_str);
						fprintf(fp, "%s\n", reg_str);
					} else {
						// format is rd, rs
						decode_machine_register(rd, reg_str);
						fprintf(fp, "%s, ", reg_str);
						decode_machine_register(rs, reg_str);
						fprintf(fp, "%s\n", reg_str);
					}
				break;

				case (0xC):  // syscall
					fprintf(fp, "syscall\n");
				break;

				default:
					printf("Error[write_instruction]: Invalid instruction (R)\n");
				break;
			}
		break;
		}

		// J-format
		// [ op - 6][        const/address - 26        ]
		case (0x2): // j
			fprintf(fp, "j %d\n", (address << 2));
		break;

		case (0x3): // jal
			fprintf(fp, "j %d\n", (address << 2));
		break;

		// I-format
		// [ op - 6][ rs - 5 ][ rt - 5 ][    immmediate - 16    ]
		case (0x8):  // addi
			fprintf(fp, "addi ");
			decode_machine_register(rt, reg_str);
			fprintf(fp, "%s, ", reg_str);
			decode_machine_register(rs, reg_str);
			fprintf(fp, "%s, %d\n", reg_str, immediate);	
		break;

		case (0x9):  // addiu
			fprintf(fp, "addiu ");
			decode_machine_register(rt, reg_str);
			fprintf(fp, "%s, ", reg_str);
			decode_machine_register(rs, reg_str);
			fprintf(fp, "%s, %d\n", reg_str, immediate);	
		break;

		case (0xC):  // andi
			fprintf(fp, "andi ");
			decode_machine_register(rt, reg_str);
			fprintf(fp, "%s, ", reg_str);
			decode_machine_register(rs, reg_str);
			fprintf(fp, "%s, %d\n", reg_str, immediate);	
		break;

		case (0xD):  // ori
			fprintf(fp, "ori ");
			decode_machine_register(rt, reg_str);
			fprintf(fp, "%s, ", reg_str);
			decode_machine_register(rs, reg_str);
			fprintf(fp, "%s, %d\n", reg_str, immediate);	
		break;

		case (0xE):  // xori
			fprintf(fp, "xori ");
			decode_machine_register(rt, reg_str);
			fprintf(fp, "%s, ", reg_str);
			decode_machine_register(rs, reg_str);
			fprintf(fp, "%s, %d\n", reg_str, immediate);	
		break;

		case (0xA):  // slti
			fprintf(fp, "slti ");
			decode_machine_register(rt, reg_str);
			fprintf(fp, "%s, ", reg_str);
			decode_machine_register(rs, reg_str);
			fprintf(fp, "%s, %d\n", reg_str, immediate);	
		break;

		case (0x23): // lw
			fprintf(fp, "lw ");
			decode_machine_register(rt, reg_str);
			fprintf(fp, "%s, ", reg_str);
			decode_machine_register(rs, reg_str);
			fprintf(fp, "%d(%s)\n", immediate, reg_str);	
		break;

		case (0x32): // lb
			fprintf(fp, "lb ");
			decode_machine_register(rt, reg_str);
			fprintf(fp, "%s, ", reg_str);
			decode_machine_register(rs, reg_str);
			fprintf(fp, "%d(%s)\n", immediate, reg_str);	
		break;

		case (0x36): // lh
			fprintf(fp, "lh ");
			decode_machine_register(rt, reg_str);
			fprintf(fp, "%s, ", reg_str);
			decode_machine_register(rs, reg_str);
			fprintf(fp, "%d(%s)\n", immediate, reg_str);	
		break;

		case (0xF):  // lui
			fprintf(fp, "lui ");
			decode_machine_register(rt, reg_str);
			fprintf(fp, "%s, %d\n", reg_str, immediate);
		break;

		case (0x2B): // sw
			fprintf(fp, "sw ");
			decode_machine_register(rt, reg_str);
			fprintf(fp, "%s, ", reg_str);
			decode_machine_register(rs, reg_str);
			fprintf(fp, "%d(%s)\n", immediate, reg_str);	
		break;

		case (0x28): // sb
			fprintf(fp, "sb ");
			decode_machine_register(rt, reg_str);
			fprintf(fp, "%s, ", reg_str);
			decode_machine_register(rs, reg_str);
			fprintf(fp, "%d(%s)\n", immediate, reg_str);	
		break;

		case (0x29): // sh 
			fprintf(fp, "sh ");
			decode_machine_register(rt, reg_str);
			fprintf(fp, "%s, ", reg_str);
			decode_machine_register(rs, reg_str);
			fprintf(fp, "%d(%s)\n", immediate, reg_str);	
		break;

		case (0x1): { // bltz or bgez
			temp = immediate;
			for (int j = 16; j < 32; j++) {
				temp += (1 << j);
			}
			temp <<= 2;
			temp += CURRENT_STATE.PC;
			
			if ( rt == 1 ) { // bgez
				fprintf(fp, "bgez ");

			} else if ( rt == 0 ) { // bltz
				fprintf(fp, "bltz ");
			} 
			decode_machine_register(rt, reg_str);
			fprintf(fp, "%s, 0x%x\n", reg_str, temp);
		break;
		}

		case (0x4): { // beq
			temp = immediate;

			fprintf(fp, "beq ");
			decode_machine_register(rs, reg_str);
			fprintf(fp, "%s, ", reg_str);
			decode_machine_register(rt, reg_str);
			fprintf(fp, "%s, 0x%x\n", reg_str, temp);
		break;
		}

		case (0x5): { // bne
			temp = immediate;
			
			fprintf(fp, "bne ");
			decode_machine_register(rs, reg_str);
			fprintf(fp, "%s, ", reg_str);
			decode_machine_register(rt, reg_str);
			fprintf(fp, "%s, 0x%x\n", reg_str, temp);
		break;
		}

		case (0x6): { // blez
			temp = immediate;		
			
			fprintf(fp, "blez ");
			decode_machine_register(rs, reg_str);
			fprintf(fp, "%s, 0x%x\n", reg_str, temp);
		break;
		}

		case (0x7): { // bgtz
			temp = immediate;

			fprintf(fp, "bgtz ");
			decode_machine_register(rs, reg_str);
			fprintf(fp, "%s, 0x%x\n", reg_str, temp);
		break;
		}

		default:
			printf("Error[write_instruction]: Invalid instruction\n");
		break;
	}

	return;
}

/***************************************************************/
/* Converts register portion of mips intstruction to machine code based on format    */
/* R-format == 0, I-format == 1, J-format == 2, I-Special == 3                       */
/***************************************************************/
void mips_regs_to_machine(int format, char* mips_instr, uint32_t* machine_instr) {
	int count = 0;
	uint32_t reg_temp;
	uint32_t r0 = 0x20; // Register ranges are 0-31, so 32 is our invalid value
	uint32_t r1 = 0x20;
	uint32_t r2 = 0x20; 
	uint32_t immediate = 0;
	uint32_t address = 0;
	uint32_t shamt = 0; 
	const char delim[2] = ",";
	char* temp;
	char* save = mips_instr;

	switch (format) {
		// R-Format
		// [ op - 6][ rs - 5 ][ rt - 5 ][ rd - 5 ][ shamt - 5 ][ funct - 6 ]
		case 0: {
			while ((temp = strtok_r(save, delim, &save))) {
				if (count > 0) {
					temp++;
				}

				if (count == 2 && *(temp) != '$') { 
					// Then we have a shamt
					shamt = atoi(temp);

				} else {
					// Typical case
					// Determine which register
					reg_temp = decode_mips_register(temp+1);

					switch (count) {
						case (0):
							r0 = reg_temp;
						break;

						case (1):
							r1 = reg_temp;
						break;

						case (2):
							r2 = reg_temp;
						break;

						default:
							printf("Error[mips_regs_to_machine]: Trying to get too many registers\n");
						break;
					}
				}
				count++;
			}

		// Add rs, rt, rd, shamt all into the instruction to be returned
		// The instruction should then contain all but last 6 bits (funct)
		if (r0 != 0x20 && r1 == 0x20 && r2 == 0x20) {
			// just have an rd
			reg_temp = r0;
			reg_temp <<= 11;
			*machine_instr += reg_temp;
		} else if (r0 != 0x20 && r1 != 0x20 && r2 == 0x20) {
			if (shamt != 0x0) {
				// rd rt and shamt
				reg_temp = r1;
				reg_temp <<= 16;
				*machine_instr += reg_temp;
				reg_temp = r0;
				reg_temp <<= 11;
				*machine_instr += reg_temp;
				reg_temp = shamt;
				reg_temp <<= 6;
				*machine_instr += reg_temp;
			} else {
				// rs and rt
				reg_temp = r0;
				reg_temp <<= 21;
				*machine_instr += reg_temp;
				reg_temp = r1;
				reg_temp <<= 16;
				*machine_instr += reg_temp;
			}
		} else {
			// Typical case, rd rs rt
			reg_temp = r1;
			reg_temp <<= 21;
			*machine_instr += reg_temp;
			reg_temp = r2;
			reg_temp <<= 16;
			*machine_instr += reg_temp;
			reg_temp = r0;
			reg_temp <<= 11;
			*machine_instr += reg_temp;
		}
		break; }

		// I-Format
		// NOTE: this case assumes typical format for I-instruction is being used
		// [ op - 6][ rs - 5 ][ rt - 5 ][    immmediate - 16    ]
		case 1: {
			int branch_flag = 0;
			while ((temp = strtok_r(save, delim, &save))) {
				if (count > 0) {
					temp++;
				}		
				
				if (count == 1 && *(temp) != '$') {
					branch_flag = 1;
					immediate = strtol(temp, NULL, 16);
					break; 
				} else if (count == 2 && *(temp) != '$') {
					immediate = atoi(temp);
					break;
				} else {
					// Determine which register
					reg_temp = decode_mips_register(temp+1);
				}

				switch (count) {
					case (0):
						r0 = reg_temp;
					break;

					case (1):
						r1 = reg_temp;
					break;

					default:
						printf("Error[mips_regs_to_machine]: Trying to get too many registers\n");
					break;
				}
				count++;
			}

			if (branch_flag == 1) {
				reg_temp = r0;
				reg_temp <<= 21;
				*machine_instr += reg_temp;
				*machine_instr += immediate;
			} else {
				reg_temp = r1;
				reg_temp <<= 21;
				*machine_instr += reg_temp;
				reg_temp = r0;
				reg_temp <<= 16;
				*machine_instr += reg_temp;
				*machine_instr += immediate;
			}
		break;
		}

		// J-Format
		// [ op - 6][        const/address - 26        ]
		case 2:
			// Value stored in machine is low-order 26 bits (addr / 4)
			address = atoi(mips_instr) / 4;
			*machine_instr += address;
		break;

		// I-Format Exceptions (Special cases)
		case 3: {
			char delim1[2] = ")";

			while ((temp = strtok_r(save, delim, &save))) {
				if (count > 0) {
					temp++;
				}

				if (count == 1 && *(temp) != '$') { 
					// either $reg0 imm($reg1) or $reg0 imm
					char* offs = strchr(temp, '(');
					
					// $rt imm($rs)
					if (offs != NULL) { 
						char* t = strtok(temp, delim1);
						int len = strlen(t);
						immediate = atoi(t);

						reg_temp = decode_mips_register((temp + (len-2))); // rs
						reg_temp <<= 21;
						*machine_instr += reg_temp;
						reg_temp = r0; // rt
						reg_temp <<= 16;
						*machine_instr += reg_temp;
						*machine_instr += immediate; // may need to shift this
						break;
					
					// $rt imm
					} else { 
						reg_temp = r0; // rt
						reg_temp <<= 16;
						*machine_instr += reg_temp;
						immediate = atoi(temp);
						*machine_instr += immediate;
						break;
					}

				} else {
					// $rs $rt imm
					if (count == 2 && *(temp) != '$') {
						immediate = strtol(temp, NULL, 16);
						// Add rt rs imm all into the instruction to be returned
						reg_temp = r0;
						reg_temp <<= 21;
						*machine_instr += reg_temp;
						reg_temp = r1;
						reg_temp <<= 16;
						*machine_instr += reg_temp;
						*machine_instr += immediate;
						break;
					} else {
						// Determine which register
						reg_temp = decode_mips_register(temp+1);
					}

					switch (count) {
						case (0):
							r0 = reg_temp;
						break;

						case (1):
							r1 = reg_temp;
						break;

						default:
							printf("Error[mips_regs_to_machine]: Trying to get too many registers\n");
						break;
					}
				}
				count++;
			}
		break;
		}

		default:
			printf("Error[mips_regs_to_machine]: Invalid format type (%d)\n", format);
		break;
	}
	return;
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