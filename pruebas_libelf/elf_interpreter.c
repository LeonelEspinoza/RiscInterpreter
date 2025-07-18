#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <elf.h>
#include <libelf.h>
#include <gelf.h>
#include <fcntl.h>

//#include "pag2.c"
#include "auxiliar.c"

#define FALSE 0
#define TRUE !0

#define LUI     0b0110111 //0x37
#define AUIPC   0b0010111 //0x17
#define JAL     0b1101111 //0x6f
#define JALR    0b1100111 //0x67

#define BRANCH  0b1100011 //0x63
#define FUNC3_BEQ   0b000   //0x0
#define FUNC3_BNE   0b001   //0x1
#define FUNC3_BLT   0b100   //0x4
#define FUNC3_BGE   0b101   //0x5
#define FUNC3_BLTU  0b110   //0x6
#define FUNC3_BGEU  0b111   //0x7

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
#define FUNC3_SLLI  0b001   //0x1
#define FUNC3_SLTI  0b010   //0x2
#define FUNC3_SLTIU 0b011   //0x3
#define FUNC3_XORI  0b100   //0x4
#define FUNC3_SRxI  0b101   //0x5
#define FUNC3_ORI   0b110   //0x6
#define FUNC3_ANDI  0b111   //0x7

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
#define MASK_11_08  0b00000000000000000000111100000000
#define MASK_11_07  0b00000000000000000000111110000000
#define MASK_07     0b00000000000000000000000001000000

/* Checks if register is 0 before assigning its value
@param reg_ar   register array
@param reg      register to write in
@param val      value to save in register
void inline assign_reg_val(int32_t* reg_ar, int8_t reg, int32_t val){
    if(reg){
        reg_ar[reg]=val;
    }
    return;
}
*/

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
    block_entry *block_table;
    
    // Initialize block table
    block_table = init_block_table();

    //printf("End Initialize\n");
    
    // ------------------------------------------------
    // Get _start address
    // ------------------------------------------------
    
    printf("\nGetting _start address\n");
    
    // Elf Scene pointer
    Elf_Scn *scn = NULL;
    
    // GElf section header
    GElf_Shdr shdr;
    
    // Elf data pointer
    Elf_Data *data = NULL;
    
    // GElf symbol
    GElf_Sym sym;
    
    // Elf uint32_t _start address
    Elf32_Addr _start_addr;
    
    // while scene isn't Null
    while ((scn = elf_nextscn(elf, scn)) != NULL) {
        
        // Retrieve the section header
        gelf_getshdr(scn, &shdr);
        
        // If section header type is symtab -> search for _start address
        if (shdr.sh_type == SHT_SYMTAB) {
            
            // Get data inside symbol section
            data = elf_getdata(scn, NULL);

            // Size of section / entry size of section = N entries in section
            int count = shdr.sh_size / shdr.sh_entsize;
            
            // for every entry in section
            for(int i=0; i < count; i++){
        
                // Get symbol
                gelf_getsym(data, i, &sym);
            
                // search for _start symbol
                if(strcmp(elf_strptr(elf, shdr.sh_link, sym.st_name),"_start") == 0){
            
                    // Save start address for later
                    _start_addr = sym.st_value;
                    printf("_start address: 0x%08x \n", _start_addr);
            
                    //break for loop
                    break;
                }
            }
        
            // Break while loop
            break;
        }
    }

    //printf("Saved _start address\n");

    // ------------------------------------------------
    // Load program headers to memory in block_table
    // ------------------------------------------------

    printf("\nBegin loader to memory\n");
    
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
    
        // Write buffer with data in block_table 
        if(write_in_bulk(block_table, vaddr, buf, memsz) < memsz){
            errx(EXIT_FAILURE, "ERROR: Number of bytes to write and bytes written doesn't match.");
        }

        // buf free malloc
        free(buf);
    }
    
    // phdr_ar free malloc
    free(phdr_ar);

    //printf("End loader to memory\n");

    // ------------------------------------------------
    // Interpreter
    // ------------------------------------------------

    printf("\nBegin interpreter\n");
    
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

    // Pseudo memory is block table
    
    // Program Counter - Instruction address
    uint32_t pc = _start_addr;

    // Next program counter - instruction address
    uint32_t next_pc = pc;
    
    // Instruction
    uint32_t inst;

    //instruction values structure
    //inst_val inst_v;

    //backlog structure
    backlog back_log;
    init_backlog(&back_log);
    uint32_t mod_val;

    // input
    char input;

    // bools
    uint8_t flag_error = FALSE;
    uint8_t stop = TRUE;
    uint8_t details = FALSE;
    uint64_t i = 0;

    while(TRUE){
        // Ask for user input
        if(stop){
            printf("Input: break(B) next-step(n) back-step(b) next-breakpoint(N) View-Registers-Values(v) Toggle-instruction-details(d) \n>");
            scanf(" %1c", &input);
            printf("\n");
            // End execution
            if(input == 'B'){
                break;
            }
            // Print register values in table (2x16)
            if(input == 'v'){
                for(int i =0; i<32; i+=2){
                    printf("x[%2u] = %10d; x[%2u] = %10d;\n", i, x[i], i+1, x[i+1]);
                }
                printf("\n");
                continue;
            }
            // Toggle to print instruction details (program counter)
            if(input == 'd'){
                details= !details;
                printf("Show more instruction details: %s\n", details? "True" : "False");
                continue;
            }
            if(input == 'b'){
                if(back_log.bot==back_log.top){
                    printf("No more backsteps stored.\n");
                    continue;
                }
                next_pc = exe_go_back(&back_log, block_table, x);
                pc = next_pc;
                continue;
            }
            // Toggle stop flag and wait till next stop
            if(input == 'N'){
                stop=FALSE;
            }else{ 
                // Check valid input
                if(input != 'n'){
                    printf("ERROR: Input not recognized. Enter a valid input.\n");
                    continue;
                }
            }
        }else{
            // Wait for x instructions and toggle stop bool
            i++;
            if(i>=99){
                i=0;
                stop=TRUE;
            }
        }

        mod_val = 0;
        
        
        // Check program counter value is valid
        if(next_pc > max_vaddr || next_pc < min_vaddr){
            printf("ERROR: trying to execute an out of limit instruction. (min_vaddr: %u, max_vaddr: %u, instruction addr: %u)\n", min_vaddr, max_vaddr, next_pc);
            stop=TRUE;
            flag_error=TRUE;
            continue;
        }
        
        // Update program counter
        if(!flag_error){
            pc = next_pc;
        }

        flag_error = FALSE;

        // Get instruction to interpret
        if(read_from_bulk(block_table, pc, &inst, 4)!= 4){
            printf("ERROR: could't read instruction at: %u\n", pc);
            stop = TRUE;
            flag_error=TRUE;
            continue;
        }
        
        // Prepare possible next instruccion address
        next_pc = pc + 4;
        
        /*
        //set instruction values
        inst_v = set_inst_val(inst, pc);
        */

        // Get operation code in instruction
        uint8_t opcode = inst       & 0b1111111;
        
        // Get register destination (rd) in instruction
        uint8_t rd =    (inst>>7)   & 0b11111;
        
        // Get func3 in instruction
        uint8_t func3 = (inst>>12)  & 0b111;
        
        // Get register source 1 (rs1) in instruction
        uint8_t rs1 =   (inst>>15)  & 0b11111;
        
        // Get register source 2 (rd2) in instruction
        uint8_t rs2 =   (inst>>20)  & 0b11111;
        
        // Auxiliar immediate
        int32_t imm;
        
        // Auxiliar unsigned immediate
        uint32_t uimm;
        
        // If print details TRUE
        if(details){
            //print instruction details
            printf("pc: %u\n", pc);
            //printf("inst: 0x%x\n", inst);
            //printf("opcode: 0x%x\n", opcode);
            //printf("rd: %d\n", rd);
            //printf("func3: 0x%x\n", func3);
            //printf("rs1: %d\n", rs1);
            //printf("rs2: %d\n", rs2);
        }

        switch (opcode) {

            //Load Upper Immediate
            case LUI:
                //imm[31:12] , rd , opcode
                imm = (inst & 0b11111111111111111111000000000000);
                printf("LUI imm: %d rd: %u\n", imm, rd);
                mod_val = assign_reg_val(x,rd,imm);
                //printf("x[%u] = %d\n", rd, imm);
                break;

            //Add Upper Immediate to PC
            case AUIPC:
                //imm[31:12] , rd , opcode
                imm = (inst & 0b11111111111111111111000000000000);
                printf("AUIPC imm: %d rd: %u\n", imm, rd);
                mod_val = assign_reg_val(x,rd,pc+imm);
                //printf("x[%u] = %d\n",rd, x[rd]);
                break;

            //Jump And Link
            case JAL:
                //offset[20|10:01|11|19:12] , rd , opcode

                //inst [31|30:21|20|19:12] 
                //imm_ [20|10:01|11|19:12]
                imm = (inst & MASK_19_12) | ((inst & MASK_20) >> 9) | ((inst & MASK_30_21) >> 20) | ((inst & MASK_31) >> 11);

                //sign extend
                if(imm & 0b100000000000000000000){
                    imm = imm | 0b11111111111100000000000000000000; 
                }

                printf("JAL offset: %d rd: %u\n", imm, rd);
                mod_val = assign_reg_val(x,rd,next_pc);
                next_pc = pc + imm;
                break;

            //Jump And Link Register
            case JALR:
                //imm rs1 func3 rd opcode
                imm = (inst>>20) & 0b111111111111;

                //sign extender
                if(imm & 0b100000000000){
                    imm = imm | 0b11111111111111111111100000000000;
                }
                printf("JALR offset: %d rs1: %u rd: %u\n", imm, rs1, rd);
                mod_val = assign_reg_val(x,rd,next_pc);
                next_pc = (x[rs1] + imm) & (~1);
                break;

            //Branches
            case BRANCH:
                //imm[12|10:05] , rs2 , rs1 , func3 , imm[04:01|11] , opcode
                
                //inst [31|30:25|11:08|07]
                //imm [12|10:05|04:01|11]
                imm=0;
                imm = ((inst & MASK_31) >> 19) | ((inst & MASK_30_25) >> 20) | ((inst & MASK_11_08) >> 7) | ((inst & MASK_07) << 4);

                //sign extender
                if(imm & 0b1000000000000){
                    imm = imm | 0b11111111111111111111000000000000;
                }

                switch (func3) {

                    //Branch EQual
                    case FUNC3_BEQ:
                        printf("BEQ imm: %d rs1: %u rs2: %u\n", imm, rs1, rs2);
                        if(x[rs1] == x[rs2]) {
                            next_pc = pc + imm;
                        }
                        break;

                    //Branch Not Equal
                    case FUNC3_BNE:
                        printf("BNE imm: %d rs1: %u rs2: %u\n", imm, rs1, rs2);
                        if(x[rs1] != x[rs2]) {
                            next_pc = pc + imm;
                        }
                        break;

                    //Branch Lower Than
                    case FUNC3_BLT:
                        printf("BLT imm: %d rs1: %u rs2: %u\n", imm, rs1, rs2);
                        if(x[rs1] < x[rs2]) {
                            next_pc = pc + imm;
                        }
                        break;

                    //Branch Greater or Equal
                    case FUNC3_BGE:  
                        printf("BGE imm: %d rs1: %u rs2: %u\n", imm, rs1, rs2);
                        if(x[rs1] >= x[rs2]) {
                            next_pc = pc + imm;
                        }
                        break;

                    //Branch Lower Than Unsigned
                    case FUNC3_BLTU:  
                        printf("BLTU imm: %d rs1: %u rs2: %u\n", imm, rs1, rs2);
                        if(((uint32_t) x[rs1]) < ((uint32_t) x[rs2])) {
                            next_pc = pc + imm;
                        }
                        break;

                    //Branch greater Than Unsigned
                    case FUNC3_BGEU:  
                        printf("BGEU imm: %d rs1: %u rs2: %u\n", imm, rs1, rs2);
                        if(((uint32_t) x[rs1]) >= ((uint32_t) x[rs2])) {
                            next_pc = pc + imm;
                        }
                        break;

                    default:
                        errx(EXIT_FAILURE, "ERROR: Couldn't match branch's func3 (instruction 0x%x; opcode 0x%x; func3 0x%x).", inst, opcode, func3);
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
                
                int32_t mem = 0;

                switch (func3) {
                    //Load Byte
                    case FUNC3_LB:
                        printf("LB imm: %d rs1: %u rd: %u\n", imm, rs1, rd);
                        int8_t lb_mem = 0;

                        if(read_from_bulk(block_table, (x[rs1] + imm), &lb_mem, sizeof(lb_mem))!= sizeof(lb_mem)){
                            printf("ERROR: couldn't read all contents at: %u\n", x[rs1] + imm);
                            stop = TRUE;
                            flag_error=TRUE;
                            break;
                        }
                        
                        mem = lb_mem;
                        /*
                        mem = mem & 0b11111111;
                        //sign extender
                        if(mem & 0b10000000){
                            mem = mem | 0b11111111111111111111111110000000;
                        }
                        */
                        mod_val = assign_reg_val(x,rd,mem);
                        //printf("x[rd=%u] = %d\n",rd,x[rd]);
                        break;

                    //Load Halfword
                    case FUNC3_LH:
                        printf("LH imm: %d rs1: %u rd: %u\n", imm, rs1, rd);
                        int16_t lh_mem = 0;
                        if(read_from_bulk(block_table, (x[rs1] + imm), &lh_mem, sizeof(lh_mem))!=sizeof(lh_mem)){
                            printf("ERROR: couldn't read all contents at: %u\n", x[rs1] + imm);
                            stop = TRUE;
                            flag_error=TRUE;
                            break;
                        }
                        
                        mem = lh_mem;
                        /*
                        mem = mem & 0b1111111111111111;
                        if(mem & 0b1000000000000000){
                            mem = mem | 0b11111111111111111000000000000000;
                        }
                        */
                        mod_val = assign_reg_val(x,rd,mem);
                        //printf("x[rd=%u] = %d\n",rd,x[rd]);
                        break;

                    //Load Word
                    case FUNC3_LW:
                        printf("LW imm: %d rs1: %u rd: %u\n", imm, rs1, rd);

                        if(read_from_bulk(block_table, (x[rs1] + imm), &mem, sizeof(mem))!=sizeof(mem)){
                            printf("ERROR: couldn't read all contents at: %u\n", x[rs1] + imm);
                            stop = TRUE;
                            flag_error=TRUE;
                            break;
                        }
                        
                        mod_val = assign_reg_val(x,rd,mem);
                        //printf("x[rd=%u] = %d\n",rd,x[rd]);
                        break;

                    //Load Byte Unsigned
                    case FUNC3_LBU:
                        printf("LBU imm: %d rs1: %u rd: %u\n", imm, rs1, rd);
                        uint8_t lbu_mem = 0;
                        if(read_from_bulk(block_table, (x[rs1] + imm), &lbu_mem, sizeof(lbu_mem)) != sizeof(lbu_mem)){
                            printf("ERROR: couldn't read all contents at: %u\n", x[rs1] + imm);
                            stop = TRUE;
                            flag_error=TRUE;
                            break;
                        }
                        mod_val = assign_reg_val(x,rd,(int32_t) lbu_mem);
                        //printf("x[rd]: %d\n",x[rd]);
                        break;

                    //Load Halfword Unsigned
                    case FUNC3_LHU:
                        printf("LHU imm: %d rs1: %u rd: %u\n", imm, rs1, rd);
                        uint16_t lhu_mem = 0;
                        if(read_from_bulk(block_table, (x[rs1] + imm), &lhu_mem, sizeof(lhu_mem)) != sizeof(lhu_mem)){
                            printf("ERROR: couldn't read all contents at: %u\n", x[rs1] + imm);
                            stop = TRUE;
                            flag_error=TRUE;
                            break;
                        }
                        mod_val = assign_reg_val(x,rd,(int32_t) lhu_mem);
                        //printf("x[rd]: %d\n",x[rd]);
                        break;
                        
                    default:
                        errx(EXIT_FAILURE, "ERROR: Couldn't match load's func3 (instruction 0x%x; opcode 0x%x; func3 0x%x).", inst, opcode, func3);
                        return 1;
                }
                break;

            //Store
            case STORE:
                //imm[11:5] , rs2 , rs1 , func3 , imm[4:0] , opcode
                //inst_ [31:25|11:07]
                //imm   [11:05|04:00]
                imm = ((inst & MASK_31_25) >> 20) | ((inst & MASK_11_07) >> 7);
                
                //sign extender
                if(imm & 0b100000000000){
                    imm = imm | 0b11111111111111111111100000000000;
                }

                switch (func3) {
                    //Store Byte
                    case FUNC3_SB:
                        printf("SB imm: %d rs1: %u rs2: %u\n", imm, rs1, rs2);
                        int8_t sb_tmp = (x[rs2] & 0b11111111);
                        
                        //Check address is initialized or not
                        uint8_t _sb = check_address(block_table, (x[rs1] + imm + sizeof(sb_tmp)));
                        if(_sb){
                            if(_sb == 1 || _sb == 2){
                                printf("ERROR: address %u is out of limits", (x[rs1] + imm + sizeof(sb_tmp)));
                                stop = TRUE;
                                flag_error = TRUE;
                                break;
                            }
                            // Set mod_val as 0
                            mod_val = 0;
                        }else{
                            // Get value at address for back-step
                            if(read_from_bulk(block_table, (x[rs1] + imm), &(mod_val), sizeof(sb_tmp)) != sizeof(sb_tmp)){
                                printf("ERROR: couldn't read all contents at: %u\n", x[rs1] + imm);
                                stop = TRUE;
                                flag_error=TRUE;
                                break;
                            }
                        }
                        
                        if(write_in_bulk(block_table, (x[rs1] + imm), &(sb_tmp), sizeof(sb_tmp)) != sizeof(sb_tmp)){
                            printf("ERROR: couldn't store all contents at: %u\n", x[rs1] + imm);
                            stop = TRUE;
                            flag_error=TRUE;
                            break;
                        }

                        //printf("stored: %d\n", x[rs2]);
                        break;

                    //Store Halfword
                    case FUNC3_SH:
                        printf("SH imm: %d rs1: %u rs2: %u\n", imm, rs1, rs2);
                        int16_t sh_tmp = (x[rs2] & 0b1111111111111111);
                        
                        //Check address is initialized or not
                        uint8_t _sh = check_address(block_table, (x[rs1] + imm + sizeof(sh_tmp)));
                        if(_sh){
                            if(_sb == 1 || _sb == 2){
                                printf("ERROR: address %u is out of limits", (x[rs1] + imm + sizeof(sh_tmp)));
                                stop = TRUE;
                                flag_error = TRUE;
                                break;
                            }
                            // Set mod_val as 0
                            mod_val = 0;
                        }else{
                            // Get value at address for back-step
                            if(read_from_bulk(block_table, (x[rs1] + imm), &(mod_val), sizeof(sh_tmp)) != sizeof(sh_tmp)){
                                printf("ERROR: couldn't read all contents at: %u\n", x[rs1] + imm);
                                stop = TRUE;
                                flag_error=TRUE;
                                break;
                            }
                        }

                        if(write_in_bulk(block_table, (x[rs1] + imm), &(sh_tmp), sizeof(sh_tmp)) != sizeof(sh_tmp)){
                            printf("ERROR: couldn't store all contents at: %u\n", x[rs1] + imm);
                            stop = TRUE;
                            flag_error=TRUE;
                            break;
                        }
                        //printf("stored: %d\n", x[rs2]);
                        break;

                    //Store Word
                    case FUNC3_SW:
                        printf("SW imm: %d rs1: %u rs2: %u\n", imm, rs1, rs2);
                        
                        //Check address is initialized or not
                        uint8_t _sw = check_address(block_table, (x[rs1] + imm + sizeof(x[rs2])));
                        if(_sw){
                            if(_sw==1 || _sw==2){
                                printf("ERROR: address %u is out of limits", (x[rs1] + imm + sizeof(x[rs2])));
                                stop = TRUE;
                                flag_error = TRUE;
                                break;
                            }

                            // Set modified value as 0
                            mod_val = 0;
                        }else{
                            // Get value at address for back-step
                            if(read_from_bulk(block_table, (x[rs1] + imm), &(mod_val), sizeof(x[rs2])) != sizeof(x[rs2])){
                                printf("ERROR: couldn't read all contents at: %u\n", x[rs1] + imm);
                                stop = TRUE;
                                flag_error=TRUE;
                                break;
                            }
                        }

                        if(write_in_bulk(block_table, (x[rs1] + imm), &(x[rs2]), sizeof(x[rs2])) != sizeof(x[rs2])){
                            printf("ERROR: couldn't store all contents at: %u\n", x[rs1] + imm);
                            stop = TRUE;
                            flag_error=TRUE;
                            break;
                        }
                        printf("stored: %d\n", x[rs2]);
                        break;

                    default:
                        errx(EXIT_FAILURE, "ERROR: Couldn't match store's func3 (instruction 0x%x; opcode 0x%x; func3 0x%x).", inst, opcode, func3);
                        return 1;
                }
                break;

            //Operation with Immediate
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
                        printf("ADDI imm: %d rs1: %u rd: %u\n", imm, rs1, rd);
                        mod_val = assign_reg_val(x,rd,x[rs1] + imm);
                        break;

                    //Set Less Than Immediate
                    case FUNC3_SLTI:
                        printf("SLTI imm: %d rs1: %u rd: %u\n", imm, rs1, rd);
                        mod_val = assign_reg_val(x,rd,x[rs1] < imm ? 0b1 : 0b0);
                        break;

                    //Set Lower Than Immediate Unsigned
                    case FUNC3_SLTIU:
                        printf("SLTIU imm: %d rs1: %u rd: %u\n", imm, rs1, rd);
                        mod_val = assign_reg_val(x,rd,(uint32_t) x[rs1] < uimm ? 0b1 : 0b0);
                        break;

                    //eXclusive OR Immediate
                    case FUNC3_XORI:
                        printf("XORI imm: %d rs1: %u rd: %u\n", imm, rs1, rd);
                        mod_val = assign_reg_val(x,rd,x[rs1] ^ imm);
                        break;

                    //OR Immediate
                    case FUNC3_ORI :
                        printf("ORI imm: %d rs1: %u rd: %u\n", imm, rs1, rd);
                        mod_val = assign_reg_val(x,rd,x[rs1] | imm);
                        break;

                    //AND Immediate
                    case FUNC3_ANDI:
                        printf("ANDI imm: %d rs1: %u rd: %u\n", imm, rs1, rd);
                        mod_val = assign_reg_val(x,rd,x[rs1] & imm);
                        break;
                    
                    //Shift Logical Left Immediate
                    case FUNC3_SLLI:
                        printf("SLLI shamt: %u rs1: %u rd: %u\n", shamt, rs1, rd);
                        mod_val = assign_reg_val(x,rd,x[rs1] << shamt);
                        break;
                    
                    //Shift Right L/A Immediate
                    case FUNC3_SRxI:
                        
                        if (imm & 0b010000000000){
                            //Shift Right Arithmetic Immediate
                            printf("SRAI shamt: %u rs1: %u rd: %u\n", shamt, rs1, rd);
                            mod_val = assign_reg_val(x,rd,((signed) x[rs1]) >> shamt);
                        } else {
                            //Shift Right Logical Immediate
                            printf("SRLI shamt: %u rs1: %u rd: %u\n", shamt, rs1, rd);
                            mod_val = assign_reg_val(x,rd,((uint32_t) x[rs1]) >> shamt);
                        }
                        break;

                    default:
                        errx(EXIT_FAILURE, "ERROR: Couldn't match operation immediate's func3 (instruction 0x%x; opcode 0x%x; func3 0x%x)", inst, opcode, func3);
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
                            mod_val = assign_reg_val(x,rd,x[rs1] * x[rs2]);
                            break;
                        
                        //MULtiplication Half
                        case FUNC3_MULH:
                            printf("MULH rs1: %u rs2: %u rd: %u\n", rs1, rs2, rd);
                            mul = (x[rs1] * x[rs2]);
                            mod_val = assign_reg_val(x,rd,mul >> 32);
                            break;
                        
                        //MULtiplication Half Signed Unsigned
                        case FUNC3_MULHSU:
                            printf("MULHSU rs1: %u rs2: %u rd: %u\n", rs1, rs2, rd);
                            mul = ((int32_t) (x[rs1] * ((uint32_t) x[rs2])));
                            mod_val = assign_reg_val(x,rd,mul >> 32);
                            break;
                        
                        //MULtiplication Half Unsigned
                        case FUNC3_MULHU:
                            printf("MULHU rs1: %u rs2: %u rd: %u\n", rs1, rs2, rd);
                            uint64_t u_mul = (((uint32_t) x[rs1]) * ((uint32_t) x[rs2]));
                            mod_val = assign_reg_val(x,rd,u_mul >> 32);
                            break;
                        
                        //DIVision
                        case FUNC3_DIV:
                            printf("DIV rs1: %u rs2: %u rd: %u\n", rs1, rs2, rd);
                            mod_val = assign_reg_val(x,rd,x[rs1] / x[rs2]);
                            break;
                        
                        //DIVision Unsigned
                        case FUNC3_DIVU:
                            printf("DIVU rs1: %u rs2: %u rd: %u\n", rs1, rs2, rd);
                            mod_val = assign_reg_val(x,rd,((uint32_t) x[rs1]) / ((uint32_t) x[rs2]));
                            break;
                        
                        
                        //REMainder
                        case FUNC3_REM:
                            printf("REM rs1: %u rs2: %u rd: %u\n", rs1, rs2, rd);
                            mod_val = assign_reg_val(x,rd,x[rs1] % x[rs2]);
                            break;
                        
                        //REMainder Unsigned
                        case FUNC3_REMU:
                            printf("REMU rs1: %u rs2: %u rd: %u\n", rs1, rs2, rd);
                            mod_val = assign_reg_val(x,rd,((uint32_t) x[rs1]) % ((uint32_t) x[rs2]));
                            break;
                            
                        default:
                            errx(EXIT_FAILURE, "ERROR: Could not match Mul/Div func3 (instruction 0x%x; opcode 0x%x; func3 0x%x).", inst, opcode, func3);
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
                            mod_val = assign_reg_val(x,rd,x[rs1] - x[rs2]);
                        }else{
                            //ADD
                            printf("ADD rs1: %u rs2: %u rd: %u\n", rs1, rs2, rd);
                            mod_val = assign_reg_val(x,rd,x[rs1] + x[rs2]);
                        }
                        break;

                    //Shift Logical Left
                    case FUNC3_SLL:
                        printf("SLL rs1: %u rs2: %u rd: %u\n", rs1, rs2, rd);
                        mod_val = assign_reg_val(x,rd,x[rs1] << x[rs2]);
                        break;

                    //Set Lower Than
                    case FUNC3_SLT:
                        printf("SLT rs1: %u rs2: %u rd: %u\n", rs1, rs2, rd);
                        mod_val = assign_reg_val(x,rd,x[rs1] < x[rs2]? 0b1 : 0b0);
                        break;

                    //Set Lower Than Unsigned
                    case FUNC3_SLTU:
                        printf("SLTU rs1: %u rs2: %u rd: %u\n", rs1, rs2, rd);
                        mod_val = assign_reg_val(x,rd,(uint32_t) x[rs1] < (uint32_t) x[rs2]);
                        break;

                    //eXlusive OR
                    case FUNC3_XOR:
                        printf("XOR rs1: %u rs2: %u rd: %u\n", rs1, rs2, rd);
                        mod_val = assign_reg_val(x,rd,x[rs1] ^ x[rs2]);
                        break;

                    //Shift Right L/A
                    case FUNC3_SRx:
                        if(sum_bool){
                            //Shift Right Logical
                            printf("SRT rs1: %u rs2: %u rd: %u\n", rs1, rs2, rd);
                            mod_val = assign_reg_val(x,rd,(uint32_t) x[rs1] >> x[rs2]);
                        }else{
                            //Shift Right Arithmetic
                            printf("SRA rs1: %u rs2: %u rd: %u\n", rs1, rs2, rd);
                            mod_val = assign_reg_val(x,rd,x[rs1] >> x[rs2]);
                        }
                        break;

                    //OR
                    case FUNC3_OR:
                        printf("OR rs1: %u rs2: %u rd: %u\n", rs1, rs2, rd);
                        mod_val = assign_reg_val(x,rd,x[rs1] | x[rs2]);
                        break;

                    //AND
                    case FUNC3_AND:
                        printf("AND rs1: %u rs2: %u rd: %u\n", rs1, rs2, rd);
                        mod_val = assign_reg_val(x,rd,x[rs1] & x[rs2]);
                        break;

                    default:
                        errx(EXIT_FAILURE, "ERROR: Couldn't match operation's func3 (instruction 0x%x; opcode 0x%x; func3 0x%x).", inst, opcode, func3);
                        return 1;
                }
                break;
            
            case FENCE:
                int8_t fm,pred,succ;
                succ = ((inst >> 20) & 0b000000001111);
                pred = ((inst >> 24) & 0b00001111);
                fm   = ((inst >> 28) & 0b1111);
                //Fence(pred,succ)
                errx(EXIT_FAILURE, "ERROR: FENCE operation not implemented yet");
                return 1;
                break;
            
            case SYSTEM:
                if(inst != (0b1 << 19)){
                    //ECALL
                    //RiseException(EnviromentCall)
                    printf("ECALL\n");
                    errx(EXIT_FAILURE, "ERROR: ECALL operation not implemented yet");
                    return 1;
                }else{
                    //EBREAK
                    //RiseExeption(BreakPoint)
                    printf("EBREAK\n");
                    errx(EXIT_FAILURE, "ERROR: EBREAK operation not implemented yet");
                    return 1;
                }
                break;
            default:
                errx(EXIT_FAILURE, "ERROR: couldn't match operation code (instruction 0x%x; opcode 0x%x).",inst, opcode);
                return 1;

        }
        if(!flag_error){
            add_step(&back_log, pc, mod_val);
        }
    }

    printf("finish\n");
    return 0;
}

//bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb