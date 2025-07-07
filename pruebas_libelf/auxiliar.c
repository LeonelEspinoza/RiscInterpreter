#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <err.h>

#include "pag2.c"
//#include "backlog.c"

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

#define BACKLOG_SIZE 101

/* Structure back_step
@param inst_addr    instruction address
@param mod_val      modified value 
*/
typedef struct back_step{
    uint32_t inst_addr;
    uint32_t mod_val;
}back_step;


/*Structure backlog
@param steps        back_step circular array
@param bot          pointer to botom back_step
@param top          pointer to top back_step
*/
typedef struct backlog{
    back_step* steps;
    uint32_t bot;
    uint32_t top;
}backlog;

/* Initialize backlog array and pointers
@param b            pointer to backlog
*/
void init_backlog(backlog *b){
    b->steps = (back_step *) malloc(BACKLOG_SIZE * sizeof(back_step));
    b->top = 0;
    b->bot = 0;
    return;
}

/* Cleanup backlog malloc array
@param b            pointer to backlog
*/
void free_backlog(backlog *b){
    free(b->steps);
    return;
}

/* Saves new back_step in backlog
@param b            pointer to backlog
@param inst_addr    instruction's address
@param mod_val      modified value 
*/
void add_step(backlog *b,uint32_t inst_addr,uint32_t mod_val){
    back_step bs;
    bs.inst_addr = inst_addr;
    bs.mod_val = mod_val;

    uint32_t bot = b->bot;
    uint32_t tmp = (b->top+1)%BACKLOG_SIZE;
    if(tmp==bot){
        b->bot = (bot+1)%BACKLOG_SIZE;
    }
    b->top = tmp;
    (b->steps)[tmp] = bs;
    return;
}

/* Gets top back_step in backlog
@param b            pointer to backlog
@return back_step 
*/
back_step go_back(backlog *b){
    uint32_t top = b->top;
    uint32_t bot = b->bot;
    if(top==bot){
        errx(EXIT_FAILURE, "ERROR: No more backsteps stored.\n");
    }
    back_step ret = (b->steps)[top];
    b->top = top==0? BACKLOG_SIZE-1 : top - 1;
    return ret;
}

/* Structure instruction_values
@param addr     address of instruction
@param inst     instruction
@param opcode   operation code
@param rd       register destination
@param func3    auxiliary term
@param rs1      resgister source 1
@param rs2      resgister source 2
@param imm      immediate value
@param uimm     unsigned immediate value
*/
typedef struct instruction_values{
    uint32_t addr;
    uint32_t inst;
    uint8_t opcode;
    uint8_t rd;
    uint8_t func3;
    uint8_t rs1;
    uint8_t rs2;
    int32_t  imm;
    uint32_t uimm;
}inst_val;

/* Sets addr, inst, opcode, rd, func3, rs1 and rs2 values from instruction
@param inst     instruction
@param addr     instruction address
@return         inst_val structure with values
*/
inst_val set_inst_val(uint32_t inst, uint32_t addr){
    inst_val iv;
    iv.addr = addr;
    iv.inst = inst;
    iv.opcode = inst       & 0b1111111;    
    iv.rd =    (inst>>7)   & 0b11111;
    iv.func3 = (inst>>12)  & 0b111;
    iv.rs1 =   (inst>>15)  & 0b11111;
    iv.rs2 =   (inst>>20)  & 0b11111;
    return iv;
}

/* Checks if register is 0 before assigning its value
@param reg_ar   register array
@param reg      register to write in
@param val      value to save in register
@return previus value on register (modified value)
*/
uint32_t inline assign_reg_val(int32_t* reg_ar, int8_t reg, int32_t val){
    uint32_t ret = reg_ar[reg];
    if(reg){
        reg_ar[reg]=val;
    }
    return ret;
}

/*go back from store byte or store halfword
@param bt       block_table pointer
@param reg_ar   register array
@param iv       instruction values
@param mod      modified value
*/
void inline back_store(block_entry *bt, int32_t *reg_ar, inst_val iv, int32_t mod){
    // get immediate
    iv.imm = ((iv.inst & MASK_31_25) >> 20) | ((iv.inst & MASK_11_07) >> 7);
    
    // bit sign extender
    if(iv.imm & 0b100000000000){
        iv.imm = iv.imm | 0b11111111111111111111100000000000;
    }
    
    // write in memory
    size_t size_tmp = sizeof(int32_t);
    if(iv.func3 == 0b000){
        size_tmp = sizeof(int8_t);
    }
    if(iv.func3 == 0b001){
        size_tmp = sizeof(int16_t);
    }
    write_in_bulk(bt, reg_ar[iv.rs1 + iv.imm], &mod, size_tmp);
    return;
}

/* Reset modified values back and returns address of next instruction to execute (the same that was undone)
@param b            pointer to backlog
@param bt           pointer to block_table (memory structure)
@param reg_ar       register array
*/
uint32_t exe_go_back(backlog *bl, block_entry *bt, int32_t *reg_ar){
    back_step bs = go_back(bl);
    inst_val iv;
    printf("instruction addres: %u\n", bs.inst_addr);
    read_from_bulk(bt, bs.inst_addr, &(iv.inst), sizeof(uint32_t));
    printf("instruction: %u\n", iv.inst);

    iv = set_inst_val(iv.inst, bs.inst_addr);
    // if opcode != BRANCH
    if(iv.opcode != 0b1100011){
        // if opcode == STORE
        if(iv.opcode == 0b0100011){
            back_store(bt, reg_ar, iv, bs.mod_val);
        }
        // opcode isnt BRANCH nor STORE
        else{
            assign_reg_val(reg_ar,iv.rd,bs.mod_val);
        }
    }

    return iv.addr;
}