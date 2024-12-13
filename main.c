#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define LUI     0b0110111 //0x37
#define AUIPC   0b0010111 //0x17
#define JAL     0b1101111 //0x6f
#define JALR    0b1100111 //0x67

#define BRANCH  0b1100011 //0x63
#define FUNC3_BEQ   0b000   //0x
#define FUNC3_BNE   0b001   //0x
#define FUNC3_BLT   0b100   //0x
#define FUNC3_BGE   0b101   //0x
#define FUNC3_BLTU  0b110   //0x
#define FUNC3_BGEU  0b111   //0x

#define LOAD    0b0000011 //0x3
#define FUNC3_LB    0b000   //0x0
#define FUNC3_LH    0b001   //0x1
#define FUNC3_LW    0b010   //0x2
#define FUNC3_LBU   0b100   //0x4
#define FUNC3_LHU   0b101   //0x5

#define STORE   0b0100011 //0x23
#define FUNC3_SB    0b000   //0x0
#define FUNC3_SH    0b001   //0x1
#define FUNC3_SW    0b010   //0x2

#define OP_IMM  0b0010011 //0x13
#define FUNC3_ADDI  0b000   //0x0
#define FUNC3_SLTI  0b010   //0x2
#define FUNC3_SLTIU 0b011   //0x3
#define FUNC3_XORI  0b100   //0x4
#define FUNC3_ORI   0b110   //0x6
#define FUNC3_ANDI  0b111   //0x7
#define FUNC3_SLLI  0b001   //0x1
#define FUNC3_SRxI  0b101   //0x5

#define OP      0b0110011 //0x33
#define FUNC3_ADD_SUB   0b000   //0x0
#define FUNC3_SLL       0b001   //0x1
#define FUNC3_SLT       0b010   //0x2
#define FUNC3_SLTU      0b011   //0x3
#define FUNC3_XOR       0b100   //0x4
#define FUNC3_SRx       0b101   //0x5
#define FUNC3_OR        0b110   //0x6
#define FUNC3_AND       0b111   //0x7

#define FUNC3_MUL       0b000   //0x0
#define FUNC3_MULH      0b001   //0x1
#define FUNC3_MULHSU    0b010   //0x2
#define FUNC3_MULHU     0b011   //0x3
#define FUNC3_DIV       0b100   //0x4
#define FUNC3_DIVU      0b101   //0x5
#define FUNC3_REM       0b110   //0x6
#define FUNC3_REMU      0b111   //0x7

#define FENCE   0b0001111 //0xf
#define SYSTEM  0b1110011 //0x73

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


#define MEM_SIZE 100

/*
Check if file_path input has file type '.ram'
@return 1 if file type not '.ram', else 0  
*/
int check_filetype(char* file_path){
    char* filetype = ".ram";
    int n1 = strlen(filetype), n2 = strlen(file_path);
    if (n1 > n2)
        return 1;
    for (int i = 0; i < n1; i++)
        if (filetype[n1 - i - 1] != file_path[n2 - i - 1])
            return 1;
    return 0;
}

/*
Count quantity of lines in file on file path
@return int count
*/
int line_counter(char* file_path){
    FILE* f_ptr;
    f_ptr = fopen(file_path,"r");
    if (f_ptr == NULL){
        printf("<line_counter> Error opening file\n");
        perror("Error opening file");
        return -1;
    }

    int count = 1;
    char c;
    while(1){
        char c = fgetc(f_ptr);
        if(feof(f_ptr)){
            break;
        }
        if (c == '\n'){
            count ++;
        }
    }
    
    fclose(f_ptr);
    return count;
}

int main(int argc, char *argv[]){
    printf("Begin\n");

    //Check if arg count is 2
    if(argc!=2){
        printf("Missing .ram file path. ");
        printf("Use of Interpreter: interpreter.exe <path/file.ram>\n");
        return 1;
    }

    //get file path
    char* file_path = argv[1];
    printf("file_path: %s\n",file_path);

    //check file type '*.ram'
    if(check_filetype(file_path)){
        printf("File type not expected.");
        printf("Expected: '<file>.ram'");
        return 1;
    }    
    
    //get line count in file
    int n_lines = line_counter(file_path);
    printf("n_lines: %d\n",n_lines);
    if(n_lines<=0){
        printf("Error counting lines\n");
        return 1;
    }

    uint32_t* program_lines = (uint32_t*) malloc(n_lines*(sizeof(uint32_t)));

    //open file in reading mode
    FILE* f_ptr;
    f_ptr = fopen(file_path,"r");
    if (f_ptr == NULL){
        printf("Error opening file\n");
        perror("Error opening file");
        return 1;
    }

    //get program lines from str to uint32_t
    char line[10];
    char* endptr;
    for(int i=0; i<n_lines; i++){
        fgets(line, 10, f_ptr);
        uint32_t hex_int = strtoul(line, &endptr, 16);
        if(endptr == line){
            printf("Error while converting str hex to long int.\n");
        } else if (*endptr != '\n' && *endptr != '\0'){
            printf("Invalid character: %c\n",*endptr);
            //return 1;
        }
        program_lines[i] = hex_int;
        //printf("program_lines[%d]: %x\n",i, program_lines[i]);
    }

    if(ferror(f_ptr)){
        perror("Error occurred while reading file\n");
    }
    fclose(f_ptr);

    /*
    Registers availables
    ---
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
    for(int i=0;i<32;i++){
        x[i]=0;
    }

    //Pseudo memory
    int32_t M[MEM_SIZE] = {0};
    
    // 0 in binary 32 bits
    //0b00000000000000000000000000000000
    uint32_t pc = 8;

    while(1){
        if(pc>=n_lines*4){
            break;
        }
        printf("Program counter: %d\n",pc);

        //isntruction to interpret
        uint32_t inst = program_lines[pc/4];
        printf("inst: 0x%x\n", inst);

        //operation code in instruction
        uint8_t opcode = inst       & 0b1111111;
        printf("opcode: 0x%x\n", opcode);

        //register destiny in instruction
        uint8_t rd =    (inst>>7)   & 0b11111;
        printf("rd: %d\n", rd);
        
        //func3 in instruction
        uint8_t func3 = (inst>>12)  & 0b111;
        printf("func3: 0x%x\n", func3);

        //register source 1 in instruction
        uint8_t rs1 =   (inst>>15)  & 0b11111;
        printf("rs1: %d\n", rs1);
        
        //register source 2 in instruction
        uint8_t rs2 =   (inst>>20)  & 0b11111;
        printf("rs2: %d\n", rs2);

        //auxiliar immediate
        int32_t imm;

        //auxiliar unsigned immediate
        uint32_t uimm;

        switch (opcode) {

            //Load Upper Immediate
            case LUI:
                //imm[31:12] , rd , opcode
                imm = (inst & 0b11111111111111111111000000000000);
                x[rd] = imm;
                pc += 4;
                break;

            //Add Upper Immediate to PC
            case AUIPC:
                //imm[31:12] , rd , opcode
                imm = (inst & 0b11111111111111111111000000000000);
                x[rd] = pc + imm;
                pc += 4;
                break;

            //Jump And Link
            case JAL:
                //offset[20|10:01|11|19:12] , rd , opcode

                //inst [31|30:21|20|19:12] 
                //imm_ [20|10:01|11|19:12]
                /*
                20
                0  0000000010 0 00000000
                offset 000000000000000000100
                rd 01010 
                opcode 1101111
                */
                imm = (inst & MASK_19_12) | ((inst & MASK_20) >> 9) | ((inst & MASK_30_21) >> 20) | ((inst & MASK_31) >> 11);

                //sign extend
                if(imm & 0b100000000000000000000){
                    imm = imm | 0b11111111111100000000000000000000; 
                }

                x[rd] = pc+4;
                pc += imm;
                break;

            //Jump And Link Register
            case JALR:
                //imm rs1 func3 rd opcode
                imm = (inst>>20) & 0b111111111111;

                //sign extender
                if(imm & 0b100000000000){
                    imm = imm | 0b11111111111111111111100000000000;
                }
                
                uint32_t t = pc + 4;
                pc = (x[rs1] + imm) & (~1);
                x[rd] = t;
                break;

            //Branches
            case BRANCH:
                //imm[12|10:05] , rs2 , rs1 , func3 , imm[04:01|11] , opcode
                
                //inst [31|30:25|11:08|07]
                //imm [12|10:05|04:01|11]
                imm = ((inst & MASK_31) >> 19) | ((inst & MASK_30_25) >> 20) | ((inst & MASK_11_08) >> 7) | ((inst & MASK_07) << 4);

                //sign extender
                if(imm & 0b1000000000000){
                    imm = imm | 0b11111111111111111111000000000000;
                }

                switch (func3) {

                    //Branch EQual
                    case FUNC3_BEQ:
                        if(x[rs1] == x[rs2]) {
                            pc += imm;
                        }
                        break;

                    //Branch Not Equal
                    case FUNC3_BNE:
                        if(x[rs1] != x[rs2]) {
                            pc += imm;
                        }
                        break;

                    //Branch Lower Than
                    case FUNC3_BLT:
                        if(x[rs1] < x[rs2]) {
                            pc += imm;
                        }
                        break;

                    //Branch Greater or Equal
                    case FUNC3_BGE:  
                        if(x[rs1] >= x[rs2]) {
                            pc += imm;
                        }
                        break;

                    //Branch Lower Than Unsigned
                    case FUNC3_BLTU:  
                        if((uint32_t) x[rs1] < (uint32_t) x[rs2]) {
                            pc += imm;
                        }
                        break;

                    //Branch greater Than Unsigned
                    case FUNC3_BGEU:  
                        if((uint32_t) x[rs1] >= (uint32_t) x[rs2]) {
                            pc += imm;
                        }
                        break;

                    default:
                        printf("Branch error func3\n");
                        printf("Instruction: %x\n",inst);
                        printf("operation code: %x\n",opcode);
                        printf("Func_3: %x\n", func3);
                        return 1;
                }
                break;

            //Load
            case LOAD:
                //imm[11:0] , rs1 , func3 , rd , opcode
                imm = (inst>>20) & 0b111111111111;

                //sign extender
                if(imm & 0b100000000000){
                    imm = imm | 0b11111111111111111111100000000000;
                }
                
                int32_t mem;

                switch (func3) {
                    //Load Byte
                    case FUNC3_LB:
                        mem = M[x[rs1] + imm] & 0b11111111;
                        if(mem & 0b10000000){
                            mem = mem | 0b11111111111111111111111110000000;
                        }
                        x[rd] = mem;
                        pc += 4;
                        break;

                    //Load Halfword
                    case FUNC3_LH:
                        mem = M[x[rs1] + imm] & 0b1111111111111111;
                        if(mem & 0b1000000000000000){
                            mem = mem | 0b11111111111111111000000000000000;
                        }
                        x[rd] = mem;
                        pc += 4;
                        break;

                    //Load Word
                    case FUNC3_LW:
                        x[rd] = M[x[rs1] + imm];
                        pc += 4;
                        break;

                    //Load Byte Unsigned
                    case FUNC3_LBU:
                        x[rd] = M[x[rs1] + imm] & 0b11111111;
                        pc += 4;
                        break;

                    //Load Halfword Unsigned
                    case FUNC3_LHU:
                        x[rd] = M[x[rs1] + imm] & 0b1111111111111111;
                        pc += 4;
                        break;
                        
                    default:
                        printf("Load error func3\n");
                        printf("Instruction: %x\n",inst);
                        printf("operation code: %x\n",opcode);
                        printf("Func_3: %x\n", func3);
                        return 1;
                }
                break;

            //Store
            case STORE:
                //imm[11:5] , rs2 , rs1 , func3 , imm[4:0] , opcode
                
                //inst_ [31:25|11:07]
                //imm [11:05|04:00]
                imm = ((inst & MASK_31_25) >> 20) | ((inst & MASK_11_07) >> 7);
                
                //sign extender
                if(imm & 0b100000000000){
                    imm = imm | 0b11111111111111111111100000000000;
                }

                switch (func3) {
                    //Store Byte
                    case FUNC3_SB:
                        M[x[rs1] + imm] = x[rs2] & 0b11111111;
                        pc += 4;
                        break;

                    //Store Halfword
                    case FUNC3_SH:
                        M[x[rs1] + imm] = x[rs2] & 0b111111111111111;
                        pc += 4;
                        break;

                    //Store Word
                    case FUNC3_SW:
                        M[x[rs1] + imm] = x[rs2];
                        pc += 4;
                        break;

                    default:
                        printf("Store error func3\n");
                        printf("Instruction: %x\n",inst);
                        printf("operation code: %x\n",opcode);
                        printf("Func_3: %x\n", func3);
                        return 1;
                }
                break;

            //OPeration with Immediate
            case OP_IMM:
                //imm[11:0] , rs1 , func3 , rd , opcode
                uimm = inst >> 20;
                imm = uimm;
                
                //sign extender
                if(imm & 0b100000000000){
                    imm = imm | 0b11111111111111111111100000000000;
                }

                uint8_t shamt = rs2;
                
                switch (func3) {

                    //ADD Immediate
                    case FUNC3_ADDI:
                        x[rd] = x[rs1] + imm;
                        pc += 4;
                        break;

                    //Set Less Than Immediate
                    case FUNC3_SLTI:
                        x[rd] = x[rs1] < imm ? 0b1 : 0b0;
                        pc += 4;
                        break;

                    //Set Lower Than Immediate Unsigned
                    case FUNC3_SLTIU:
                        x[rd] = (uint32_t) x[rs1] < uimm ? 0b1 : 0b0;
                        pc += 4;
                        break;

                    //eXclusive OR Immediate
                    case FUNC3_XORI:
                        x[rd] = x[rs1] ^ imm;
                        pc += 4;
                        break;

                    //OR Immediate
                    case FUNC3_ORI :
                        x[rd] = x[rs1] | imm;
                        pc += 4;
                        break;

                    //AND Immediate
                    case FUNC3_ANDI:
                        x[rd] = x[rs1] & imm;
                        pc += 4;
                        break;
                    
                    //Shift Logical Left Immediate
                    case FUNC3_SLLI:
                        x[rd] = x[rs1] << shamt;
                        pc += 4;
                        break;
                    
                    //Shift Right L/A Immediate
                    case FUNC3_SRxI:
                        
                        if (imm & 0b010000000000){
                            //Shift Right Arithmetic Immediate
                            x[rd] = x[rs1] >> shamt;
                            pc += 4;
                        } else {
                            //Shift Right Logical Immediate
                            x[rd] = (uint32_t) x[rs1] >> shamt;
                            pc += 4;
                        }
                        break;

                    default:
                        printf("Operation Immediate error func3\n");
                        printf("Instruction: %x\n",inst);
                        printf("operation code: %x\n",opcode);
                        printf("Func_3: %x\n", func3);
                        return 1;
                }
                break;

            //Operations
            case OP:
                //const , rs2 , rs1 , func3 , rd , opcode
                int8_t sum_bool = (inst >> 30) & 0b1;
                int8_t mul_bool = (inst >> 25) & 0b1;

                if(mul_bool){
                    int64_t mul;
                    switch (func3){
                        
                        //MULtiplication 
                        case FUNC3_MUL:
                            x[rd] = x[rs1] * x[rs2];
                            pc += 4;
                            break;
                        
                        //MULtiplication Half
                        case FUNC3_MULH:
                            mul = (x[rs1] * x[rs2]);
                            x[rd] = mul >> 32;
                            pc += 4;
                            break;
                        
                        //MULtiplication Half Signed Unsigned
                        case FUNC3_MULHSU:
                            mul = ((int32_t) (x[rs1] * ((uint32_t) x[rs2])));
                            x[rd] = mul >> 32;
                            pc += 4;
                            break;
                        
                        //MULtiplication Half Unsigned
                        case FUNC3_MULHU:
                            uint64_t u_mul = (((uint32_t) x[rs1]) * ((uint32_t) x[rs2]));
                            x[rd] = u_mul >> 32;
                            pc += 4;
                            break;
                        
                        //DIVision
                        case FUNC3_DIV:
                            x[rd] = x[rs1] / x[rs2];
                            pc += 4;
                            break;
                        
                        //DIVision Unsigned
                        case FUNC3_DIVU:
                            x[rd] = ((uint32_t) x[rs1]) / ((uint32_t) x[rs2]);
                            pc += 4;
                            break;
                        
                        
                        //REMainder
                        case FUNC3_REM:
                            x[rd] = x[rs1] % x[rs2];
                            pc += 4;
                            break;
                        
                        //REMainder Unsigned
                        case FUNC3_REMU:
                            x[rd] = ((uint32_t) x[rs1]) % ((uint32_t) x[rs2]);
                            pc += 4;
                            break;
                            
                        default:
                            printf("Operation Mul/Div error func3\n");
                            printf("Instruction: %x\n",inst);
                            printf("operation code: %x\n",opcode);
                            printf("Func_3: %x\n", func3);
                            return 1;
                    }
                    break;
                }
                switch(func3) {

                    //ADD / SUB
                    case FUNC3_ADD_SUB:
                        if(sum_bool){
                            //SUB
                            x[rd] = x[rs1] - x[rs2];
                            pc += 4;
                        }else{
                            //ADD
                            x[rd] = x[rs1] + x[rs2]; 
                            pc += 4;
                        }
                        break;

                    //Shift Logical Left
                    case FUNC3_SLL:
                        x[rd] = x[rs1] << x[rs2];
                        pc += 4;
                        break;

                    //Set Lower Than
                    case FUNC3_SLT:
                        x[rd] = x[rs1] < x[rs2]? 0b1 : 0b0;
                        pc += 4;
                        break;

                    //Set Lower Than Unsigned
                    case FUNC3_SLTU:
                        x[rd] = (uint32_t) x[rs1] < (uint32_t) x[rs2];
                        pc += 4;
                        break;

                    //eXlusive OR
                    case FUNC3_XOR:
                        x[rd] = x[rs1] ^ x[rs2];
                        pc += 4;
                        break;

                    //Shift Right L/A
                    case FUNC3_SRx:
                        if(sum_bool){
                            //Shift Right Logical
                            x[rd] = (uint32_t) x[rs1] >> x[rs2];
                        }else{
                            //Shift Right Arithmetic
                            x[rd] = x[rs1] >> x[rs2];
                        }
                        pc += 4;
                        break;

                    //OR
                    case FUNC3_OR:
                        x[rd] = x[rs1] | x[rs2];
                        pc += 4;
                        break;

                    //AND
                    case FUNC3_AND:
                        x[rd] = x[rs1] & x[rs2];
                        pc += 4;
                        break;

                    default:
                        printf("Operation error func3\n");
                        printf("Instruction: %x\n",inst);
                        printf("operation code: %x\n",opcode);
                        printf("Func_3: %x\n", func3);
                        return 1;

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
                    printf("ECALL\n");
                }else{
                    //EBREAK
                    //RiseExeption(BreakPoint)
                    printf("EBREAK\n");
                }
                break;
            default:
                printf("Error opcode\n");
                printf("Instruction: %x\n",inst);
                printf("operation code: %x\n",opcode);
                return 1;

        }

        if(pc>=n_lines*4){
            printf("Program Counter value out of range\n");
            printf("pc: %u\n",pc);
            printf("n_lines*4: %d\n",n_lines*4);
            return 1;
        }
    }

    printf("finish\n");
    return 0;
}