#include <stdio.h>
#include <stdint.h>

#define LUI     0b0110111
#define AUIPC   0b0010111 
#define JAL     0b1101111
#define JALR    0b1100111
#define BRANCH  0b1100011
#define LOAD    0b0000011
#define STORE   0b0100011
#define OP_IMM  0b0010011
#define OP      0b0110011
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
            break;

        case AUIPC:
            break;

        case JAL:
            break;

        case JALR:
            break;

        case BRANCH:
            break;

        case LOAD:
            break;

        case STORE:
            break;

        case OP_IMM:
            break;

        case OP:
            break;
        
        case FENCE:
            break;
        
        case SYSTEM:
            break;
    }
}