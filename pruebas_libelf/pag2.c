#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>

// Tamaño bloque (2**7 bytes == 2**10 bits)
#define BLOCK_SIZE 128

// Cantidad de bloques & tamaño tabla de bloques (2**3)
#define BLOCK_TABLE_SIZE 8

/* Structure block_entry
@param no           number of block_entry
@param start_addr   start address of block_entry
@param mem_init     Boolean to check if memory was initialized
@param mem          pointer to malloc array of size BLOCK_SIZE
*/
struct block_entry{ 
    int     no;         // Block's Number
    int     start_addr; // Block's start addr
    char    mem_init;   // Boolean to check if memory is initialized
    char*   mem;        // Block's memory pointer
};

/* Initialize the memory of a block entry if not initialized.
@param be pointer to block_entry structure
@return 0 if succesful
*/
int init_block_entry_memory(struct block_entry *be){
    if(be->mem_init==0){
        be->mem = (char *) malloc(BLOCK_SIZE);
        be->mem_init = 1;
    }
    return 0;
}

/* Cleanup block_entry freeing malloc memory if initialized.
@param be pointer to block_entry structure
@return 0 if succesful
*/
int clean_block_entry_memory(struct block_entry *be){
    if(be->mem_init==1){
        free(be->mem);
        be->mem_init=0;
        //printf("memory freed\n");
    }
    return 0;
}

/* Initializes block_table
@return pointer to memory allocated containing the block_table
*/
struct block_entry *init_block_table(){
    struct block_entry *bt = (struct block_entry *) malloc( sizeof(struct block_entry) * BLOCK_TABLE_SIZE );
    for(int i=0;i<BLOCK_TABLE_SIZE;i++){
        bt[i].no = i;
        bt[i].start_addr = i*BLOCK_SIZE;
        bt[i].mem_init = 0;
    }
    return bt;
}

/* Cleanup allocated memory in block_table
@return 0 if succesful
*/
int clean_block_table(struct block_entry* be){
    for(int i=0;i<BLOCK_TABLE_SIZE;i++){
        /*
        if(be[i].mem_init){
            printf("Block %i:",i);
        }        
        */
        clean_block_entry_memory(&be[i]);
    }
}

/* Writes the char given in the block_table at the logical address given
@param bt block_table containing all block_entry
@param logical_addr logical address to write
@param c character to be written
@return 0 if succesful
*/
int write_in(struct block_entry *bt, unsigned int logical_addr, char c){

    if(logical_addr >= BLOCK_SIZE*BLOCK_TABLE_SIZE){
        errx(EXIT_FAILURE, "<write_in> ERROR: %u out of limits\n", logical_addr);
        return 1;
    }
    
    int block_no = logical_addr >> 7;
    int offset   = logical_addr & BLOCK_SIZE-1;

    if(!bt[block_no].mem_init){
        init_block_entry_memory(&bt[block_no]);
    }
    
    bt[block_no].mem[offset]=c;
    //printf("#%i , offset %i \n",block_no,offset);
    return 0;
}

/* Reads the block_table at the logical address given
@param bt block_table containing all block_entry
@param logical_addr logical address to read
@return character in logical address
*/
char read_from(struct block_entry *bt, unsigned int logical_addr){
    if(logical_addr >= BLOCK_SIZE*BLOCK_TABLE_SIZE){
        errx(EXIT_FAILURE, "<read_from> ERROR: %u out of limits\n", logical_addr);
        return 1;
    }
    
    int block_no = logical_addr >> 7;
    int offset   = logical_addr & BLOCK_SIZE-1;

    if(!bt[block_no].mem_init){
        errx(EXIT_FAILURE, "<read_from> ERROR: reading uninitialized block_entry memory\n");
        return 1;
    }

    //printf("#%i , offset %i \n",block_no,offset);
    return bt[block_no].mem[offset];
}

int main(int argc, char **argv){
    unsigned int logical_addr = atoi(argv[1]);
    char C = 'C';

    struct block_entry *block_table = init_block_table();
    
    if ( write_in(block_table,logical_addr,C) ){
        return 1;
    }

    printf("mem[%u]=%c\n",logical_addr,read_from(block_table,logical_addr));
    
    clean_block_table(block_table);

    return 0;
}