#include "Core.h"

Core *initCore(Instruction_Memory *i_mem)
{
    Core *core = (Core *)malloc(sizeof(Core));
    core->clk = 0;
    core->PC = 0;
    core->instr_mem = i_mem;
    core->tick = tickFunc;

    // FIXME, initialize register file here.
    // core->data_mem[0] = ...
    
    //data memory setting for uint64_t arr[] = {16, 128, 8, 4}

    //uint64_t arr[] = {16, 128, 8, 4};


    core->data_mem[0] = 16;
    core->data_mem[1] = 0;
    core->data_mem[2] = 0;
    core->data_mem[3] = 0;
    core->data_mem[4] = 0;
    core->data_mem[5] = 0;
    core->data_mem[6] = 0;
    core->data_mem[7] = 0;


    core->data_mem[8] = 128;
    core->data_mem[9] = 0;
    core->data_mem[10] = 0;
    core->data_mem[11] = 0;
    core->data_mem[12] = 0;
    core->data_mem[13] = 0;
    core->data_mem[14] = 0;
    core->data_mem[15] = 0;

    core->data_mem[16] = 8;
    core->data_mem[17] = 0;
    core->data_mem[18] = 0;
    core->data_mem[19] = 0;
    core->data_mem[20] = 0;
    core->data_mem[21] = 0;
    core->data_mem[22] = 0; 
    core->data_mem[23] = 0;

    core->data_mem[24] = 4;
    core->data_mem[25] = 0;
    core->data_mem[26] = 0;
    core->data_mem[27] = 0;
    core->data_mem[28] = 0;
    core->data_mem[29] = 0;
    core->data_mem[30] = 0;
    core->data_mem[31] = 0;



    // FIXME, initialize data memory here.
    // core->reg_file[0] = ...

    //register file setting
    core->reg_file[25] = 4;
    core->reg_file[10] = 4;
    core->reg_file[22] = 1;

    return core;
}

// FIXME, implement this function
bool tickFunc(Core *core)
{
    // Steps may include
    // (Step 1) Reading instruction from instruction memory
    unsigned instruction = core->instr_mem->instructions[core->PC / 4].instruction;
    printf("Instruction: %u\n", instruction);
    // (Step 2) ...

    Signal input = instruction & 127;
    printf("Opcode: %ld\n", input); 

    ControlSignals signals;
    ControlUnit(input, &signals);

    Signal Funct3 = (instruction >> (7 + 5)) & 7;
    Signal Funct7 = (instruction >> (7 + 5 + 3 + 5 + 5)) & 127;
    Signal ALU_ctrl_signal = ALUControlUnit(signals.ALUOp, Funct7, Funct3);

    Register r1 = (instruction >> (7 + 5 + 3)) & 31;
    Register r2 = (instruction >> (7 + 5 + 3 + 5)) & 31;

    //create signal input to ALU from read data 1 output
    Signal input_0;
    input_0 = core->reg_file[r1];

    Signal input_1 = MUX(signals.ALUSrc,core->reg_file[r2],ImmeGen(instruction));
    Signal ALU_result;
    Signal zero;

    ALU(input_0, input_1, ALU_ctrl_signal, &ALU_result, &zero);
    printf("ALU out: %ld\n", ALU_result);

    Register write_reg = (instruction >> 7) & 31;

    if(signals.MemWrite)
    {
        core->data_mem[ALU_result] = input_1;
    }

    Signal read_data_mem = 0;

    
    // (Step N) Increment PC. FIXME, is it correct to always increment PC by 4?!
    //core->PC += 4;  //for sequential

    //for conditional instructions
    //make a mux with selector to choose between conditional address and PC+4
    //if selector s = 1, PC = jump address
    //if s = 0, PC = PC + 4

    read_data_mem |= core->data_mem[ALU_result + 7];

    read_data_mem = read_data_mem << 8 | core->data_mem[ALU_result + 6];
    read_data_mem = read_data_mem << 16 | core->data_mem[ALU_result + 5];
    read_data_mem = read_data_mem << 24 | core->data_mem[ALU_result + 4];
    read_data_mem = read_data_mem << 32 | core->data_mem[ALU_result + 3];
    read_data_mem = read_data_mem << 40 | core->data_mem[ALU_result + 2];
    read_data_mem = read_data_mem << 48 | core->data_mem[ALU_result + 1];
    read_data_mem = read_data_mem << 56 | core->data_mem[ALU_result + 0];
    printf("%ld\n", read_data_mem);

    if(signals.RegWrite)
    {
        core->reg_file[write_reg] = MUX(signals.MemtoReg, ALU_result, read_data_mem);
    }

    printf("ImmeGen: %ld\n", ImmeGen(input));
    printf("read_data_mem: %ld\n", read_data_mem);
    printf("Register x9: %ld\n", core->reg_file[9]);
    printf("Register x11: %ld\n", core->reg_file[11]);

    Signal shifted_signal = ShiftLeft1(ImmeGen(input));
    core->PC = Add(core->PC, MUX((zero & signals.Branch), 4, (signed int)shifted_signal));
    printf("PC: %ld\n", core->PC);



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
    if (input == 51)  //opcode
    {
        signals->ALUSrc = 0;
        signals->MemtoReg = 0;
        signals->RegWrite = 1;
        signals->MemRead = 0;
        signals->MemWrite = 0;
        signals->Branch = 0;
        signals->ALUOp = 2;
    }
    // For ld (I-type)
    if (input == 3)
    {
        signals->ALUSrc = 1;
        signals->MemtoReg = 1;
        signals->RegWrite = 1;
        signals->MemRead = 1;
        signals->MemWrite = 0;
        signals->Branch = 0;
        signals->ALUOp = 0;
    }
    // For addi and slli (I-type)
    if (input == 19)
    {
        signals->ALUSrc = 1;
        signals->MemtoReg = 1;
        signals->RegWrite = 1;
        signals->MemRead = 1;
        signals->MemWrite = 0;
        signals->Branch = 0;
        signals->ALUOp = 0;
    }
    // For sd (S-type)
    if (input == 35)
    {
        signals->ALUSrc = 1;
        signals->MemtoReg = 0; //not applicable to sd
        signals->RegWrite = 0;
        signals->MemRead = 0;
        signals->MemWrite = 1;
        signals->Branch = 0;
        signals->ALUOp = 0;
    }
    // For beq (SB-type)
    if (input == 99)
    {
        signals->ALUSrc = 0;
        signals->MemtoReg = 0; //not applicable to beq
        signals->RegWrite = 0;
        signals->MemRead = 0;
        signals->MemWrite = 0;
        signals->Branch = 1;
        signals->ALUOp = 1;
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

    //note to self: funct fields only used when ALUOp bits equal 10 (2).

    // For sub (still under r-type)
    if (ALUOp == 2 && Funct7 == 32 && Funct3 == 0)
    {
        return 6;
    }
    // For and (still under r-type)
    if (ALUOp == 2 && Funct7 == 0 && Funct3 == 7)
    {
        return 0;
    }
    // For or (still under r-type)
    if (ALUOp == 2 && Funct7 == 0 && Funct3 == 6)
    {
        return 1;
    }

    // For ld (I-type)
    if (ALUOp == 0)
    {
        return 2;
    }
    // For sd (S-type)
    if (ALUOp == 0)
    {
        return 2;
    }
    // For beq (SB-type)
    if (ALUOp == 1)
    {
        return 6;
    }
    // For slli
    if (ALUOp == 0 && Funct7 == 0 && Funct3 == 1)
    {
        return 3;
    }
}

// FIXME (3). Imme. Generator
Signal ImmeGen(Signal input)
{
    signed int immediate = 0;

    //ld
    if (input == 3)
    {
        //immediate = 000000000000;
        immediate = 0;
    }
    //addi
    if (input == 19)
    {
        //immediate = 000000000001;
        immediate = 1;
    }
    //slli
    if (input == 14)
    {
        //immediate = 000000000011;
        immediate = 3;
    }
    //bne
    if (input == 99)
    {
        //immediate = 111111111110;
        immediate = -4;
    }

    return immediate;

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
    // For and
    if (ALU_ctrl_signal == 0)
    {
        *ALU_result = (input_0 & input_1);
        if (*ALU_result == 0) { *zero = 1; } else { *zero = 0; }
    }
    // For or
    if (ALU_ctrl_signal == 1)
    {
        *ALU_result = (input_0 | input_1);
        if (*ALU_result == 0) { *zero = 1; } else { *zero = 0; }
    }
    // For subtraction
    if (ALU_ctrl_signal == 6)
    {
        *ALU_result = (input_0 - input_1);
        if (*ALU_result != 0) { *zero = 1; } else { *zero = 0; }
    }
    // For shift left
    if (ALU_ctrl_signal == 3)
    {
        *ALU_result = (input_0 << input_1);
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
