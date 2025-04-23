# README carpeta de Pruebas Libelf

La carpeta 'Pruebas_libelf' contiene los principales programas que he estado utilizando para aprender y utilizar la librería libelf para interpretar archivos ELF en el interprete de RiscV

### Codigos
``fact10.c``  
    codigo simple que calcula el factorial de 10 y lo imprime.

``libelf_test.c``  
    Es un codigo que utilicé para probar que la librería estuviera bien instalada

``prog1.c``  
    Imprime el tipo de archivo entregado

``prog2.c``  
    Este no lo consegui compilar debido a una libreria que me daba problemas. 
    Debería iterar por las secciones de un ELF e imprimirlas

``prog3.c``  
    Este extrae la sección .text del archivo 'fact10.qemu-O' (esta hardcodeado)  y la deja en 'out_libelf'

``prog4.c``  
    Imprime las primeras operaciones dentro de lib elf
    Lo utilice para ver que efectivamente estuviera funcionando prog3.c

``prog5.c``   
    Busca a través de los simbolos o etiquetas de un ELF e imprime el valor, tamaño y nombre. Está editado para que solo imprima la etiqueta '_start'  
Se compila utilizando:  

     make prog5.gcc

se utiliza tal que:

     ./prog5.gcc fact10.qemu-O

y debería retornar:

    > Valor 0x00... Tamaño 0x00... _start

El valor siendo la dirección almacenada de _start.  
Encontré esta documentación sobre las tablas de simbolos de libelf como referencia: https://refspecs.linuxbase.org/elf/gabi4+/ch4.symtab.html

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
Durante la reunión de avances conversamos sobre la diferencia entre la dirección de _start obtenida utilizando readelf en la linea de comandos

    make fact10.readelf-qemu

que genera un archivo con todos los datos de fact10.qemu-O, entre los que está el simbolo _start.  

Y el resultado obtenido utilizando la libreria libelf ejecutando 

    ./prog5.gcc fact10.qemu-O