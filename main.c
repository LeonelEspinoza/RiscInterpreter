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

//bit masks
#define MASK_31     0b10000000000000000000000000000000
#define MASK_31_25  0b11111110000000000000000000000000
#define MASK_30_25  0b01111110000000000000000000000000
#define MASK_30_21  0b01111111111000000000000000000000
#define MASK_20     0b00000000000100000000000000000000
#define MASK_19_12  0b00000000000011111111000000000000
#define MASK_11_08  0b00000000000000000000011110000000
#define MASK_11_07  0b00000000000000000000011111000000
#define MASK_07     0b00000000000000000000000001000000


#define MEM_SIZE 1000000


void main(){
    // 0 in binary 32 bits
    //0b00000000000000000000000000000000
    uint32_t pc = 0;
    
    /*
    x[0] -> Zero
    x[1] -> ra; return addres
    x[2] -> sp; stack pointer
    x[3] -> gp; global (var) pointer
    x[4] -> tp; thread (local var) pointer
    x[5:7], x[28:31] -> [t0:t6]
    x[8:9], x[18:27] -> [s0:s11]
    x[10:17], x[10,17] -> [a0:a7]
    */
    int32_t x[32];
    x[0]=0;

    
    int32_t M[MEM_SIZE];

    uint32_t inst = 0b1011;

    uint8_t opcode = inst       & 0b1111111;
    uint8_t rd =    (inst>>7)   & 0b11111;
    uint8_t func3 = (inst>>12)  & 0b111;
    uint8_t rs1 =   (inst>>15)  & 0b11111;
    uint8_t rs2 =   (inst>>20)  & 0b11111;

    int32_t imm;
    uint32_t uimm;

    switch (opcode) {
        case LUI:
            //Load Upper Immediate
            //imm[31:12] , rd , opcode
            imm = (inst & 0b11111111111111111111000000000000);
            x[rd] = imm;
            break;

        case AUIPC:
            //Add Upper Immediate to PC
            //imm[31:12] , rd , opcode
            imm = (inst & 0b11111111111111111111000000000000);
            x[rd] = pc + imm;
            break;

        case JAL:
            //Jump And Link
            //offset[20|10:01|11|19:12] , rd , opcode

            //inst [31|30:21|20|19:12] 
            //imm_ [20|10:01|11|19:12]
            imm = (inst & MASK_19_12) | ((inst & MASK_20) >> 9) | ((inst & MASK_30_21) >> 20) | ((inst & MASK_31) >> 11);

            //sign extend
            if(imm & 0b100000000000000000000){
                imm = imm | 0b11111111111100000000000000000000; 
            }

            x[rd] = pc+4;
            pc += imm;
            break;

        case JALR:
            //Jump And Link Register
            //imm rs1 func3 rd opcode
            imm = (inst>>20) & 0b111111111111;
            if(imm & 0b100000000000){
                imm = imm | 0b11111111111111111111100000000000;
            }
            uint32_t t = pc + 4;
            pc = (x[rs1] + imm) & (~1);
            x[rd] = t;
            break;

        case BRANCH:
            //Branches
            //imm[12|10:05] , rs2 , rs1 , func3 , imm[04:01|11] , opcode
            
            //inst [31|30:25|11:08|07]
            //imm [12|10:05|04:01|11]
            imm = ((inst & MASK_31) >> 19) | ((inst & MASK_30_25) >> 20) | ((inst & MASK_11_08) >> 7) | ((inst & MASK_07) << 4);

            //sign extender
            if(imm & 0b1000000000000){
                imm = imm | 0b11111111111111111111000000000000;
            }

            switch (func3) {
                case FUNC3_BEQ:
                    //Branch EQual
                    if(x[rs1] == x[rs2]) {
                        pc += imm;
                    }
                    break;
                case FUNC3_BNE:
                    //Branch Not Equal
                    if(x[rs1] != x[rs2]) {
                        pc += imm;
                    }
                    break;
                case FUNC3_BLT:
                    //Branch Lower Than
                    if(x[rs1] < x[rs2]) {
                        pc += imm;
                    }
                    break;
                case FUNC3_BGE:  
                    //Branch Greater or Equal
                    if(x[rs1] >= x[rs2]) {
                        pc += imm;
                    }
                    break;
                case FUNC3_BLTU:  
                    //Branch Lower Than Unsigned
                    if((uint32_t) x[rs1] < (uint32_t) x[rs2]) {
                        pc += imm;
                    }
                    break;
                case FUNC3_BGEU:  
                    //Branch greater Than Unsigned
                    if((uint32_t) x[rs1] >= (uint32_t) x[rs2]) {
                        pc += imm;
                    }
                    break;
            }
            break;

        case LOAD:
            //Load
            //imm[11:0] , rs1 , func3 , rd , opcode
            imm = (inst>>20) & 0b111111111111;

            //sign extender
            if(imm & 0b100000000000){
                imm = imm | 0b11111111111111111111100000000000;
            }
            
            int32_t mem;

            switch (func3) {
                case FUNC3_LB:
                    //Load Byte
                    mem = M[x[rs1] + imm] & 0b11111111;
                    if(mem & 0b10000000){
                        mem = mem | 0b11111111111111111111111110000000;
                    }
                    x[rd] = mem;
                    break;
                case FUNC3_LH:
                    //Load Halfword
                    mem = M[x[rs1] + imm] & 0b1111111111111111;
                    if(mem & 0b1000000000000000){
                        mem = mem | 0b11111111111111111000000000000000;
                    }
                    x[rd] = mem;
                    break;
                case FUNC3_LW:
                    //Load Word
                    x[rd] = M[x[rs1] + imm];
                    break;
                case FUNC3_LBU:
                    //Load Byte Unsigned
                    x[rd] = M[x[rs1] + imm] & 0b11111111;
                    break;
                case FUNC3_LHU:
                    //Load Halfword Unsigned
                    x[rd] = M[x[rs1] + imm] & 0b1111111111111111;
                    break;
            }
            break;

        case STORE:
            //Store
            //imm[11:5] , rs2 , rs1 , func3 , imm[4:0] , opcode
            
            //inst_ [31:25|11:07]
            //imm [11:05|04:00]
            imm = ((inst & MASK_31_25) >> 20) | ((inst & MASK_11_07) >> 7);
            
            //sign extender
            if(imm & 0b100000000000){
                imm = imm | 0b11111111111111111111100000000000;
            }

            switch (func3) {
                case FUNC3_SB:
                    //Load Byte
                    M[x[rs1] + imm] = x[rs2] & 0b11111111;
                    break;
                case FUNC3_SH:
                    //Load Halfword
                    M[x[rs1] + imm] = x[rs2] & 0b111111111111111;
                    break;
                case FUNC3_SW:
                    //Load Word
                    M[x[rs1] + imm] = x[rs2];
                    break;
            }
            break;

        case OP_IMM:
            //OPeration with Immediate
            //imm[11:0] , rs1 , func3 , rd , opcode
            uimm = inst >> 20;
            imm = uimm;
            
            //sign extender
            if(imm & 0b100000000000){
                imm = imm | 0b11111111111111111111100000000000;
            }

            uint8_t shamt = rs2;
            
            switch (func3) {
                case FUNC3_ADDI:
                    //ADD Immediate
                    x[rd] = x[rs1] + imm;
                    break;

                case FUNC3_SLTI:
                    //Set Less Than Immediate
                    x[rd] = x[rs1] < imm ? 0b1 : 0b0;
                    break;

                case FUNC3_SLTIU:
                    //Set Lower Than Immediate Unsigned
                    x[rd] = (uint32_t) x[rs1] < uimm ? 0b1 : 0b0;
                    break;

                case FUNC3_XORI:
                    //eXclusive OR Immediate
                    x[rd] = x[rs1] ^ imm;
                    break;

                case FUNC3_ORI :
                    //OR Immediate
                    x[rd] = x[rs1] | imm;
                    break;

                case FUNC3_ANDI:
                    //AND Immediate
                    x[rd] = x[rs1] & imm;
                    break;
                
                case FUNC3_SLLI:
                    //Shift Logical Left Immediate
                    x[rd] = x[rs1] << shamt;
                    break;

                case FUNC3_SRxI:
                    
                    if (imm & 0b010000000000){
                        //Shift Right Arithmetic Immediate
                        x[rd] = x[rs1] >> shamt;
                    } else {
                        //Shift Right Logical Immediate
                        x[rd] = (uint32_t) x[rs1] >> shamt;
                    }
                    break;
            }
            break;

        case OP:
            //Operations
            //const , rs2 , rs1 , func3 , rd , opcode
            int8_t k = inst >> 30;
            switch(func3) {
                case FUNC3_ADD_SUB:
                    if(k){
                        //SUB
                        x[rd] = x[rs1] - x[rs2];
                    }else{
                        //ADD
                        x[rd] = x[rs1] + x[rs2]; 
                    }
                    break;
                case FUNC3_SLL:
                    //Shift Logical Left
                    x[rd] = x[rs1] << x[rs2];
                    break;
                case FUNC3_SLT:
                    //Set Lower Than
                    x[rd] = x[rs1] < x[rs2]? 0b1 : 0b0;
                    break;
                case FUNC3_SLTU:
                    //Set Lower Than Unsigned
                    x[rd] = (uint32_t) x[rs1] < (uint32_t) x[rs2];
                    break;
                case FUNC3_XOR:
                    //eXlusive OR
                    x[rd] = x[rs1] ^ x[rs2];
                    break;
                case FUNC3_SRx:
                    //Shift Right
                    if(k){
                        //Shift Right Logical
                        x[rd] = (uint32_t) x[rs1] >> x[rs2];
                    }else{
                        //Shift Right Arithmetic
                        x[rd] = x[rs1] >> x[rs2];
                    }
                    break;
                case FUNC3_OR:
                    //OR
                    x[rd] = x[rs1] | x[rs2];
                    break;
                case FUNC3_AND:
                    //AND
                    x[rd] = x[rs1] & x[rs2];
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