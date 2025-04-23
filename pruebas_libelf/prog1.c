/*
* Getting started with libelf
* 
* Este programa imprime el tipo de archivo entregado. Fue el primero con el que experimente para entender como inicializar la librería
*/

#include <err.h>
#include <fcntl.h>
#include <libelf.h> //1
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main (int argc, char **argv){
    int fd;
    Elf *e; //2
    char *k;
    Elf_Kind ek; //3

    if (argc != 2){
        errx(EXIT_FAILURE, "usage: %s file-name", argv[0]);
    }
    if (elf_version(EV_CURRENT) == EV_NONE){ 
        err(EXIT_FAILURE, "ELF library initialization failed: %s", elf_errmsg(-1));
    }
    /*
    Antes de que las funciones de la libreria sean invocadas, 
    una aplicacion debe indicar a la librería la version de ELF 
    que se espera que use. Esto es hecho llamando a elf_version.

    Llamar a elf_version es obligatorio antes de que otras funciones 
    en la ELF library

    
    */
    if ((fd = open(argv[1], O_RDONLY, 0)) < 0){
        err(EXIT_FAILURE, "open %s\" failed", argv[1]);
    }
    if((e = elf_begin(fd, ELF_C_READ, NULL)) == NULL){ //
        errx(EXIT_FAILURE, "elf_begin() failed: %s", elf_errmsg(-1)); //
    }

    ek = elf_kind(e); //

    switch (ek){
        case ELF_K_AR:
            k = "ar(1) archive";
            break;
        case ELF_K_ELF:
            k = "elf object";
            break;
        case ELF_K_NONE:
            k = "data";
        default:
            k= "unrecognized";
    }

    printf("%s: %s\n", argv[1],k);
    elf_end(e);  //
    close(fd);

    exit(EXIT_SUCCESS);
}
