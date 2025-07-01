#include <stdlib.h>
#include <stdint.h>
#include "pag2.c"

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

/* Structure instruction_values
@param addr     address of instruction
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
@return inst_val structure with values
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
*/
void inline assign_reg_val(int32_t* reg_ar, int8_t reg, int32_t val){
    if(reg){
        reg_ar[reg]=val;
    }
    return;
}


/*go back from assign rd (LUI, AUIPC, JAL, JALR, LOAD, OP)
@param bt       block_table pointer
@param reg_ar   register array
@param iv       instruction values
@param mod1     modified value 1
@param mod2     modified value 2
*/
void back_rd_assign(block_entry *bt, int32_t *reg_ar, inst_val iv, int32_t mod1, int32_t mod2){
    assign_reg_val(reg_ar,iv.rd,mod1);
    return;
}

/*go back from store byte or store halfword
@param bt       block_table pointer
@param reg_ar   register array
@param iv       instruction values
@param mod1     modified value 1
@param mod2     modified value 2
*/
void back_store(block_entry *bt, int32_t *reg_ar, inst_val iv, int32_t mod1, int32_t mod2){
    iv.imm = ((iv.inst & MASK_31_25) >> 20) | ((iv.inst & MASK_11_07) >> 7);
    if(iv.imm & 0b100000000000){
        iv.imm = iv.imm | 0b11111111111111111111100000000000;
    }
    if(iv.func3 == 0b010){
        assign_reg_val(reg_ar, iv.rs2, mod1);
        write_in_bulk(bt, reg_ar[iv.rs1 + iv.imm], &mod2, sizeof(uint32_t));
        return;
    }
    write_in_bulk(bt, reg_ar[iv.rs1 + iv.imm], &mod1, sizeof(uint32_t));
    return;
}