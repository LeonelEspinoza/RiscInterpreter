#include <stdio.h>
#include <stdint.h>

#define LUI     0b0110111
#define AUIPC   0b0010111 
#define JAL     0b1101111
#define JALR    0b1100111

#define BRANCH  0b1100011
#define FUNC3_BEQ   0b000
#define FUNC3_BNE   0b001
#define FUNC3_BLT   0b100
#define FUNC3_BGE   0b101
#define FUNC3_BLTU  0b110
#define FUNC3_BGEU  0b111

#define LOAD    0b0000011
#define FUNC3_LB    0b000
#define FUNC3_LH    0b001
#define FUNC3_LW    0b010
#define FUNC3_LBU   0b100
#define FUNC3_LHU   0b101

#define STORE   0b0100011
#define FUNC3_SB    0b000
#define FUNC3_SH    0b001
#define FUNC3_SW    0b010

#define OP_IMM  0b0010011
#define FUNC3_ADDI  0b000
#define FUNC3_SLTI  0b010
#define FUNC3_SLTIU 0b011
#define FUNC3_XORI  0b100
#define FUNC3_ORI   0b110
#define FUNC3_ANDI  0b111
#define FUNC3_SLLI  0b001
#define FUNC3_SRxI  0b101

#define OP      0b0110011
#define FUNC3_ADD_SUB   0b000
#define FUNC3_SLL       0b001
#define FUNC3_SLT       0b010
#define FUNC3_SLTU      0b011
#define FUNC3_XOR       0b100
#define FUNC3_SRx       0b101
#define FUNC3_OR        0b110
#define FUNC3_AND       0b111

#define FENCE   0b0001111
#define SYSTEM  0b1110011


void main(){
    // 0 in binary 32 bits
    //0b00000000000000000000000000000000
    uint32_t inst = 0b1011;
    uint8_t opcode = inst && 0b01111111;
    uint8_t rd = inst && (0b11111 << 6);
    uint8_t func3 = inst && (0b111 << 11);
    uint8_t rs1 = inst && (0b11111 << 14);
    uint8_t rs2 = inst && (0b11111 << 19);
    
    switch (opcode) {
        case LUI:
            //Load Upper Immediate
            //imm , rd , opcode
            int32_t imm = inst && (0b11111111111111111111 << 11);
            
            break;

        case AUIPC:
            //Add Upper Immediate to PC
            //imm , rd , opcode
            int32_t imm = inst && (0b11111111111111111111 << 11);
            
            break;

        case JAL:
            //Jump And Link
            //imm[20|10:1|11|19:12] , rd , opcode
            break;

        case JALR:
            //Jump And Link Register
            //imm rs1 func3 rd opcode
            int16_t imm = inst && (0b111111111111 << 19);
            break;

        case BRANCH:
            //Branches
            //imm[12|10:5] , rs2 , rs1 , func3 , imm[4:1|11] , opcode
            switch (func3) {
                case FUNC3_BEQ:
                    //Branch EQual
                    break;
                case FUNC3_BNE:
                    //Branch Not Equal
                    break;
                case FUNC3_BLT:
                    //Branch Lower Than
                    break;
                case FUNC3_BGE:  
                    //Branch Greater Than
                    break;
                case FUNC3_BLTU:  
                    //Branch Lower Than Upper
                    break;
                case FUNC3_BGEU:  
                    //Branch greater Than Upper
                    break;
            }
            break;

        case LOAD:
            //Load
            //imm[11:0] , rs1 , func3 , rd , opcode
            int16_t imm = inst && (0b111111111111 << 19);
            switch (func3) {
                case FUNC3_LB:
                    //Load Byte
                    break;
                case FUNC3_LH:
                    //Load Halfword
                    break;
                case FUNC3_LW:
                    //Load Word
                    break;
                case FUNC3_LBU:
                    //Load Bute Upper
                    break;
                case FUNC3_LHU:
                    //Load Halfword Upper
                    break;
            }
            break;

        case STORE:
            //Store
            //imm[11:5] , rs2 , rs1 , func3 , imm[4:0] , opcode
            switch (func3) {
                case FUNC3_SB:
                    //Load Byte
                    break;
                case FUNC3_SH:
                    //Load Halfword
                    break;
                case FUNC3_SW:
                    //Load Word
                    break;
            }
            break;

        case OP_IMM:
            //OPeration with Immediate
            //imm[11:0] , rs1 , func3 , rd , opcode
            switch (func3) {
                case FUNC3_ADDI:
                    //ADD Immediate
                    break;

                case FUNC3_SLTI:
                    //
                    break;

                case FUNC3_SLTIU:
                    //
                    break;

                case FUNC3_XORI:
                    //eXclusive OR Immediate
                    break;

                case FUNC3_ORI :
                    //OR Immediate
                    break;

                case FUNC3_ANDI:
                    //AND Immediate
                    break;
                
                case FUNC3_SLLI:
                    //
                    int8_t shamt = rs2;
                    break;

                case FUNC3_SRxI:
                    int8_t shamt = rs2;
                    imm = imm >> 5;
                    if (imm != 0b0100000){
                        //SRLI
                    } else {
                        //SRAI
                    }
                    break;

            }
            break;

        case OP:
            //Operations
            //const , rs2 , rs1 , func3 , rd , opcode
            int8_t k = inst >> 25;
            switch(func3) {
                case FUNC3_ADD_SUB:
                    //
                    if(k != 0b0100000){
                        //ADD
                    }else{
                        //SUB
                    }
                    break;
                case FUNC3_SLL:
                    //
                    break;
                case FUNC3_SLT:
                    //
                    break;
                case FUNC3_SLTU:
                    //
                    break;
                case FUNC3_XOR:
                    //
                    break;
                case FUNC3_SRx:
                    //
                    if(k != 0b0100000){
                        //SRL
                    }else{
                        //SRA
                    }
                    break;
                case FUNC3_OR:
                    //
                    break;
                case FUNC3_AND:
                    //
                    break;

            }
            break;
        
        case FENCE:
            break;
        
        case SYSTEM:
            if(inst != (0b1 << 19)){
                //ECALL
            }else{
                //EBREAK
            }
            break;
    }
}