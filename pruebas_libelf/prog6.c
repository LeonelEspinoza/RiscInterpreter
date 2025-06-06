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
prog6.c

Reads an ELF file and gets _start address and .text binary
*/

void main(int argc, char **argv) {
    
    // Initialize important variables

    Elf *elf = NULL;
    Elf_Scn *scn = NULL;
    Elf_Data *data = NULL;
    GElf_Shdr shdr;
    GElf_Sym sym;
    
    //Elf64_Addr start_addr;

    Elf32_Addr start_addr;
    // (char*) section header name 
    char *name = NULL;
    
    // (int) file descriptor input
    int fd_in = 0;

    // (int) file descriptor output
    int fd_out = 0;

    size_t shstrndx = 0, n = 0;

    // Check ELF file name argument is given
    if (argc != 2){
        errx(EXIT_FAILURE, "usage: %s ELF-file-name", argv[0]);
    }

    // Necessary to call other libelf library functions
    if (elf_version(EV_CURRENT) == EV_NONE){ 
        err(EXIT_FAILURE, "ELF library initialization failed: %s", elf_errmsg(-1));
    }

    // Open ELF file
    if ((fd_in = open(argv[1], O_RDONLY, 0)) < 0){
        err(EXIT_FAILURE, "open %s\" failed", argv[1]);
    }
    
    // Convert the file descriptor to an ELF handle
    if((elf = elf_begin(fd_in, ELF_C_READ, NULL))==NULL){
        errx(EXIT_FAILURE, "elf_begin() failed: %s", elf_errmsg(-1));
    }
    
    // Check if file is ELF kind
    if(elf_kind(elf) != ELF_K_ELF){
        errx(EXIT_FAILURE, "\"%s\" is not an ELF object.", argv[1]);
    }
    

    // Iterate through the sections
    while ((scn = elf_nextscn(elf, scn)) != NULL) {
        
        // Retrieve the section header
        gelf_getshdr(scn, &shdr);
        
        // If section header type is symtab -> get _start address
        if (shdr.sh_type == SHT_SYMTAB) {
            
            // Get data inside symbol section
            data = elf_getdata(scn, NULL);

            // Size of section / entry size of section = N entries in section
            int count = shdr.sh_size / shdr.sh_entsize;
            
            for(int i=0; i< count; i++){
        
                // Get symbol
                gelf_getsym(data, i, &sym);
            
                // search for _start symbol
                if(strcmp(elf_strptr(elf, shdr.sh_link, sym.st_name),"_start") == 0){
            
                    // Save start address for later
                    start_addr = sym.st_value;
                    printf("_start value: 0x%lx \n", start_addr);
            
                    //break for loop
                    break;
                }
            }
        
            // Break while loop
            break;
        }
    }

    // Reset scn
    scn = NULL;
    data=NULL;
    
    // Retrieve the index of the section name string table
    elf_getshdrstrndx(elf, &shstrndx);
    
    // Iterate through again 
    while ((scn = elf_nextscn(elf, scn)) != NULL) {
        
        // Retrieve the section header
        gelf_getshdr(scn, &shdr);

        // Get the name of the section
        name = elf_strptr(elf, shstrndx, shdr.sh_name);
        //printf("%s \n",name);
        
        // Check if the section is .text
        if (strcmp(name, ".text") == 0) {
            printf("found .text section\n");
            break;
        }
    }
    // Elf descriptor in .text section

    
    // Open the binary output file
    fd_out = open("prog6_out", O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);
    
    // Read the section and write it to the file
    while ((data = elf_getdata(scn, data)) != NULL) {
        write(fd_out, data->d_buf, data->d_size);
    }

    // Close the file descriptor of output file
    close(fd_out);
    
    int fd_bin = open("prog6_out", O_RDONLY, S_IRUSR);
    lseek(fd_bin,(unsigned int) start_addr, SEEK_SET);
    unsigned char buf[2];
    for(int i=0; i<8; i++){
        read(fd_bin, buf, 2);
        printf("0x%02x%02x ", buf[1], buf[0]);
    }
    printf("\n");
    close(fd_bin);
    
    // Release the ELF descriptor
    elf_end(elf);
    
    // Close main
    close(fd_in);   
}