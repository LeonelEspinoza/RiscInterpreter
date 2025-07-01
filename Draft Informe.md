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
### Manejo de archivos ELFs
### Estructura de Memoria
### Funcionalidad Step
### Estructura de Almacenamiento de Traza
### Funcionalidad de Back-Step
# Cap 3: Evaluación de la Solución
## Método de Evaluación
## Resultados de la Evaluación
# Cap 4: Conclusiones y Trabajo a Futuro
## Trabajo a Futuro