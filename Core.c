#include "Core.h"
#include "Registers.h"

Core *initCore(Instruction_Memory *i_mem)
{
    Core *core = (Core *)malloc(sizeof(Core));
    core->clk = 0;
    core->PC = 0;
    core->instr_mem = i_mem;
    core->tick = tickFunc;

    // FIXME, initialize register file here.
    // core->data_mem[0] = ...
	core -> data_mem[0] = 16;
	core -> data_mem[1] = 128;
	core -> data_mem[2] = 8;
	core -> data_mem[3] = 4;

    // FIXME, initialize data memory here.
    core->reg_file[25] = 4;
	core->reg_file[10] = 4;
	core->reg_file[22] = 1;

    return core;
}

// FIXME, implement this function
bool tickFunc(Core *core)
{           
	// *********** organized by processing stages (lecture notes #9 )****************
	
	//** instruction fetch **
	// Steps may include
    // (Step 1) Reading instruction from instruction memory
    unsigned instruction = core->instr_mem->instructions[core->PC / 4].instruction;
	Signal incremented_instruction = core->PC += 4;	
	//** decoding / reg reading  **
	
	// call control unit 
	Signal control_unit_input = (instruction / 64);
	ControlSignals *signals;
	ControlUnit(control_unit_input, signals);
	// run immGen 
	Signal ImmeGen_sig = ImmeGen(instruction); // <------------------------------------ not finished, fix this!!!!!!!!!!!!!
	
	//get reg values
	
	// get inputs for reg file from instructions
	int reg_index_1,reg_index_2,write_register;
	reg_index_1 = (instruction / 524288)>>15;
	reg_index_2 = (instruction / 16777216)>>20;
	write_register = (instruction / 2048)>>7;
	Signal reg_read_1, reg_read_2;
	if ( signals->RegWrite== 0 )
	{
		reg_read_1 = core->reg_file[reg_index_1];
		reg_read_2 = core->reg_file[reg_index_2];
	}
	else if (signals->RegWrite == 1)		
	{
		core->reg_file[reg_index_1] = 0;// result of memory manipulation Mux all the way to the right
	
	}
	
	//call 
	// ** execute / address calc
	// mux1
	Signal mux_1_signal = MUX( signals->ALUSrc, reg_read_2,ImmeGen_sig);
			
	
	
	//Alu control 
	Signal aluControlResult = ALUControlUnit(signals->ALUOp, instruction>>24,instruction >> 11);
	// alu 	
	Signal *ALU_result = 0 ;
	Signal *zero = 0;
	ALU(reg_read_1,mux_1_signal,aluControlResult, ALU_result, zero);
	
	Signal branch_location = Add(incremented_instruction,aluControlResult<<1);
	//** memory access 
	
	Signal memory_result ;
	memory_result = 0; // <----------------------- will fix when  dealing with ld 
	Signal mux_2_signal = MUX(signals->MemtoReg, *ALU_result, memory_result); // fix this 
	// <-------------------- figure out how i type loads results in  register 
	
	
	
	// mux 3
	Signal mux_3_control = *zero && signals->Branch ;
	Signal mux_3_signal = MUX( mux_3_control,incremented_instruction,branch_location);
	//write results
	core->reg_file[write_register] = mux_3_signal;
	incremented_instruction = core->PC = mux_3_signal;
	printf("The data in register %x is %x",write_register, core->reg_file[write_register]);
    

    ++core->clk;
    // Are we reaching the final instruction?
    if (core->PC > core->instr_mem->last->addr)
    {
        return false;
    }
    return true;
}

// FIXME (1). Control Unit. Refer to Figure 4.18.
void ControlUnit(Signal input,
                 ControlSignals *signals)
{
    // For R-type
    if (input == 51)
    {
        signals->ALUSrc = 0;
        signals->MemtoReg = 0;
        signals->RegWrite = 1;
        signals->MemRead = 0;
        signals->MemWrite = 0;
        signals->Branch = 0;
        signals->ALUOp = 2;
    }
	// For I-type -- ld
    if (input == 3)
    {
        signals->ALUSrc = 1;
        signals->MemtoReg = 1;
        signals->RegWrite = 1;
        signals->MemRead = 1;
        signals->MemWrite = 0;
        signals->Branch = 0;
        signals->ALUOp = 3;
    }
	
	// For I-type -- slli <------------------ fix this
    if (input == 19)
    {
        signals->ALUSrc = 1;
        signals->MemtoReg = 0;
        signals->RegWrite = 1;
        signals->MemRead = 1;
        signals->MemWrite = 0;
        signals->Branch = 0;
        signals->ALUOp = 3;
    }
	
	// For sb-type -- bne <--------------------- fix this
    if (input == 99)
    {
        signals->ALUSrc = 0;
        signals->MemtoReg = 0;
        signals->RegWrite = 0;
        signals->MemRead = 0;
        signals->MemWrite = 0;
        signals->Branch = 1;
        signals->ALUOp = 0;
    }
}

// FIXME (2). ALU Control Unit. Refer to Figure 4.12.
Signal ALUControlUnit(Signal ALUOp,
                      Signal Funct7,
                      Signal Funct3)
{
    // For add
    if (ALUOp == 2 && Funct7 == 0 && Funct3 == 0)
    {
        return 2;
    }
	// For ldd
    if (ALUOp == 2 && Funct3 == 7)
    {
        return 3;
    }
	
}

// FIXME (3). Imme. Generator
Signal ImmeGen(Signal input)
{
	long ImmeGen = input;
}

// FIXME (4). ALU
void ALU(Signal input_0,
         Signal input_1,
         Signal ALU_ctrl_signal,
         Signal *ALU_result,
         Signal *zero)
{
    // For addition
    if (ALU_ctrl_signal == 2)
    {
        *ALU_result = (input_0 + input_1);
        if (*ALU_result == 0) { *zero = 1; } else { *zero = 0; }
    }
	// For ld
    if (ALU_ctrl_signal == 3)
    {
        *ALU_result = input_1;
        if (*ALU_result == 0) { *zero = 1; } else { *zero = 0; }
    }
	
}

// (4). MUX
Signal MUX(Signal sel,
           Signal input_0,
           Signal input_1)
{
    if (sel == 0) { return input_0; } else { return input_1; }
}

// (5). Add
Signal Add(Signal input_0,
           Signal input_1)
{
    return (input_0 + input_1);
}

// (6). ShiftLeft1
Signal ShiftLeft1(Signal input)
{
    return input << 1;
}
