#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <elf.h>
#include <libelf.h>
#include <gelf.h>
#include <fcntl.h>
#include "pag2.c"

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

int main(int argc, char *argv[]){
    
    // ------------------------------------------------
    // Initialize
    // ------------------------------------------------    
    printf("Begin Initialize\n");
    
    //Check if arg count is 2
    if(argc!=2){
        errx(EXIT_FAILURE,"Usage: %s <ELF-file-name>\n", argv[0]);
        return 1;
    }

    //get file path
    const char* file_name = argv[1];
    printf("file_name: %s\n",file_name);

    // Necessary to call other libelf library functions
    if (elf_version(EV_CURRENT) == EV_NONE){ 
        err(EXIT_FAILURE, "ELF library initialization failed: %s", elf_errmsg(-1));
    }

    // Open file with fopen read
    FILE *f_in = fopen(file_name, "r");
    if (f_in == NULL){
        errx(EXIT_FAILURE,"ERROR: fopen(%s,\"r\") failed", file_name);
    }
    
    // (int) file descriptor for elf file handler
    int fd = 0;
    // Open file with open
    if ((fd = open(file_name, O_RDONLY, 0)) < 0){
        err(EXIT_FAILURE, "ERROR: open \"%s\" failed", file_name);
    }

    // ELF file handler
    Elf *elf = NULL;
    // Convert the file descriptor to an ELF handle
    if((elf = elf_begin(fd, ELF_C_READ, NULL))==NULL){
        errx(EXIT_FAILURE, "elf_begin() failed: %s", elf_errmsg(-1));
    }

    // Check if file is ELF kind
    if (elf_kind(elf) != ELF_K_ELF){
        errx(EXIT_FAILURE, "\"%s\" is not an ELF object.", argv[1]);
    }

    // Elf header 
    GElf_Ehdr ehdr;
    // Get elf file header
    if (gelf_getehdr(elf, &ehdr) == NULL){
        errx(EXIT_FAILURE, "gelf_getehdr() failed: %s", elf_errmsg(-1));
    }

    // Memory pagination structure
    struct block_entry *block_table;

    // Initialize block table
    block_table = init_block_table();

    printf("End Initialize\n");

    // ------------------------------------------------
    // Load program headers to memory in block_table
    // ------------------------------------------------

    printf("Begin loader to memory\n");
    
    // Check program header entry size
    if(ehdr.e_phentsize != sizeof(Elf32_Phdr)){
        err(EXIT_FAILURE, "Unexpected program header size: %d", ehdr.e_phentsize);
    }

    // Total program header entries
    unsigned int phnum = ehdr.e_phnum;
    
    // Total size of all program header entries
    size_t phtotal = phnum * ehdr.e_phentsize;
    
    // Array to program headers
    GElf_Phdr *phdr_ar = malloc(phtotal);

    // Get pointers to all program headers
    unsigned int i=0;
    GElf_Phdr phdr_temp;
    for(int i=0; i<phnum; i++){
        gelf_getphdr(elf,i,&phdr_temp);
        phdr_ar[i]=phdr_temp;
        //printf("phdr_ar[%u]: %u\n", i, phdr_ar[0].p_type);
        i++;
    }

    // Get size of vaddr (max and min virtual address used)
    size_t max_vaddr = 0;
    size_t min_vaddr = SIZE_MAX;
    for(int i=0; i<phnum; i++){
        if (phdr_ar[i].p_type != PT_LOAD){
            continue;
        }
        if (phdr_ar[i].p_vaddr + phdr_ar[i].p_memsz > max_vaddr){
            max_vaddr = phdr_ar[i].p_vaddr + phdr_ar[i].p_memsz;
        }
        if (phdr_ar[i].p_vaddr < min_vaddr){
            min_vaddr = phdr_ar[i].p_vaddr;
        }
    }
    printf("min vaddr: %lu\n",min_vaddr);
    printf("max vaddr: %lu\n",max_vaddr);

    // Save contents of program headers with PT_LOAD type
    // in block_table at their respective addresses
    int items;
    void* buf;
    for (int i=0; i<phnum; i++){
        
        // Only phdr with LOAD type
        if (phdr_ar[i].p_type != PT_LOAD){
            continue;
        }
        
        // Get phdr atributes
        unsigned long vaddr = phdr_ar[i].p_vaddr;
        unsigned long memsz = phdr_ar[i].p_memsz;
        unsigned long filesz = phdr_ar[i].p_filesz;
        unsigned long offset = phdr_ar[i].p_offset;

        // buffer to read from file and then write in block table
        buf = malloc(memsz);

        // Initialize buffer with 0's
        memset(buf, 0, memsz);

        // Point cursor to offset of phdr
        fseek(f_in, offset, SEEK_SET);

        // Read filesz*1 bytes content from file to buffer
        if ((items = fread(buf, filesz, 1, f_in))<0 ){
            errx(EXIT_FAILURE, "ERROR: Failed ro read segment %d\n", i);
        }
        
        // If items read are not the expected amount
        if(items<1){
            errx(EXIT_FAILURE, "ERROR: Segment %d is incomplete\n", i);
        }

        printf("\nContents in phdr #%u:\n",i);
        for(size_t j=0;j<memsz;j++){
            printf("%02x ",((uint8_t *)buf)[j]);
        }
    
        // Write buffer with data in block_table 
        if(write_in_bulk(block_table, vaddr, buf, memsz) < memsz){
            errx(EXIT_FAILURE, "ERROR: Number of bytes to write and bytes written doesn't match.");
        }

        // buf free malloc
        free(buf);
    }
    
    // phdr_ar free malloc
    free(phdr_ar);

    printf("End loader to memory\n");

    // ------------------------------------------------
    // Interpreter
    // ------------------------------------------------
    printf("Begin interpreter\n");
    
    /*
    ### Registers availables

    x[0] -> Zero
    x[1] -> ra; return addres
    x[2] -> sp; stack pointer
    x[3] -> gp; global (var) pointer
    x[4] -> tp; thread (local var) pointer
    x[5:7], x[28:31] -> [t0:t6]
    x[8:9], x[18:27] -> [s0:s11]
    x[10:17] -> [a0:a7]
    */
    int32_t x[32];
    memset(x, 0, 32*sizeof(int32_t));

    //Pseudo memory is block table
    
    // Get _start instruction
    uint32_t _start_addr = get_start_addr();
    
    // Program Counter - Instruction Index
    uint32_t pc = _start_addr;
    
    // Instruction
    uint32_t inst;
    while(1){
        // If program counter out of limits break
        // {something is missing here}

        // Get instruction to interpret
        read_from_bulk(block_table, pc, &inst, sizeof(uint32_t));
        //printf("inst: 0x%x\n", inst);

        // Get operation code in instruction
        uint8_t opcode = inst       & 0b1111111;
        //printf("opcode: 0x%x\n", opcode);

        // Get register destination (rd) in instruction
        uint8_t rd =    (inst>>7)   & 0b11111;
        //printf("rd: %d\n", rd);
        
        // Get func3 in instruction
        uint8_t func3 = (inst>>12)  & 0b111;
        //printf("func3: 0x%x\n", func3);

        //Get register source 1 (rs1) in instruction
        uint8_t rs1 =   (inst>>15)  & 0b11111;
        //printf("rs1: %d\n", rs1);
        
        // Get register source 2 (rd2) in instruction
        uint8_t rs2 =   (inst>>20)  & 0b11111;
        //printf("rs2: %d\n", rs2);

        //auxiliar immediate
        int32_t imm;

        //auxiliar unsigned immediate
        uint32_t uimm;

        switch (opcode) {

            //Load Upper Immediate
            case LUI:
                //imm[31:12] , rd , opcode
                imm = (inst & 0b11111111111111111111000000000000);
                printf("LUI imm: %lu rd: %u\n", imm, rd);

                x[rd] = imm;
                pc += 4;
                break;

            //Add Upper Immediate to PC
            case AUIPC:
                //imm[31:12] , rd , opcode
                imm = (inst & 0b11111111111111111111000000000000);
                printf("AUIPC imm: %lu rd: %u\n", imm, rd);
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
                printf("JAL offset: %lu rd: %u\n", imm, rd);
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
                printf("JALR offset: %lu rs1: %u rd: %u\n", imm, rs1, rd);
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
                        printf("BEQ imm: %lu rs1: %u rs2: %u\n", imm, rs1, rs2);
                        if(x[rs1] == x[rs2]) {
                            pc += imm;
                        }
                        break;

                    //Branch Not Equal
                    case FUNC3_BNE:
                        printf("BNE imm: %lu rs1: %u rs2: %u\n", imm, rs1, rs2);
                        if(x[rs1] != x[rs2]) {
                            pc += imm;
                        }
                        break;

                    //Branch Lower Than
                    case FUNC3_BLT:
                        printf("BLT imm: %lu rs1: %u rs2: %u\n", imm, rs1, rs2);
                        if(x[rs1] < x[rs2]) {
                            pc += imm;
                        }
                        break;

                    //Branch Greater or Equal
                    case FUNC3_BGE:  
                        printf("BGE imm: %lu rs1: %u rs2: %u\n", imm, rs1, rs2);
                        if(x[rs1] >= x[rs2]) {
                            pc += imm;
                        }
                        break;

                    //Branch Lower Than Unsigned
                    case FUNC3_BLTU:  
                        printf("BLTU imm: %lu rs1: %u rs2: %u\n", imm, rs1, rs2);
                        if((uint32_t) x[rs1] < (uint32_t) x[rs2]) {
                            pc += imm;
                        }
                        break;

                    //Branch greater Than Unsigned
                    case FUNC3_BGEU:  
                        printf("BGEU imm: %lu rs1: %u rs2: %u\n", imm, rs1, rs2);
                        if((uint32_t) x[rs1] >= (uint32_t) x[rs2]) {
                            pc += imm;
                        }
                        break;

                    default:
                        errx(EXIT_FAILURE, "ERROR: Couldn't match branch's func3 (instruction %x; opcode %x; func3 %x).", inst, opcode, func3);
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
                        printf("LB imm: %lu rs1: %u rd: %u\n", imm, rs1, rd);
                        read_from_bulk(block_table, (x[rs1] + imm), &mem, sizeof(mem));
                        mem = mem & 0b11111111;
                        //mem = M[x[rs1] + imm] & 0b11111111;
                        if(mem & 0b10000000){
                            mem = mem | 0b11111111111111111111111110000000;
                        }
                        x[rd] = mem;
                        printf("x[rd=%u] = %d\n",rd,x[rd]);
                        pc += 4;
                        break;

                    //Load Halfword
                    case FUNC3_LH:
                        printf("LH imm: %lu rs1: %u rd: %u\n", imm, rs1, rd);
                        read_from_bulk(block_table, (x[rs1] + imm), &mem, sizeof(mem));
                        mem = mem & 0b1111111111111111;
                        //mem = M[x[rs1] + imm] & 0b1111111111111111;
                        if(mem & 0b1000000000000000){
                            mem = mem | 0b11111111111111111000000000000000;
                        }
                        x[rd] = mem;
                        printf("x[rd=%u] = %d\n",rd,x[rd]);
                        pc += 4;
                        break;

                    //Load Word
                    case FUNC3_LW:
                        printf("LW imm: %lu rs1: %u rd: %u\n", imm, rs1, rd);
                        read_from_bulk(block_table, (x[rs1] + imm), &(x[rd]), sizeof(x[rd]));
                        //x[rd] = M[x[rs1] + imm];
                        printf("x[rd=%u] = %d\n",rd,x[rd]);
                        pc += 4;
                        break;

                    //Load Byte Unsigned
                    case FUNC3_LBU:
                        printf("LBU imm: %lu rs1: %u rd: %u\n", imm, rs1, rd);
                        read_from_bulk(block_table, (x[rs1] + imm), &(x[rd]), sizeof(x[rd]));
                        x[rd] = x[rd] & 0b11111111;
                        //x[rd] = M[x[rs1] + imm] & 0b11111111;
                        printf("x[rd]: %d\n",x[rd]);
                        pc += 4;
                        break;

                    //Load Halfword Unsigned
                    case FUNC3_LHU:
                        printf("LHU imm: %lu rs1: %u rd: %u\n", imm, rs1, rd);
                        read_from_bulk(block_table, (x[rs1] + imm), &(x[rd]), sizeof(x[rd]));
                        x[rd] = x[rd] & 0b1111111111111111;
                        //x[rd] = M[x[rs1] + imm] & 0b1111111111111111;
                        printf("x[rd]: %d\n",x[rd]);
                        pc += 4;
                        break;
                        
                    default:
                        errx(EXIT_FAILURE, "ERROR: Couldn't match load's func3 (instruction %x; opcode %x; func3 %x).", inst, opcode, func3);
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
                        printf("SB imm: %lu rs1: %u rs2: %u\n", imm, rs1, rs2);
                        x[rs2] = x[rs2] & 0b11111111;
                        write_in_bulk(block_table, x[rs1] + imm, &(x[rs2]), sizeof(x[rs2]));
                        //M[x[rs1] + imm] = x[rs2] & 0b11111111;

                        printf("stored: %d\n", x[rs2]);
                        pc += 4;
                        break;

                    //Store Halfword
                    case FUNC3_SH:
                        printf("SH imm: %lu rs1: %u rs2: %u\n", imm, rs1, rs2);
                        x[rs2] = x[rs2] & 0b111111111111111;
                        write_in_bulk(block_table, (x[rs1] + imm), &(x[rs2]), sizeof(x[rs2]));
                        //M[x[rs1] + imm] = x[rs2] & 0b111111111111111;
                        printf("stored: %d\n", x[rs2]);
                        pc += 4;
                        break;

                    //Store Word
                    case FUNC3_SW:
                        printf("SW imm: %lu rs1: %u rs2: %u\n", imm, rs1, rs2);
                        write_in_bulk(block_table, (x[rs1] + imm), &(x[rs2]), sizeof(x[rs2]));
                        //M[x[rs1] + imm] = x[rs2];
                        printf("stored: %d\n", x[rs2]);
                        pc += 4;
                        break;

                    default:
                        errx(EXIT_FAILURE, "ERROR: Couldn't match store's func3 (instruction %x; opcode %x; func3 %x).", inst, opcode, func3);
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
                        printf("ADDI imm: %lu rs1: %u rd: %u\n", imm, rs1, rd);
                        x[rd] = x[rs1] + imm;
                        pc += 4;
                        break;

                    //Set Less Than Immediate
                    case FUNC3_SLTI:
                        printf("SLTI imm: %lu rs1: %u rd: %u\n", imm, rs1, rd);
                        x[rd] = x[rs1] < imm ? 0b1 : 0b0;
                        pc += 4;
                        break;

                    //Set Lower Than Immediate Unsigned
                    case FUNC3_SLTIU:
                        printf("SLTIU imm: %lu rs1: %u rd: %u\n", imm, rs1, rd);
                        x[rd] = (uint32_t) x[rs1] < uimm ? 0b1 : 0b0;
                        pc += 4;
                        break;

                    //eXclusive OR Immediate
                    case FUNC3_XORI:
                        printf("XORI imm: %lu rs1: %u rd: %u\n", imm, rs1, rd);
                        x[rd] = x[rs1] ^ imm;
                        pc += 4;
                        break;

                    //OR Immediate
                    case FUNC3_ORI :
                        printf("ORI imm: %lu rs1: %u rd: %u\n", imm, rs1, rd);
                        x[rd] = x[rs1] | imm;
                        pc += 4;
                        break;

                    //AND Immediate
                    case FUNC3_ANDI:
                        printf("ANDI imm: %lu rs1: %u rd: %u\n", imm, rs1, rd);
                        x[rd] = x[rs1] & imm;
                        pc += 4;
                        break;
                    
                    //Shift Logical Left Immediate
                    case FUNC3_SLLI:
                        printf("SLLI imm: %lu rs1: %u rd: %u\n", imm, rs1, rd);
                        x[rd] = x[rs1] << shamt;
                        pc += 4;
                        break;
                    
                    //Shift Right L/A Immediate
                    case FUNC3_SRxI:
                        
                        if (imm & 0b010000000000){
                            //Shift Right Arithmetic Immediate
                            printf("SRAI imm: %lu rs1: %u rd: %u\n", imm, rs1, rd);
                            x[rd] = x[rs1] >> shamt;
                            pc += 4;
                        } else {
                            //Shift Right Logical Immediate
                            printf("SRLI imm: %lu rs1: %u rd: %u\n", imm, rs1, rd);
                            x[rd] = (uint32_t) x[rs1] >> shamt;
                            pc += 4;
                        }
                        break;

                    default:
                        errx(EXIT_FAILURE, "ERROR: Couldn't match pperation immediate's func3 (instruction %x; opcode %x; func3 %x)", inst, opcode, func3);
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
                            printf("MUL rs1: %u rs2: %u rd: %u\n", rs1, rs2, rd);
                            x[rd] = x[rs1] * x[rs2];
                            pc += 4;
                            break;
                        
                        //MULtiplication Half
                        case FUNC3_MULH:
                            printf("MULH rs1: %u rs2: %u rd: %u\n", rs1, rs2, rd);
                            mul = (x[rs1] * x[rs2]);
                            x[rd] = mul >> 32;
                            pc += 4;
                            break;
                        
                        //MULtiplication Half Signed Unsigned
                        case FUNC3_MULHSU:
                            printf("MULHSU rs1: %u rs2: %u rd: %u\n", rs1, rs2, rd);
                            mul = ((int32_t) (x[rs1] * ((uint32_t) x[rs2])));
                            x[rd] = mul >> 32;
                            pc += 4;
                            break;
                        
                        //MULtiplication Half Unsigned
                        case FUNC3_MULHU:
                            printf("MULHU rs1: %u rs2: %u rd: %u\n", rs1, rs2, rd);
                            uint64_t u_mul = (((uint32_t) x[rs1]) * ((uint32_t) x[rs2]));
                            x[rd] = u_mul >> 32;
                            pc += 4;
                            break;
                        
                        //DIVision
                        case FUNC3_DIV:
                            printf("DIV rs1: %u rs2: %u rd: %u\n", rs1, rs2, rd);
                            x[rd] = x[rs1] / x[rs2];
                            pc += 4;
                            break;
                        
                        //DIVision Unsigned
                        case FUNC3_DIVU:
                            printf("DIVU rs1: %u rs2: %u rd: %u\n", rs1, rs2, rd);
                            x[rd] = ((uint32_t) x[rs1]) / ((uint32_t) x[rs2]);
                            pc += 4;
                            break;
                        
                        
                        //REMainder
                        case FUNC3_REM:
                            printf("REM rs1: %u rs2: %u rd: %u\n", rs1, rs2, rd);
                            x[rd] = x[rs1] % x[rs2];
                            pc += 4;
                            break;
                        
                        //REMainder Unsigned
                        case FUNC3_REMU:
                            printf("REMU rs1: %u rs2: %u rd: %u\n", rs1, rs2, rd);
                            x[rd] = ((uint32_t) x[rs1]) % ((uint32_t) x[rs2]);
                            pc += 4;
                            break;
                            
                        default:
                            errx(EXIT_FAILURE, "ERROR: Could not match Mul/Div func3 (instruction %x; opcode %x; func3 %x).", inst, opcode, func3);
                            return 1;
                    }
                    break;
                }
                switch(func3) {

                    //ADD / SUB
                    case FUNC3_ADD_SUB:
                        if(sum_bool){
                            //SUB
                            printf("SUB rs1: %u rs2: %u rd: %u\n", rs1, rs2, rd);
                            x[rd] = x[rs1] - x[rs2];
                            pc += 4;
                        }else{
                            //ADD
                            printf("ADD rs1: %u rs2: %u rd: %u\n", rs1, rs2, rd);
                            x[rd] = x[rs1] + x[rs2]; 
                            pc += 4;
                        }
                        break;

                    //Shift Logical Left
                    case FUNC3_SLL:
                        printf("SLL rs1: %u rs2: %u rd: %u\n", rs1, rs2, rd);
                        x[rd] = x[rs1] << x[rs2];
                        pc += 4;
                        break;

                    //Set Lower Than
                    case FUNC3_SLT:
                        printf("SLT rs1: %u rs2: %u rd: %u\n", rs1, rs2, rd);
                        x[rd] = x[rs1] < x[rs2]? 0b1 : 0b0;
                        pc += 4;
                        break;

                    //Set Lower Than Unsigned
                    case FUNC3_SLTU:
                        printf("SLTU rs1: %u rs2: %u rd: %u\n", rs1, rs2, rd);
                        x[rd] = (uint32_t) x[rs1] < (uint32_t) x[rs2];
                        pc += 4;
                        break;

                    //eXlusive OR
                    case FUNC3_XOR:
                        printf("XOR rs1: %u rs2: %u rd: %u\n", rs1, rs2, rd);
                        x[rd] = x[rs1] ^ x[rs2];
                        pc += 4;
                        break;

                    //Shift Right L/A
                    case FUNC3_SRx:
                        if(sum_bool){
                            //Shift Right Logical
                            printf("SRT rs1: %u rs2: %u rd: %u\n", rs1, rs2, rd);
                            x[rd] = (uint32_t) x[rs1] >> x[rs2];
                        }else{
                            //Shift Right Arithmetic
                            printf("SRA rs1: %u rs2: %u rd: %u\n", rs1, rs2, rd);
                            x[rd] = x[rs1] >> x[rs2];
                        }
                        pc += 4;
                        break;

                    //OR
                    case FUNC3_OR:
                        printf("OR rs1: %u rs2: %u rd: %u\n", rs1, rs2, rd);
                        x[rd] = x[rs1] | x[rs2];
                        pc += 4;
                        break;

                    //AND
                    case FUNC3_AND:
                        printf("AND rs1: %u rs2: %u rd: %u\n", rs1, rs2, rd);
                        x[rd] = x[rs1] & x[rs2];
                        pc += 4;
                        break;

                    default:
                        errx(EXIT_FAILURE, "ERROR: Couldn't match operation's func3 (instruction %x; opcode %x; func3 %x).", inst, opcode, func3);
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
                errx(EXIT_FAILURE, "ERROR: couldn't match operation code (instruction %x; opcode %x).",inst, opcode);
                return 1;

        }

        //if program counter (pc) is out of range
        if(pc>=max_pc){
            errx(EXIT_FAILURE, "ERROR: Program counter value (pc = %u) out of range (max_pc = %d)", pc, max_pc);
            return 1;
        }
    }

    printf("finish\n");
    return 0;
}