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

    uint8_t opcode = inst       & 0b1111111;
    uint8_t rd =    (inst>>7)   & 0b11111;
    uint8_t func3 = (inst>>12)  & 0b111;
    uint8_t rs1 =   (inst>>15)  & 0b11111;
    uint8_t rs2 =   (inst>>20)  & 0b11111;
    
    switch (opcode) {
        case LUI:
            //Load Upper Immediate
            //imm[31:12] , rd , opcode
            int32_t imm = (inst >> 12);
            //x[rd] = sign_extend(imm << 12);
            break;

        case AUIPC:
            //Add Upper Immediate to PC
            //imm[31:12] , rd , opcode
            int32_t imm = (inst >> 12);
            //x[rd] = pc + sign_extend(imm << 12);
            break;

        case JAL:
            //Jump And Link
            //offset[20|10:1|11|19:12] , rd , opcode
            //x[rd] = pc+4;
            //pc += sign_extend(offset)
            break;

        case JALR:
            //Jump And Link Register
            //imm rs1 func3 rd opcode
            int16_t imm = inst & (0b111111111111 << 19);
            // t = pc + 4;
            // pc = (x[rs1] + signed_extend(offset))&~1;
            // x[rd] = t;
            break;

        case BRANCH:
            //Branches
            //imm[12|10:5] , rs2 , rs1 , func3 , imm[4:1|11] , opcode
            switch (func3) {
                case FUNC3_BEQ:
                    //Branch EQual
                    /*
                    if(x[rs1] == x[rs2]) {
                        pc += sign_extend(offset)
                    }
                    */
                    break;
                case FUNC3_BNE:
                    //Branch Not Equal
                    /*
                    if(x[rs1] != x[rs2]) {
                        pc += sign_extend(offset)
                    }
                    */
                    break;
                case FUNC3_BLT:
                    //Branch Lower Than
                    /*
                    if(x[rs1] < signed x[rs2]) {
                        pc += sign_extend(offset)
                    }
                    */
                    break;
                case FUNC3_BGE:  
                    //Branch Greater or Equal
                    /*
                    if(x[rs1] >= signed x[rs2]) {
                        pc += sign_extend(offset)
                    }
                    */
                    break;
                case FUNC3_BLTU:  
                    //Branch Lower Than Unsigned
                    /*
                    if(x[rs1] < unsigned x[rs2]) {
                        pc += sign_extend(offset)
                    }
                    */
                    break;
                case FUNC3_BGEU:  
                    //Branch greater Than Unsigned
                    /*
                    if(x[rs1] >= unsigned x[rs2]) {
                        pc += sign_extend(offset)
                    }
                    */
                    break;
            }
            break;

        case LOAD:
            //Load
            //imm[11:0] , rs1 , func3 , rd , opcode
            int16_t imm = inst & (0b111111111111 << 19);
            switch (func3) {
                case FUNC3_LB:
                    //Load Byte
                    //x[rd] = sign_extend(M[x[rs1] + sign_extend(offset)][7:0])
                    break;
                case FUNC3_LH:
                    //Load Halfword
                    //x[rd] = sign_extend(M[x[rs1] + sign_extend(offset)][15:0])
                    break;
                case FUNC3_LW:
                    //Load Word
                    //x[rd] = sign_extend(M[x[rs1] + sext(offset)][31:0])
                    break;
                case FUNC3_LBU:
                    //Load Byte Unsigned
                    //x[rd] = M[x[rs1] + sext(offset)][7:0]
                    break;
                case FUNC3_LHU:
                    //Load Halfword Unsigned
                    //x[rd] = M[x[rs1] + sext(offset)][15:0]
                    break;
            }
            break;

        case STORE:
            //Store
            //imm[11:5] , rs2 , rs1 , func3 , imm[4:0] , opcode
            switch (func3) {
                case FUNC3_SB:
                    //Load Byte
                    //M[x[rs1] + sign_extend(offset)] = x[rs2][7:0]
                    break;
                case FUNC3_SH:
                    //Load Halfword
                    //M[x[rs1] + sign_extend(offset)] = x[rs2][15:0]
                    break;
                case FUNC3_SW:
                    //Load Word
                    //M[x[rs1] + sign_extend(offset)] = x[rs2][31:0]
                    break;
            }
            break;

        case OP_IMM:
            //OPeration with Immediate
            //imm[11:0] , rs1 , func3 , rd , opcode
            uint16_t imm = inst >> 20;
            switch (func3) {
                case FUNC3_ADDI:
                    //ADD Immediate
                    //x[rd] = x[rs1] + sign_extend(imm)
                    break;

                case FUNC3_SLTI:
                    //Set Less Than Immediate
                    //x[rd] = x[rs1] < s sign_extend(imm)
                    break;

                case FUNC3_SLTIU:
                    //Set Lower Than Immediate Unsigned
                    //x[rd] = x[rs1] < u sign_extend(imm)
                    break;

                case FUNC3_XORI:
                    //eXclusive OR Immediate
                    //x[rd] = x[rs1] ^ sign_extend(imm)
                    break;

                case FUNC3_ORI :
                    //OR Immediate
                    //x[rd] = x[rs1] | sign_extend(imm)
                    break;

                case FUNC3_ANDI:
                    //AND Immediate
                    //x[rd] = x[rs1] & sign_extend(imm)
                    break;
                
                case FUNC3_SLLI:
                    //Shift Logical Left Immediate
                    int8_t shamt = rs2;
                    //x[rd] = x[rs1] << shamt
                    break;

                case FUNC3_SRxI:
                    int8_t shamt = rs2;
                    imm = imm >> 5;
                    if (imm != 0b0100000){
                        //Shift Right Logical Immediate
                        //x[rd] = x[rs1] >> unsigned shamt
                    } else {
                        //Shift Right Arithmetic Immediate
                        //x[rd] = x[rs1] >> signed shamt
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
                    if(k != 0b0100000){
                        //ADD
                        //x[rd] = x[rs1] + x[rs2] 
                    }else{
                        //SUB
                        //x[rd] = x[rs1] - x[rs2]
                    }
                    break;
                case FUNC3_SLL:
                    //Shift Logical Left
                    //x[rd] = x[rs1] << x[rs2]
                    break;
                case FUNC3_SLT:
                    //Set Lower Than
                    //x[rd] = x[rs1] < signed x[rs2]
                    break;
                case FUNC3_SLTU:
                    //Set Lower Than Unsigned
                    //x[rd] = x[rs1] < unsigned x[rs2]
                    break;
                case FUNC3_XOR:
                    //eXlusive OR
                    //x[rd] = x[rs1] ^ x[rs2]
                    break;
                case FUNC3_SRx:
                    //Shift Right
                    if(k != 0b0100000){
                        //Shift Right Logical
                        //x[rd] = x[rs1] >> unsigned x[rs2]
                    }else{
                        //Shift Right Arithmetic
                        //x[rd] = x[rs1] >> signed x[rs2]
                    }
                    break;
                case FUNC3_OR:
                    //OR
                    //x[rd] = x[rs1] | x[rs2]
                    break;
                case FUNC3_AND:
                    //AND
                    //x[rd] = x[rs1] & x[rs2]
                    break;

            }
            break;
        
        case FENCE:
            int8_t fm,pred,succ;
            succ = ((inst >> 20) & 0b000000001111);
            pred = ((inst >> 24) & 0b00001111);
            fm   = ((inst >> 28) & 0b1111);
            //Fence(pred,succ)
            break;
        
        case SYSTEM:
            if(inst != (0b1 << 19)){
                //ECALL
                //RiseException(EnviromentCall)
            }else{
                //EBREAK
                //RiseExeption(BreakPoint)
            }
            break;
    }
}