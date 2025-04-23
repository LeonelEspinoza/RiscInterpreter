#include <err.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <gelf.h>

#include <libelf.h>
#include <string.h>

/*
prog3.c

Reads an ELF binary file and extract the .text section to a new file called "out_libelf"
*/

void main() {
    // Initialize important variables
    Elf *elf = NULL;
    Elf_Scn *scn = NULL;
    Elf_Data *data = NULL;
    GElf_Shdr shdr;
    // Elf64_Shdr *shdr = NULL;
    char *name = NULL;
    int fd_in = 0, fd_out = 0;
    size_t shstrndx = 0, n = 0;

    // Necessary to call other libelf library functions
    if (elf_version(EV_CURRENT) == EV_NONE){ 
        err(EXIT_FAILURE, "ELF library initialization failed: %s", elf_errmsg(-1));
    }
    
    // Open main
    if ((fd_in = open("fact10.qemu-O", O_RDONLY, 0)) < 0){
        err(EXIT_FAILURE, "open \"fact10.qemu-O\" failed");
    }

    // Convert the file descriptor to an ELF handle
    elf = elf_begin(fd_in, ELF_C_READ, NULL);

    // Check if file is ELF kind
    if(elf_kind(elf) != ELF_K_ELF){
        errx(EXIT_FAILURE, "\"%s\" is not an ELF object.", "fact10");
    }
    
    // Retrieve the index of the section name string table
    elf_getshdrstrndx(elf, &shstrndx);

    // Iterate through the sections
    while ((scn = elf_nextscn(elf, scn)) != NULL) {
        // Retrieve the section header
        gelf_getshdr(scn, &shdr);

        // Get the name of the section
        name = elf_strptr(elf, shstrndx, shdr.sh_name);

        // Check if the section is .text
        if (strcmp(name, ".text") == 0) {
            // Open the binary output file
            fd_out = open("out_libelf", O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR); //no sé porque me alega sobre S_IRUSR y S_IWUSR pero al compilar y ejecutar funciona bien

            // Read the section and write it to the file
            while ((data = elf_getdata(scn, data)) != NULL) {
                write(fd_out, data->d_buf, data->d_size);
            }

            // Close the file descriptor of output file
            close(fd_out);
        }
    }
    // Release the ELF descriptor
    elf_end(elf);
    // Close main
    close(fd_in);   
}   
