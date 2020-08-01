#include "Parser.h"

// FIXME, implement this function.
// Here shows an example on how to translate "add x10, x10, x25"
void loadInstructions(Instruction_Memory *i_mem, const char *trace)
{
    //printf("Loading trace file: %s\n", trace);

    FILE *fd = fopen(trace, "r");
    if (fd == NULL)
    {
        perror("Cannot open trace file. \n");
        exit(EXIT_FAILURE);
    }

    // Iterate all the assembly instructions
    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    Addr PC = 0; // program counter points to the zeroth location initially.
    int IMEM_index = 0;
    while ((read = getline(&line, &len, fd)) != -1)
    {
        // Assign program counter
        i_mem->instructions[IMEM_index].addr = PC;

        // Extract operation for R-Type
        char *raw_instr = strtok(line, " ");
        if (strcmp(raw_instr, "add") == 0 ||
            strcmp(raw_instr, "sub") == 0 ||
            strcmp(raw_instr, "sll") == 0 ||
            strcmp(raw_instr, "srl") == 0 ||
            strcmp(raw_instr, "xor") == 0 ||
            strcmp(raw_instr, "or")  == 0 ||
            strcmp(raw_instr, "and") == 0)
        {
		//printf("The type of instruction is: %s\n", raw_instr);
            parseRType(raw_instr, &(i_mem->instructions[IMEM_index]));
            i_mem->last = &(i_mem->instructions[IMEM_index]);
        }
		
		// Extract operation for I-Type		
	else if (strcmp(raw_instr, "addi") == 0 ||
            strcmp(raw_instr, "slli") == 0 ||
             strcmp(raw_instr, "ld") == 0 )
        {
		//printf("The type of instruction is: %s\n", raw_instr);
    		parseIType(raw_instr, &(i_mem->instructions[IMEM_index]));
            i_mem->last = &(i_mem->instructions[IMEM_index]);
        }
		
		
		// Extract operation for SB-Type		
     else if (strcmp(raw_instr, "bne") == 0  )
        {
            parseSBType(raw_instr, &(i_mem->instructions[IMEM_index]));
            i_mem->last = &(i_mem->instructions[IMEM_index]);
        } 

        IMEM_index++;
 PC += 4;
    }

    fclose(fd);
}





void parseRType(char *opr, Instruction *instr)
{
    instr->instruction = 0;
    unsigned opcode = 0;
    unsigned funct3 = 0;
    unsigned funct7 = 0;

    if (strcmp(opr, "add") == 0)
    {
        opcode = 51;
        funct3 = 0;
        funct7 = 0;
    }

    char *reg = strtok(NULL, ", ");
    unsigned rd = regIndex(reg);

    reg = strtok(NULL, ", ");
    unsigned rs_1 = regIndex(reg);

    reg = strtok(NULL, ", ");
    reg[strlen(reg)-1] = '\0';
    unsigned rs_2 = regIndex(reg);

    // Contruct instruction
    instr->instruction |= opcode;
    instr->instruction |= (rd << 7);
    instr->instruction |= (funct3 << (7 + 5));
    instr->instruction |= (rs_1 << (7 + 5 + 3));
    instr->instruction |= (rs_2 << (7 + 5 + 3 + 5));
    instr->instruction |= (funct7 << (7 + 5 + 3 + 5 + 5));
}




void parseIType(char *opr, Instruction *instr)
{
    instr->instruction = 0;
    unsigned opcode = 0;
    unsigned funct3 = 0;
    unsigned immediate = 0;

    if (strcmp(opr, "addi") == 0)
    {
        opcode = 19;
        funct3 = 0;       
    

		char *reg = strtok(NULL, ", ");
		unsigned rd = regIndex(reg);

		reg = strtok(NULL, ", ");
		unsigned rs_1 = regIndex(reg);

		reg = strtok(NULL, ", ");
		reg[strlen(reg)-1] = '\0';
		unsigned imm = atoi(reg);
		

		// Contruct instruction
		instr->instruction |= opcode;
		instr->instruction |= (rd << 7);
		instr->instruction |= (funct3 << (7 + 5));
		instr->instruction |= (rs_1 << (7 + 5 + 3));
		instr->instruction |= (imm << (7 + 5 + 3 + 5));		
	}
	else if (strcmp(opr, "ld") == 0 )
	{
		opcode = 3;
        funct3 = 3;       
		  

		char *reg = strtok(NULL, ", ");
		char *immChar = strtok(NULL, "(");
		unsigned imm = atoi(immChar);
		unsigned rd = regIndex(reg);

		reg = strtok(NULL, ")");
		unsigned rs_1 = regIndex(reg);

		// Contruct instruction
		instr->instruction |= opcode;
		instr->instruction |= (rd << 7);
		instr->instruction |= (funct3 << (7 + 5));
		instr->instruction |= (rs_1 << (7 + 5 + 3));
		instr->instruction |= (imm << (7 + 5 + 3 + 5));	
		
	}
	
	if (strcmp(opr, "slli") == 0)
    {
        opcode = 19;
        funct3 = 1;       
    

		char *reg = strtok(NULL, ", ");
		unsigned rd = regIndex(reg);

		reg = strtok(NULL, ", ");
		unsigned rs_1 = regIndex(reg);

		reg = strtok(NULL, ", ");
		reg[strlen(reg)-1] = '\0';
		unsigned shamt = atoi(reg);

		// Contruct instruction
		instr->instruction |= opcode;
		instr->instruction |= (rd << 7);
		instr->instruction |= (funct3 << (7 + 5));
		instr->instruction |= (rs_1 << (7 + 5 + 3));
		instr->instruction |= (shamt << (7 + 5 + 3 + 5));		
		instr->instruction |= (0 << (7 + 5 + 3 + 5 + 5));		
		
	}
}


 void parseSBType(char *opr, Instruction *instr)
{
    instr->instruction = 0;
    unsigned opcode = 0;
    unsigned funct3 = 0;    
    if (strcmp(opr, "bne") == 0)
    {
        opcode = 103;
        funct3 = 1;       
    
		char *reg = strtok(NULL, ", ");
		unsigned rs_1 = regIndex(reg);
		reg = strtok(NULL, ", ");
		unsigned rs_2 = regIndex(reg);
		reg = strtok(NULL, ", ");
		reg[strlen(reg)-1] = '\0';		
		int imm = atoi(reg);
		//printf("The imm initial number is %d\n",imm);
		int neg = 0;
		unsigned int imm1, imm2, imm3, imm4;
		if(imm < 0)
		{
			//two's comp 
			imm = (~abs(imm) )+1;
			neg = 1;
			
		}
		//printf("The twos comp of the imm is %d\n",imm);
		// Contruct instruction				
		imm1 = imm; // imm 11
		imm1 = (imm1 & (1 << 10)) ;
		imm1 = imm1 >> 10;	
		imm2 = imm; // imm 4:1
		imm2 = 15&imm2;// [4:1]
		imm3 = imm;
		imm3 = 63&(imm3>>4); // imm 10:5
		
		instr->instruction |= opcode;		
		instr->instruction |= (imm1 << 7);
		instr->instruction |= (imm2 << (7 + 1));		
		instr->instruction |= (funct3 << (7 + 1 +  4));
		instr->instruction |= (rs_1 << (7 + 1 +  4 + 3));		
		instr->instruction |= (rs_2 << (7+1+4+3+5));
		instr->instruction |= (imm3 << (7+1+4+3+5+5));	
		instr->instruction |= (neg << (7+1+4+3+5+5+6));			
		
	
	
		
	}
}
 





int regIndex(char *reg)
{
    unsigned i = 0;
                                            
for (i; i < NUM_OF_REGS; i++)
    {
        if (strcmp(REGISTER_NAME[i], reg) == 0)
        {
            break;
        }
    }

    return i;
}
