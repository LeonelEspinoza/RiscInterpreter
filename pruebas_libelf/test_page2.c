#include "pag2.c"
#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

int main(int argc, char **argv){
    
    if(argc!=2){
        printf("Usage: %s <int>\n", argv[0]);
        return 1;
    }

    // ----------
    // Init 
    // ----------
    uint32_t logical_addr = atoi(argv[1]);

    printf("logical_addr: %u\n\n",logical_addr);
    int n = 16;
    uint32_t buf[n];
    for(int i =0; i<n;i++){
        buf[i] = rand() % UINT32_MAX;
        printf("buff[%d]= %u\n",i,buf[i]);
    }

    // ----------
    // write_in_bulk
    // ----------
    printf("\nTEST: write_in_bulk()\n");
    struct block_entry *block_table = init_block_table();
    size_t buf_size = sizeof(buf);
    if (write_in_bulk(block_table, logical_addr, buf, buf_size) != buf_size){
        printf("Bytes written != bytes to write.\n");
        return 1;
    }

    // ----------
    // read_from_bulk
    // ----------
    printf("\nTEST: read_from_bulk()\n");
    size_t content_size = n*sizeof(uint32_t);
    uint32_t* buf2 = malloc(content_size);
    printf("content_size: %lu\n", content_size);
    if(read_from_bulk(block_table, logical_addr, buf2, content_size) != content_size){
        printf("Bytes read != bytes to read\n");
        return 1;
    }
    for(int i =0; i <n; i++){
        printf("buf = %u | mem = %u\n", buf[i], buf2[i]);
    }
    free(buf2);

    // ----------
    // ptr_to
    // ----------
    printf("\nTEST: ptr_to()\n");
    uint32_t *ptr;
    for(int i=0;i<n;i++){
        ptr =(uint32_t*) ptr_to(block_table, logical_addr);
        printf("buf = %u | mem = %u\n", buf[i], *ptr);
        logical_addr+=4;
    }

    clean_block_table(block_table);
    
    return 0;
}