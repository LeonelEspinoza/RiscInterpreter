#include <stdio.h>
#include <libelf.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

/*
prog4.c

Reads a binary file and prints in hexadecimal

Este principalmente lo utilice para confirmar que estuviera funcionando prog3.c
*/

int main(){
    // file pointer
    FILE * fptr;
    //open binary file with read binary
    fptr = fopen("out_libelf","rb");
    //Buffer to for reading
    unsigned char buf[32];

    // Read file and put it in buffer
    fread(buf, sizeof(buf), 1, fptr);

    // for every 2 elements of the array print them in little endian hexadecimal style
    for (int i = 0; i<32; i+=2){
        printf("%02x%02x ", buf[i+1], buf[i]);
    }
    printf("\n");

    fclose(fptr);
}

/*
buf[x] = XX
 _ _ _ _
16 8 4 2
*/