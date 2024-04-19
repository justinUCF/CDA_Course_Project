#include "spimcore.h"

/* ALU */
void ALU(unsigned A,unsigned B,char ALUControl,unsigned *ALUresult,char *Zero)
{
    if (ALUControl == 0)
        *ALUresult = A + B;
    else if (ALUControl == 1)
        *ALUresult = A - B;
    else if (ALUControl == 2)
        *ALUresult = (int) A < (int) B ? 1 : 0;
    else if (ALUControl == 3)
        *ALUresult = A < B ? 1 : 0;
    else if (ALUControl == 4)
        *ALUresult = A & B;
    else if (ALUControl == 5)
        *ALUresult = A | B;
    else if (ALUControl == 6)
        *ALUresult = B << 16;
    else if (ALUControl == 7)
        *ALUresult = ~A;

    *Zero = *ALUresult == 0 ? 1 : 0;
    return;
}

/* Instruction Fetch */
/* 10 Points */
int instruction_fetch(unsigned PC,unsigned *Mem,unsigned *instruction)
{
    int lowerLim = 0; // 0x0000 = 0
    int upperLim = 65535; // 0xFFFF = 65535
    if(PC < lowerLim || PC > upperLim || PC%4 != 0){
        return 1;
    }
    else{
        *instruction = Mem[PC];
        return 0;
    }
}

/* Instruction Partition */
/* 10 Points */
void instruction_partition(unsigned instruction, unsigned *op, unsigned *r1,unsigned *r2, unsigned *r3, unsigned *funct, unsigned *offset, unsigned *jsec)
{
    *op = (instruction & 0xFC000000) >> 26;
    *r1 = (instruction & 0x3E00000) >> 21;
    *r2 = (instruction & 0x1F0000) >> 16;
    *r3 = (instruction & 0xF800) >> 11;
    *funct = (instruction & 0x3F);
    *offset = (instruction & 0xFFFF);
    *jsec = (instruction & 0x1FFFFFF);

}

/* Instruction Decode */
/* 30 Points */
int instruction_decode(unsigned op,struct_controls *controls)
{
    if(op == 0){ // R type control signals
        controls->RegDst = '1';
        controls->Jump = '0';
        controls->Branch = '0';
        controls->MemRead = '0';
        controls->MemtoReg = '0';
        controls->ALUOp = '7';
        controls->MemWrite = '0';
        controls->ALUSrc = '0';
        controls->RegWrite = '1';
        return 0;
    }
    else if(op = 35){ // Lw - Load word
        controls->RegDst = '0';
        controls->Jump = '0';
        controls->Branch = '0';
        controls->MemRead = '1';
        controls->MemtoReg = '1';
        controls->ALUOp = '0';
        controls->MemWrite = '0';
        controls->ALUSrc = '1';
        controls->RegWrite = '1';
        return 0;
    }
    else if(op = 43){ // Sw - Store word
        controls->RegDst = 'DC';
        controls->Jump = '0';
        controls->Branch = '0';
        controls->MemRead = '0';
        controls->MemtoReg = 'DC';
        controls->ALUOp = '0';
        controls->MemWrite = '1';
        controls->ALUSrc = '1';
        controls->RegWrite = '0';
        return 0;
    }
    else if(op == 4){ // Beq - Branch if equal
        controls->RegDst = 'DC';
        controls->Jump = '0';
        controls->Branch = '1';
        controls->MemRead = '0';
        controls->MemtoReg = 'DC';
        controls->ALUOp = '1';
        controls->MemWrite = '0';
        controls->ALUSrc = '0';
        controls->RegWrite = '0';
        return 0;
    }
    else if(op == 10){ // Slti - Set if less than immediate
        controls->RegDst = '0';
        controls->Jump = '0';
        controls->Branch = '0';
        controls->MemRead = '0';
        controls->MemtoReg = '0';
        controls->ALUOp = '2';
        controls->MemWrite = '0';
        controls->ALUSrc = '1';
        controls->RegWrite = '1';
        return 0;
    }
    else if(op == 9){ // Sltiu - Set on less than immediate unsigned
        controls->RegDst = '0';
        controls->Jump = '0';
        controls->Branch = '0';
        controls->MemRead = '0';
        controls->MemtoReg = '0';
        controls->ALUOp = '3';
        controls->MemWrite = '0';
        controls->ALUSrc = '1';
        controls->RegWrite = '1';
        return 0;
    }
    else if(op == 8){ // Addi - Add immediate
        controls->RegDst = '0';
        controls->Jump = '0';
        controls->Branch = '0';
        controls->MemRead = '0';
        controls->MemtoReg = '0';
        controls->ALUOp = '0';
        controls->MemWrite = '0';
        controls->ALUSrc = '1';
        controls->RegWrite = '1';
        return 0;
    }
    else if(op == 67){ // Lui - Load upper immediate
        controls->RegDst = '0';
        controls->Jump = '0';
        controls->Branch = '0';
        controls->MemRead = '0';
        controls->MemtoReg = '0';
        controls->ALUOp = '6';
        controls->MemWrite = '0';
        controls->ALUSrc = '1';
        controls->RegWrite = '1';
        return 0;
    }
    else{ // Unknown Op
        return 1;
    }
}

/* Read Register */
/* 5 Points */
void read_register(unsigned r1,unsigned r2,unsigned *Reg,unsigned *data1,unsigned *data2)
{
    *data1 = Reg[r1];
    *data2 = Reg[r2];
}

/* Sign Extend */
/* 10 Points */
void sign_extend(unsigned offset,unsigned *extended_value)
{
    if(offset >> 15 == 1){
        *extended_value = (offset | 0xFFFF0000);
    }
    else if(offset >> 15 == 0){
        *extended_value = offset;
    }
    else{
        return;
    }
}

/* ALU Operations */
int ALU_operations(unsigned data1,unsigned data2,unsigned extended_value,unsigned funct,char ALUOp,char ALUSrc,unsigned *ALUresult,char *Zero)
{
	// Handle second parameter to ALU
    unsigned B = ALUSrc == 1 ? extended_value : data2;

    // Handle ALU control unit
    if (ALUOp == 7) {
        // Add
        if (funct == 0b100000)
            ALUOp = 0;
        // Subtract
        else if (funct == 0b100010)
            ALUOp = 1;
        // And
        else if (funct == 0b100100)
            ALUOp = 4;
        // Or
        else if (funct == 0b100101)
            ALUOp = 5;
        // Slt
        else if (funct == 0b101010)
            ALUOp = 2;
        // Sltu
        else if (funct == 0b101011)
            ALUOp = 3;
        // Unknown funct code
        else
            return 1;
    }

    ALU(data1, B, ALUOp, ALUresult, Zero);
    return 0;

}

/* Read / Write Memory */
/* 10 Points */
int rw_memory(unsigned ALUresult,unsigned data2,char MemWrite,char MemRead,unsigned *memdata,unsigned *Mem)
{
    if(MemRead != 0){
        if(ALUresult%4 == 0){
            *memdata = ALUresult;
        }
        else{
            return 1;
        }
    }
    if(MemWrite != 0){
        if(ALUresult%4 == 0){
            ALUresult = data2;
        }
        else{
            return 1;
        }
    }
    return 0;
}

/* Write Register */
/* 10 Points */
void write_register(unsigned r2,unsigned r3,unsigned memdata,unsigned ALUresult,char RegWrite,char RegDst,char MemtoReg,unsigned *Reg)
{
    if(RegWrite == 0){
        return;
    }
    else if(RegWrite != 0){
        if()
    }

}

/* PC Update */
/* 15 Points */
void PC_update(unsigned jsec,unsigned extended_value,char Branch,char Jump,char Zero,unsigned *PC)
{

}
