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
    
    // Retrieve the index of the section name string table
    elf_getshdrstrndx(elf, &shstrndx);

    // Iterate through the sections
    while ((scn = elf_nextscn(elf, scn)) != NULL) {
        // Retrieve the section header
        gelf_getshdr(scn, &shdr);
        name = elf_strptr(elf, shstrndx, shdr.sh_name);
        //printf("%s \n",name);
        
        // If section header type is symtab -> get _start address
        if (shdr.sh_type == SHT_SYMTAB) {
            data = elf_getdata(scn, NULL);
            int count = shdr.sh_size / shdr.sh_entsize;
            for(int i=0; i< count; i++){
                gelf_getsym(data, i, &sym);
                if(strcmp(elf_strptr(elf, shdr.sh_link, sym.st_name),"_start")){
                    continue;
                }
                Elf64_Addr start_addr = sym.st_value;
                printf("_start value: 0x%.8lx \n", start_addr);
                break;
            }
            break;
        }
        
        // Get the name of the section
        
        // Check if the section is .text
        if (strcmp(name, ".text") == 0) {
            printf("found .text section\n");
            //break;
        }
        
    }

    // Elf descriptor in .text section
    
    // Close the file descriptor of output file
    close(fd_out);

    // Release the ELF descriptor
    elf_end(elf);

    // Close main
    close(fd_in);   
}   
/*
// Open the binary output file
fd_out = open("out_libelf", O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR); //no sé porque me alega sobre S_IRUSR y S_IWUSR pero al compilar y ejecutar funciona bien

// Read the section and write it to the file
while ((data = elf_getdata(scn, data)) != NULL) {
    write(fd_out, data->d_buf, data->d_size);
}

*/