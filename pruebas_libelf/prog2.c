/*
Print the names of ELF sections.

Este programa me dio muchos problemas debido a la libreria bsd/vis.h que no conseguí hacerla funcionar, 
pero con este pude entender las funciones que se usan para explorar las secciones en un ELF
*/

#include <err.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <gelf.h>

#include <stdint.h>
#include <bsd/vis.h>


int main (int argc, char **argv){
    int fd;
    Elf *e;
    Elf_Scn *scn;
    Elf_Data * data;
    GElf_Shdr shdr;
    size_t n, shstrndx, sz;
    char *name, *p, pc[(4 * sizeof(char)) + 1];
    
    if (argc != 2){
        errx(EXIT_FAILURE, "usage: %s file-name", argv[0]);
    }

    // Necessary to call other libelf library functions
    if (elf_version(EV_CURRENT) == EV_NONE){ 
        err(EXIT_FAILURE, "ELF library initialization failed: %s", elf_errmsg(-1));
    }
    
    // Open file
    if ((fd = open(argv[1], O_RDONLY, 0)) < 0){
        err(EXIT_FAILURE, "open \"%s\" failed", argv[1]);
    }

    // Convert the file descriptor to an ELF handle
    if((e = elf_begin(fd, ELF_C_READ, NULL)) == NULL){ //
        errx(EXIT_FAILURE, "elf_begin() failed: %s", elf_errmsg(-1)); //
    }

    // Check if file is ELF kind
    if(elf_kind(e) != ELF_K_ELF){
        errx(EXIT_FAILURE, "\"%s\" is not an ELF object.", argv[1]);
    }
    
    // Retrieve the index of the section name string table
    if (elf_getshdrstrndx(e, &shstrndx) != 0){
        errx(EXIT_FAILURE, "elf_getshdrstrndx() failed: %s.", elf_errmsg(-1));
    }

    scn = NULL;

    // Iterate through the sections
    while ((scn = elf_nextscn(e, scn)) != NULL){
        // Retrieve the section header
        if (gelf_getshdr(scn, &shdr) != &shdr){
            errx(EXIT_FAILURE, "getshdr() failed: %s.", elf_errmsg(-1));
        } 

        // Get the name of the section
        if ((name = elf_strptr(e, shstrndx, shdr.sh_name)) == NULL){
            errx(EXIT_FAILURE, "elf_strptr() failed: %s.", elf_errmsg(-1));
        }

        printf("Section %-4.4jd %s\n", (uintmax_t) elf_ndxscn(scn), name);
    }

    if ((scn = elf_getscn(e,shstrndx)) == NULL){
        errx( EXIT_FAILURE, "getscn() failed: %s.", elf_errmsg(-1));
    }

    if (gelf_getshdr(scn, &shdr) != &shdr){
        errx( EXIT_FAILURE, "getshdr() failed: %s.", elf_errmsg(-1));
    }

    printf(".shstrab: size=%jd\n", (uintmax_t) shdr.sh_size);

    data = NULL; n=0;
    while (n < shdr.sh_size && (data = elf_getdata(scn, data)) != NULL){
        p= (char *) data->d_buf;
        while (p < (char *) data->d_buf + data->d_size){
            if (vis(pc, *p, VIS_WHITE, 0)){
                printf("%s", pc);
            }
            n++; p++;
            putchar((n % 16) ? ' ' : '\n');
        }
    }
    putchar ('\n');

    elf_end(e);
    close(fd);
    exit(EXIT_SUCCESS);
}