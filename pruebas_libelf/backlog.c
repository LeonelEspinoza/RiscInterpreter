#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <err.h>

#include "pag2.c"
#include "auxiliar.c"

#define BACKLOG_SIZE 101
/* Structure back_step
@param inst_addr    instruction address
@param mod_val1     modified value 1
@param mod_val2     modified value 2
*/
typedef struct back_step{
    uint32_t inst_addr;
    uint32_t mod_val_1;
    uint32_t mod_val_2;
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
@param inst         instruction executed
@param mod_val_1    modified value 1
@param mod_val_2    modified value 2
*/
void add_step(backlog *b,uint32_t inst,uint32_t mod_val_1,uint32_t mod_val_2){
    back_step bs;
    bs.inst_addr = inst;
    bs.mod_val_1 = mod_val_1;
    bs.mod_val_2 = mod_val_2;

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

/* Reset modified values back
@param b            pointer to backlog
@param bt           pointer to block_table (memory structure)
void exe_go_back(backlog *b, block_entry * bt){
    back_step bs = go_back(b);
    uint32_t back_add = bs.inst_addr;
    uint32_t inst;
    read_from_bulk(bt, back_add, &inst, sizeof(inst));
    uint32_t mod_val1 = bs.mod_val_1;
    uint32_t mod_val2 = bs.mod_val_2;
    return;   
}
*/

void main(){
    uint32_t inst[200],mod[200],mod2[200];
    for(int i=0;i<200;i++){
        inst[i] = 1000 + i;
        mod[i]  = 2000 + i;
        mod2[i] = 3000 + i;
    }

    backlog bl;
    back_step bs;
    init_backlog(&bl);

    for(int i=0;i<150;i++){
        add_step(&bl, inst[i],mod[i],mod2[i]);
        printf("top: %u,bot: %u\n", bl.top,bl.bot);
    }

    int x = rand() % 100;
    printf("x: %u\n", x);
    for(int i=0; i<x; i++){
        bs = go_back(&bl);
        printf("inst: %u, mod1: %u, mod2: %u\n", bs.inst_addr, bs.mod_val_1, bs.mod_val_2);
        printf("top: %u,bot: %u\n", bl.top,bl.bot);
    }
    for(int i=150-x;i<200;i++){
        add_step(&bl, inst[i],mod[i],mod2[i]);
        printf("top: %u,bot: %u\n", bl.top,bl.bot);
    }
    free_backlog(&bl);
    return;
}