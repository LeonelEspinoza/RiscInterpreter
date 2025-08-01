#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <malloc.h>

// Block size (2MB = 2**21 bytes)
#define BLOCK_SIZE 2097152

// Number of blocks & block table size (2**11)
#define BLOCK_TABLE_SIZE 2048

/* Structure block_entry
@param no           number of block_entry
@param start_addr   start address of block_entry
@param mem_init     Boolean to check if memory was initialized
@param mem          pointer to malloc array of size BLOCK_SIZE
*/
typedef struct block_entry{ 
    int     no;         // Block's Number
    int     start_addr; // Block's start addr
    char    mem_init;   // Boolean to check if memory is initialized
    uint8_t*   mem;     // Block's memory pointer
}block_entry;

/* Initialize the memory of a block entry if not initialized.
@param be pointer to block_entry structure
@return 0 if succesful
*/
int init_block_entry_memory(block_entry *be){
    if(be->mem_init==0){
        be->mem = (uint8_t *) malloc(BLOCK_SIZE);
        be->mem_init = 1;
    }
    return 0;
}

/* Cleanup block_entry freeing malloc memory if initialized.
@param be pointer to block_entry structure
@return 0 if succesful
*/
int clean_block_entry_memory(block_entry *be){
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
block_entry *init_block_table(){
    block_entry *bt = (block_entry *) malloc( sizeof(block_entry) * BLOCK_TABLE_SIZE );
    for(int i=0;i<BLOCK_TABLE_SIZE;i++){
        bt[i].no = i;
        //i * (2**21)
        bt[i].start_addr = i<<21;
        bt[i].mem_init = 0;
    }
    return bt;
}

/* Cleanup allocated memory in block_table
@return 0 if succesful
*/
int clean_block_table(block_entry* be){
    for(int i=0;i<BLOCK_TABLE_SIZE;i++){
        /*
        if(be[i].mem_init){
            printf("Block %i:",i);
        }        
        */
        clean_block_entry_memory(&be[i]);
    }
}

/* Writes the byte given in the block_table at the logical address given
@param bt block_table containing all block_entry
@param logical_addr logical address to write
@param c character to be written
@return 0 if succesful

Error code 1 -> block number out of limits

Error code 2 -> offset out of limits
*/
int write_in(block_entry *bt, uint32_t logical_addr, uint8_t c){
    
    uint32_t block_no = logical_addr >> 21;
    uint32_t offset   = logical_addr & 0b111111111111111111111;
    
    if(block_no >= BLOCK_TABLE_SIZE){
        printf("<write_in> ERROR: block number (%u) assigned to addres \"%u\" is out of limits\n", block_no, logical_addr);
        return 1;
    }
    if(offset >= BLOCK_SIZE){
        printf("<write_in> ERROR: offset (%u) assigned to addres \"%u\" is out of limits\n", offset, logical_addr);
        return 2;
    }

    if(!bt[block_no].mem_init){
        init_block_entry_memory(&bt[block_no]);
    }
    
    bt[block_no].mem[offset]= c;

    return 0;
}

/* Checks state of address given
@param bt block_table containing all block_entry
@param logical_addr logical address to read
@return 0 if succesful; 1 if block number out of limits; 2 if offset out of limits; 3 if block entry not initialized.
*/
uint8_t check_address(block_entry *bt, uint32_t logical_addr){

    uint32_t block_no = logical_addr >> 21;
    uint32_t offset   = logical_addr & 0b111111111111111111111;

    if(block_no >= BLOCK_TABLE_SIZE){
        printf("<read_from> ERROR: block number (%u) assigned to address \"%u\" is out of limits\n", block_no, logical_addr);
        return 1;
    }
    if(offset >= BLOCK_SIZE){
        printf("<read_from> ERROR: offset (%u) assigned to address \"%u\" is out of limits\n", offset, logical_addr);
        return 2;
    }

    if(!bt[block_no].mem_init){
        //printf("<read_from> Warning: reading uninitialized block_entry memory\n");
        return 3;
    }
    //printf("#%i , offset %i \n",block_no,offset);
    return 0; 
}

/* Reads the block_table at the logical address given
@param bt block_table containing all block_entry
@param logical_addr logical address to read
@param ret  var to store the byte at the logical address
@return 0 if succesful

Error code 1 -> block number out of limits

Error code 2 -> offset out of limits

Error code 3 -> Reading uninitialized address
*/
uint8_t read_from(block_entry *bt, uint32_t logical_addr, uint8_t *ret){

    uint32_t block_no = logical_addr >> 21;
    uint32_t offset   = logical_addr & 0b111111111111111111111;

    if(block_no >= BLOCK_TABLE_SIZE){
        printf("<read_from> ERROR: block number (%u) assigned to address \"%u\" is out of limits\n", block_no, logical_addr);
        return 1;
    }
    if(offset >= BLOCK_SIZE){
        printf("<read_from> ERROR: offset (%u) assigned to address \"%u\" is out of limits\n", offset, logical_addr);
        return 2;
    }

    if(!bt[block_no].mem_init){
        printf("<read_from> Warning: reading uninitialized block_entry memory\n");
        return 3;
    }
    //printf("#%i , offset %i \n",block_no,offset);
    *ret = bt[block_no].mem[offset];
    return 0; 
}

/* Return pointer to logical addres given
@param bt block_table containing all block_entry
@param logical_addr logical address to point
@return pointer to logical_addr
*/
uint8_t *ptr_to(block_entry *bt, uint32_t logical_addr){
       
    uint32_t block_no = logical_addr >> 21;
    uint32_t offset   = logical_addr & 0b111111111111111111111;

    if(block_no >= BLOCK_TABLE_SIZE){
        errx(EXIT_FAILURE, "<ptr_to> ERROR: block number (%u) assigned to addres \"%u\" is out of limits\n", block_no, logical_addr);
        return NULL;
    }
    if(offset >= BLOCK_SIZE){
        errx(EXIT_FAILURE, "<ptr_to> ERROR: offset (%u) assigned to addres \"%u\" is out of limits\n", offset, logical_addr);
        return NULL;
    }

    if(!bt[block_no].mem_init){
        errx(EXIT_FAILURE, "<ptr_to> ERROR: pointer to uninitialized block_entry memory\n");
        return NULL;
    }

    uint8_t *ret = &(bt[block_no].mem[offset]);

    return ret;
}

/* Writes buffer given starting at given logical address
@param bt block_table containing all block_entry
@param start_logical_addr logical address to start writing at
@param buf buffer with content to be written in block_table
@param content_size total size of content to be written in bytes
@return bytes written
*/
size_t write_in_bulk(block_entry *bt, uint32_t start_logical_addr, void *buf, size_t content_size){
    
    uint32_t logical_addr = start_logical_addr;

    for(size_t i=0; i < content_size; i++, logical_addr++){
        if(write_in(bt, logical_addr, ((uint8_t*)buf)[i])){
            printf("<write_in_bulk> ERROR: filed to write at %u address. Wrote %lu bytes.\n", logical_addr, i);
            return i;
        }
    }

    return logical_addr - start_logical_addr;
}

/* Reads starting at given logical address and saves content in buffer
@param bt block_table containing all block_entry
@param start_logical_addr logical address to start reading at
@param buf buffer for content read
@param content_size total size of content to be read in bytes
@return bytes read
*/
size_t read_from_bulk(block_entry *bt, uint32_t start_logical_addr, void *buf, size_t content_size){    
    uint32_t logical_addr = start_logical_addr;
    
    for( size_t i = 0; i < content_size; i++, logical_addr++){
        if(read_from(bt, logical_addr,&(((uint8_t *)buf)[i]))){
            printf("<read_from_bulk> ERROR: couldn't read byte from logical address: %u\n", logical_addr);
            return i;
        }
    }

    return logical_addr - start_logical_addr;
}