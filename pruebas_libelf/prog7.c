#include <err.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <gelf.h>
#include <libelf.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "pag2.c"

/*
prog7.c

Loads program headers from an ELF file into a block_entry array or block_table structure
*/

void main(int argc, char **argv) {

    // ------------------------------------------------
    // Initialize
    // ------------------------------------------------    
    
    // Check number of arguments is correct
    if (argc != 2){
        errx(EXIT_FAILURE, "usage: %s <ELF-file-name>", argv[0]);
    }
    
    // Initialize block table
    struct block_entry *block_table = init_block_table();
    
    // Save filename
    const char *filename = argv[1];
    
    // Necessary to call other libelf library functions
    if (elf_version(EV_CURRENT) == EV_NONE){ 
        err(EXIT_FAILURE, "ELF library initialization failed: %s", elf_errmsg(-1));
    }

    // Open file with fopen read
    FILE *f_in = fopen(filename, "r");
    if (f_in == NULL){
        errx(EXIT_FAILURE,"ERROR: fopen(%s,\"r\") failed", filename);
    }
    
    // (int) file descriptor for elf file handler
    int fd = 0;
    // Open file with open
    if ((fd = open(filename, O_RDONLY, 0)) < 0){
        err(EXIT_FAILURE, "ERROR: open \"%s\" failed", filename);
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

    // ------------------------------------------------
    // Load program headers to memory in block_table
    // ------------------------------------------------

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
    //printf("min vaddr: %lu\n",min_vaddr);
    //printf("max vaddr: %lu\n",max_vaddr);
    // 2097152 bytes per block in block_table
    //  150332 [0][150332]
    //   65536  [0][65536]
    

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
        /*
        for(int i = 0; i < memsz; i++){
            ((int8_t *)buf)[i]=0;
        }
        */
        
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

    size_t content_size = max_vaddr - min_vaddr;
    uint8_t *read_buf = (uint8_t *) malloc(content_size);
    if(read_from_bulk(block_table, min_vaddr, read_buf, content_size) != content_size){
        errx(EXIT_FAILURE, "ERROR: Number of bytes read dosent match number of bytes expected.");
        return;
    }

    printf("\nContent in memory:\n");
    for(size_t i=0;i<content_size;i++){
        printf("%02x ",read_buf[i]);
    }
    free(read_buf);
    
    // Release the ELF descriptor
    elf_end(elf);
    
    // fd close
    close(fd);   
}