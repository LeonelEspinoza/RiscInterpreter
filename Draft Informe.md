# Cap 1: Introducción
¿Qué? ¿Por qué? Define lo que el lector puede esperar y justifica la existencia.
Contexto, Justificación, objetivos, alcances, método, estructura
Temas importantes a la memoria:
+ Por qué realizar este trabajo
+ Qué es un ISA
+ Qué es Risc-V
+ Qué es un ELF
+ Qué es un depurador
+ Qué es un depurador inverso
+ Uso de depuradores inversos
+ Depuradores inversos existentes
 
En cursos de Arquitectura de Computadores y Sistemas Operativos, parte del curriculum de estos es trabajar con un ISA, instruction set architecture, para entender a bajo nivel el funcionamiento de máquinas y PCs. En particular se utiliza la arquitectura Risc-V, una ISA de código abierta enfocada en un set de instrucciones reducido y eficiente, ideal para propósitos didácticos, por lo que se utiliza dentro de ramos de arquitectura de computadores y sistemas operativos. Trabajar con esta arquitectura de bajo nivel es un reto en algunos casos, pues no es familiar y se diferencia con otros lenguajes de programación en que no está pensada para la comodidad del desarrollador que lo utiliza, por tanto el tener una herramienta que facilite el desarrollo de código y permita entender la ejecución de el mismo sería de gran utilidad pera les estudiantes, apoyando su aprendizaje y solventando algunos problemas. Un depurador es una herramienta para desarrolladores que permite entender la ejecución de un código y encontrar errores dentro de un ambiente seguro. 
## Problema a Solucionar
% Qué problemática se busca solucionar con el trabajo realizado
El problema que intenta solucionar este trabajo esta relacionado a las dificultades que se presentan al utilizar rv32im para desarrollar código. Además de apoyar a les estudiantes de cursos de Sistemas Operativos y Arquitectura de Computadores DCC.
## Soluciones Existentes
% Ejemplos de soluciones existentes (depuradores y depuradores inversos)

## Objetivos de la Memoria
% Objetivos que se quieren cumplir al realizar el trabajo
### Objetivo General
Desarrollar un depurador de programas compilados para Risc-V, capaz de depurar errores guardado una traza de las modificaciones hechas en registros y memoria de forma eficiente y extensible. 
### Objetivos Específicos
+  Crear un depurador clásico para programas compilados en assembler Risc-V
+  Implementar funcionalidades de avanzar y retroceder por instrucciones de máquina.
+  Implementar funciones de avanzar y retroceder por llamada de función.
+  Implementar interfaz que presente el contenido de memoria y registros.
+  Implementar funcionalidad de realizar búsquedas hasta modificar una dirección de memoria hacia adelante y hacia atrás.

## Resumen de la Solución
% Resumen de la solución obtenida
La solución obtenida es básicamente un interprete que recibe un input de usuario para:
+ Ejecutar una instrucción y mostrar en pantalla los valores utilizados
+ Mostrar los valores de los 32 registros de RIsc-V
+ Saltar un bloque de instrucciones
+ 
## Estructura del Documento
% Quizás no necesito esta sección
# Cap 2: Desarrollo de la Solución
## Requisitos
Qué tiene que cumplir este depurador?
+ Debe ser abierto
+ Debe ser extensible
+ Debe soportar archivos ELFs compilados para rv32im
+ Debe ser capaz de avanzar y retroceder en la ejecución instrucción por instrucción y por llamadas de función
+ Debe soportar que un programa tenga errores
## Desarrollo
### Metodología de Desarrollo
Realmente se fue tacleando los problemas a medida que surgen.
1. Se partió desarrollando el interprete de instrucciones rv32im, utilizando switch de C para matchear cada caso de instrucciones adecuadamente según la documentación de rv32im y los registros necesarios para su funcionamiento
2. Después paso a ser necesario programar el soporte a archivos ELFs, lo que llevo muchisimo más tiempo del esperado.
3. A la vez, una parte necesaria para dar soporte a los archivos ELFs era tener una estructura de memoria capaz de cargar las partes necesarias del archivo a ejecutar y manejar memoria durante la ejecución.
4. Con estas tres partes listas y el depurador en sus primeros pasos es necesario trabajar en las funciones de step y las funciones que permitan al usuario ver los registros
5. Ya listo fue necesario trabajar en el back-step. Para esto es necesario crear una estructura capaz de guardar en memoria de forma circular una traza. además de dar soporte a 
### Intérprete de instrucciones rv32im
El intérprete es de las partes más grandes de la memoria. Una vez todas la información importante del archivo ELF a depurar está cargada en memoria, comienza la interpretación del mismo. 

La primera instrucción a ejecutar es aquella en la dirección especificada por el símbolo '\_start', a partir de ahí se avanza la ejecución leyendo de a 4 bytes o 32 bits en la abstracción de memoria.

Para interpretar una instrucción primero se obtiene de la memoria 4 bytes de información, los cuales contienen toda la información necesaria para ejecutar. 
Después esta instrucción se divide en todas las partes que la componen: 
+ opcode
+ rd
+ rs1
+ rs2
+ func3
+ imm
+ uimm
Dependiendo del código de operación algunas secciones se usan y otras no.
Las instrucciones posibles son:
+ LOAD UPPER IMMEDIATE
+ ADD UPPER IMMEDIATE PROGRAM COUNTER
+ JUMP AND LINK
+ JUMP AND LINK RETURN
+ BRANCH EQUAL
+ BRANCH NOT EQUAL
+ BRANCH LOWER THAN
+ BRANCH GREATER OR EQUAL
+ BRANCH LOWER THAN UNSIGNED
+ BRANCH GREATER EQUAL UNSIGNED
+ LOAD BYTE
+ LOAD HALF WORD
+ LOAD WORD
+ LOAD BYTE UNSIGNED
+ LOAD HALF WORD UNSIGNED
+ STORE BYTE
+ STORE HALF WORD
+ STORE WORD
+ OPERATION IMMEDIATE 
+ OPERATION
Utilizando el código de operación dentro de la instrucción se matchea con cada caso necesario.

Una vez reconocido el código de operación se ejecuta de acuerdo a la documentación, con algunas observaciones adicionales, como asegurarse de que el registro 0 siempre se mantenga como 0, y mostrarle al usuario que instrucción se ejecuta en conjunto con valores importantes de la instrucción.
### Manejo de archivos ELFs
Para hacer que la solución sea lo más útil posible, se decidió que el depurador debía poder procesar archivos ELFs compilados para una arquitectura rv32im. 

Para lograr esto fue necesario estudiar como se componen estos archivos y las partes necesarias para la ejecución, intentando minimizar la cantidad de información.

Las partes que se cargan a memoria son los segmentos marcados con la flag LOAD. Y se cargan a memoria manteniendo las mismas direcciones virtuales, evitando re-localizar direcciones dentro del intérprete. Lo principal a tener en cuenta de este proceso es que es aquí donde todas las instrucciones se cargan a la estructura de memoria del depurador, quedando en las misma distribución en la que están.

Además algo necesario para comenzar la ejecución es la dirección de la primera instrucción a ejecutar, la cuál esta indicada con el símbolo "\_start", en la sección de símbolos dentro del ELF.

Una vez cargados en memoria todos los segmentos y secciones necesarias para la ejecución, y se tiene la dirección de la primera instrucción a ejecutar, se puede comenzar con la interpretación del binario.
### Estructura de Memoria
La estructura de memoria implementada esta pensada utilizando paginamiento con bloques de tamaño 2MB (2²¹ bytes), y 2¹¹ entradas en la tabla de bloques, dejando así 2³² posibles direcciones de memoria. Además esta implementacion permite que solo se inicialicen los bloques de memoria necesarios para la ejecución. Hay 2¹¹ posibles entradas en la tabla, pero solo aquellas que son utilizadas e inicializadas ocupan los 2MB de memoria. 

Con el fin de optimizar el acceso a la memoria y al uso de espacio de la misma, se implemento una estructura de memoria que utiliza paginamiento con paginas ó bloques de 2MB y una tabla con 2048 entradas para bloques. De esta forma, la tabla tiene $2^{11}$ entradas para bloques de 2MB ó $2^{21}$ bytes, dejando un total $2^{32}$ posibles direcciones, las cuales son todas las posibles direcciones a las que se puede acceder en un sistema de 32 bits. Así también se aprovecha de tomar los primeros $2^{11}$ bits de una dirección para definir el bloque de memoria, y los otros $2^{21}$ bits definen el offset dentro del bloque. 
La optimización de memoria se da debido a que solo se inicializan los bloques de memoria en los que se escribe información. Es decir, la memoria utilizada crece de a 2MB de memoria por cada vez que se escriba en una dirección dentro de un bloque que no esté inicializado.
### Funcionalidad Step
La funcionalidad de step se refiere a que mientras se está depurando el programa o interpretando, el usuario puede ingresar un input indicando si quiere avanzar una instrucción hacia adelante en la ejecución. Además de esto el usuario tiene la opción de ingresar un input para:
+ Ver los valores de los registros en el punto actual de la ejecución
+ Pedir ver más detalles de la instrucción ejecutada
+ Saltar varios instrucciones hacia adelante
+ Terminar el programa
### Estructura de Almacenamiento de Traza
En rv32im no hay ninguna instrucción que modifique 2 valores por ciclo. Todas las instrucciones de Risc-V modifican como máximo 1 registro, o dirección de memoria, y definen la dirección en memoria de la próxima instrucción a ejecutar. Gracias a esto se puede crear una estructura que almacena solo dos valores necesarios para restaurar el estado de la ejecución a justo antes de ejecutar la instrucción. Estos dos valores son: Valor antes de modificar del registro ó dirección en memoria, y la dirección de la instrucción recién ejecutada. Una de estas estructuras es suficiente para volver atrás un paso en la ejecución. Y para almacenar una traza de las instrucciones utilizando la memoria de forma circular, se crea otra estructura que contiene: un arreglo de tamaño N  
### Funcionalidad de Back-Step
# Cap 3: Evaluación de la Solución
## Método de Evaluación
## Resultados de la Evaluación
# Cap 4: Conclusiones y Trabajo a Futuro
## Trabajo a Futuro