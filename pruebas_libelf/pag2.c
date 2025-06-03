#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <malloc.h>

// Tamaño bloque (2MB = 2**21 bytes == 2**24 bits)
#define BLOCK_SIZE 2097152

// Cantidad de bloques & tamaño tabla de bloques (2**8)
#define BLOCK_TABLE_SIZE 256

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
    uint32_t*   mem;        // Block's memory pointer
};

/* Initialize the memory of a block entry if not initialized.
@param be pointer to block_entry structure
@return 0 if succesful
*/
int init_block_entry_memory(struct block_entry *be){
    if(be->mem_init==0){
        be->mem = (uint32_t *) malloc(BLOCK_SIZE);
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
int write_in(struct block_entry *bt, unsigned long logical_addr, uint32_t c){

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
uint32_t read_from(struct block_entry *bt, unsigned long logical_addr){
    if(logical_addr >= BLOCK_SIZE*BLOCK_TABLE_SIZE){
        errx(EXIT_FAILURE, "<read_from> ERROR: %u out of limits\n", logical_addr);
        return 1;
    }
    
    uint32_t block_no = logical_addr >> 7;
    uint32_t offset   = logical_addr & BLOCK_SIZE-1;

    if(!bt[block_no].mem_init){
        errx(EXIT_FAILURE, "<read_from> ERROR: reading uninitialized block_entry memory\n");
        return 1;
    }

    //printf("#%i , offset %i \n",block_no,offset);
    return bt[block_no].mem[offset];
}

/* Return pointer to logical addres given
@param bt block_table containing all block_entry
@param logical_addr logical address to point
@return pointer to logical_addr
*/
uint32_t *ptr_to(struct block_entry *bt, uint32_t logical_addr){
    if(logical_addr >= BLOCK_SIZE*BLOCK_TABLE_SIZE){
        errx(EXIT_FAILURE, "<read_from> ERROR: %u out of limits\n", logical_addr);
        return 1;
    }
    
    uint32_t block_no = logical_addr >> 7;
    uint32_t offset   = logical_addr & BLOCK_SIZE-1;

    if(!bt[block_no].mem_init){
        errx(EXIT_FAILURE, "<read_from> ERROR: reading uninitialized block_entry memory\n");
        return 1;
    }

    uint32_t *ret = &(bt[block_no].mem[offset]);

    return ret;
}

/* Writes buffer given starting at given logical address
@param bt block_table containing all block_entry
@param start_logical_addr logical address to start writing at
@param buf buffer with content to be written in block_table
@param content_size total size of content to be written in bytes
@return bytes written
*/
int write_in_chunk(struct block_entry *bt, unsigned long start_logical_addr, uint32_t *buf, size_t content_size){
    // si me entragan x bytes ocupo x/4 addr

    // cantidad de items uint32_t en buffer
    long n = (long)(content_size/4);
    n = content_size%4 == 0 ? n : n+1;

    // un item uint32_t por address
    unsigned long final_logical_addr = start_logical_addr + n;

    if(start_logical_addr >= BLOCK_SIZE*BLOCK_TABLE_SIZE){
        errx(EXIT_FAILURE, "<write_in_chunk> ERROR: starting logical address %u out of limits\n", start_logical_addr);
        return 0;
    }

    if((final_logical_addr-1)>= BLOCK_SIZE*BLOCK_TABLE_SIZE >= BLOCK_SIZE*BLOCK_TABLE_SIZE){
        errx(EXIT_FAILURE, "<write_in_chunk> ERROR: final logical address %u out of limits. Content to write in block_table is too big.\n", final_logical_addr);
        return 0;
    }
    
    // Loop para cada direccion de memoria a escribir
    unsigned long logical_addr = start_logical_addr;
    size_t  i = 0;
    while(logical_addr < final_logical_addr && i< n){
        int block_no = logical_addr >> 7;
        int offset   = logical_addr & BLOCK_SIZE-1;

        if(!bt[block_no].mem_init){
            init_block_entry_memory(&bt[block_no]);
        }
        
        bt[block_no].mem[offset]=buf[i];
        
        logical_addr++;
        i++;
    }

    return i*4;
}

/*
int main(int argc, char **argv){
    unsigned int logical_addr = atoi(argv[1]);
    uint32_t C = 1221;

    struct block_entry *block_table = init_block_table();
    
    if ( write_in(block_table,logical_addr,C) ){
        return 1;
    }

    printf("mem[%u]=%u\n",logical_addr,read_from(block_table,logical_addr));
    
    clean_block_table(block_table);
    
    return 0;
}
*/