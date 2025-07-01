# README carpeta de Pruebas Libelf

La carpeta 'Pruebas_libelf' contiene los principales programas que he estado utilizando para aprender y utilizar la librería libelf para interpretar archivos ELF en el interprete de RiscV

### Códigos
``fact10.c``  
    Código simple que calcula el factorial de 10 de forma recursiva e imprime el resultado.

``pag2.c``  
    Estructura y lógica de funcionamiento de la memoria del debuger. 

``elf_interpreter.c``  
    Intéprete de ejecutables ELFs compilados para RISC-V, con funcionalidades básicas de un debuger.

``backlog.c``  
    Estructura y lógica para volver en la ejecución.

### Compilar
Para facilitar la compilación esta el make con las siguientes operaciones:

    > make file_name.gcc
Compila el archivo ``file_name.c`` usando gcc y la librería libelf. El output queda en el archivo ``file_name.gcc``  

Para compilar ``prog3.c``, ``prog4.c``, ``prog5.c``

    > make file_name.qemu-O
Compila para RiscV.  
Este está hecho para compilar el fact10.c para RiscV

    > make file_name.readelf-qemu
Genera un archivo con los datos del archivo ``file_name``. Aquí se encuentran todas sus secciones y symbolos ó etiquetas, como .text o _start.

## Notas
