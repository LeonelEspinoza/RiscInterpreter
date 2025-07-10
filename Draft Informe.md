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
El desarrollo de la solución se realizo implementando cada modulo de la solución, comprobando que este funcionara de forma esperada para después integrar sus funcionalidades al conjunto completo de la solución. El primer módulo implementado fue el más grande y la base de todo los demás, e interprete de instrucciones de rv32im.  Este interprete en un principio fue pensado para recibir instrucciones de un archivo de texto con instrucciones en formato hexadecimal, el cual era generado por otra aplicación que procesaba un binario compilado para Risc-v. Una vez este estuvo en funcionamiento, se probó utilizando códigos de prueba que las instrucciones funcionarán de la forma esperada.

Una vez el interprete estuvo completo, de decidió que debía ser capaz de procesar archivos binarios compilados para rv32im. Estos archivos específicamente en el formato de ELF, por sus siglas Executable Linked Format. Para esto fue necesario utilizar una librería capaz de manipular y leer estos archivos, la librería "libelf". Aprender a utilizar esta librería consumió una parte importante del desarrollo de la solución. Además, para poder implementar este modulo a la solución era necesario también implementar la abstracción de memoria que se utilizaría, puesto que se tenían que cargar a memoria partes del archivo a interpretar, y el interprete debía ser capaz de utilizar esta memoria también. 

La implementación de la abstracción de memoria fue sugerida por el profesor guía, utilizando un comportamiento similar al de paginamiento. Una vez obtenido el código que define esta estructura de memoria y sus funciones principales para utilizarse se comprobó su funcionamiento ejecutando tests que comprobaran los casos bordes y verificaran los resultados de todas las funciones dentro de la estructura.

Una vez obtenido un código capaz de obtener toda la información de los archivos ELFs necesaria para interpretar y cargar esta información a la abstracción de memoria, se modificó el interprete para que fuera capaz de recibir la información procesada de los archivos ELFs, e interpretar sin problemas. Una vez estuvo completa la integración de ambos módulos, se comprobó su funcionamiento utilizando un código que calcula el factorial de 10 utilizando recursión. Esto levantó varios errores en el interprete, lo que llevo a depurar estos errores hasta obtener una solución estable. Es en este punto donde la solución ya es un producto mínimo viable, pero no es el depurador que estamos esperando, es un interprete de binarios compilados para rv32im.

Tomando el estado del interprete, lo siguiente era la funcionalidad de procesar el input del usuario para poder realizar diferentes tareas de depurador, tal como: avanzar la ejecución en una instrucción, revisar los valores dentro de los registros y terminar la ejecución. Una vez fue implementado, la solución ya constituía un depurador de binarios compilados para rv32im con funcionalidades básicas.

Entonces el siguiente paso fue implementar las funcionalidades para retroceder en la ejecución del depurador. Esto significaba crear una estructura que guardara todos los valores necesarios para restaurar el estado del programa a uno anterior en la ejecución, y una estructura que guardara estas estructuras utilizando memoria de forma circular, permitiendo así que se pudieran agregar estos datos por cada instrucción ejecutada, eliminando aquellas más antiguas para escribir las nuevas. Para comprobar el funcionamiento de esta última parte se utilizo otro código simple en c que intenta acceder a una dirección de memoria no inicializada, produciendo un segmentation fault. Este nuevo compilado levanto nuevos errores en la solución, las cuales se intentaron resolver.
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
### Almacenamiento de Traza
#### Estructura Back-step
En rv32im no hay ninguna instrucción que modifique 2 o más valores por ciclo. Todas las instrucciones de Risc-V modifican como máximo 1 registro o dirección de memoria, y definen la dirección en memoria de la próxima instrucción a ejecutar. Gracias a esto se puede crear una estructura que almacena solo dos valores necesarios para restaurar el estado de la ejecución a justo antes de ejecutar la instrucción. Estos dos valores son: Valor antes de modificar del registro o dirección en memoria, y la dirección en memoria de la instrucción recién ejecutada. Llamemos a esta estructura: back-step.
#### Estructura Backlog
Backlog es una estructura creada para almacenar un arreglo de $N$ back-steps de forma circular que tiene los siguientes atributos:
+ **Steps:** es un arreglo tamaño $N+1$ de back-log. Aquí se almacenan todos los back-steps de la ejecución.
+ **Top:** es un valor entero que va entre $0$ a $N$. Este indica cuál es el primer índice dentro del arreglo 'steps' donde se puede escribir un nuevo back-step.
+ **Bot:** es un valor entero que va entre $0$ a $N$. Este indica cuál es el primer índice donde hay almacenado un back-step, es decir, el índice donde está almacenado el back-step más antiguo.

Es necesario Top y Bot para asegurar que el arreglo de steps se utilice de forma circular, dando la opción de que siempre se puedan agregar más steps a memoria, eliminando aquellos más antiguos. Esto se hace manteniendo la lógica de que siempre que se agregue un nuevo step, se avanza top al siguiente indice en el arreglo. Si al aumentar top es igual a bot, significa que el arreglo tiene $N$ back-steps almacenados y por tanto es necesario eliminar el back-step más antiguo. Así, se aumenta bot al siguiente indice, el cual queda apuntando al back-step que era el más antiguo.

Y en el caso de retroceder en la instrucciones, al obtener un back-step, primero se verifica que top no sea igual a bot, en ese caso, significa que no hay ninguna instrucción almacenada en el arreglo de steps. Pero si top es diferente de bot, siempre habrá al menos un back-step almacenado en el arreglo, por lo que se extrae el último back-step almacenado, y se mueve top a apuntar al índice con el back-step recién extraído, pues este índice pasa a ser el primer índice donde se puede escribir un nuevo back-step. 
#### Funcionalidad de Back-Step
Para devolver el estado de ejecución a justo antes de ejecutar una instrucción, solo son necesarios los valores almacenados en el back-step de la instrucción que se quiere retroceder. Entonces, al volver en un paso atrás en la ejecución, lo primero que se hace es pedirle a backlog que nos entregue el último back-step almacenado. Una vez obtenido el back-step, se obtiene la instrucción en la dirección de memoria almacenada en el back-step. Con esta instrucción se define una de tres opciones: 
+ No se modifico ningún valor, que puede ser una instrucción de branch. En este caso lo único que se hace para devolver el estado previo a la ejecución es indicar que la siguiente instrucción a ejecutar es la dirección de esta instrucción.
+ Se modifico una dirección de memoria, producto de una operación de store. En este caso, se obtiene la dirección de memoria modificada a través de la misma instrucción. Y dentro de esta se almacena el valor previo a ser modificado, el cuál fue guardado dentro del back-step cuando fue ejecutada previamente. Por último se indica que la dirección de la próxima instrucción a ejecutar es esta instrucción.
+ Se modifico un registro, que puede ser producto de cualquier otra instrucción. En este caso se obtiene el registro que fue modificado a través de la misma instrucción, y en este se almacena el valor previo a ser modificado, el cuál fue guardado dentro del back-step cuando fue ejecutada previamente. Por último se indica que la dirección de la próxima instrucción a ejecutar es esta instrucción.
Al terminar, el estado del programa es el mismo que al momento anterior de ejecutar la instrucción recién retrocedida. Por lo tanto, este proceso se puede repetir siempre y cuando haya al menos un back-step almacenado en el backlog.
# Cap 3: Evaluación de la Solución
## Método de Evaluación
## Resultados de la Evaluación
# Cap 4: Conclusiones y Trabajo a Futuro
## Trabajo a Futuro