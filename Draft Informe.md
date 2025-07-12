# Cap 1: Introducción
¿Qué? ¿Por qué? Define lo que el lector puede esperar y justifica la existencia.
Contexto, Justificación, objetivos, alcances, método, estructura
Temas importantes a la memoria:
+ Por qué realizar este trabajo
+ Qué es un depurador
+ Qué es un depurador inverso
+ Uso de depuradores inversos
+ Depuradores inversos existentes
+ Qué es un Instruction Set Architecture
+ Qué es Risc-V
+ Qué es un ELF
 
En cursos de Arquitectura de Computadores y Sistemas Operativos, parte del curriculum de estos es trabajar con un ISA, instruction set architecture, para entender a bajo nivel el funcionamiento de máquinas y PCs. En particular se utiliza la arquitectura Risc-V, una ISA de código abierta enfocada en un set de instrucciones reducido y eficiente, ideal para propósitos didácticos, por lo que se utiliza dentro de ramos de arquitectura de computadores y sistemas operativos. Trabajar con esta arquitectura de bajo nivel es un reto en algunos casos, pues no es familiar y se diferencia con otros lenguajes de programación en que no está pensada para la comodidad del desarrollador que lo utiliza, por tanto el tener una herramienta que facilite el desarrollo de código y permita entender la ejecución de el mismo sería de gran utilidad pera les estudiantes, apoyando su aprendizaje y solventando algunos problemas. Un depurador es una herramienta para desarrolladores que permite entender la ejecución de un código y encontrar errores dentro de un ambiente seguro. 

Un depurador es una herramienta diseñada para facilitarles a los programadores la labor de encontrar y entender bugs que se presentan en tiempo de ejecución de programas y códigos. Esto mediante ejecutar el código a depurar en un ambiente controlado, lo más parecido a un ambiente real de ejecución, pero con la capacidad de manipular el flujo de ejecución del programa, de analizar los valores de variables en los diferentes puntos de la ejecución, y otras funcionalidades útiles.

Un depurador inverso es un tipo de depurador, que además de cumplir con las funcionalidades normales, también permite retroceder en la ejecución de un código.
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
La solución obtenida presenta un depurador con funcionalidades básicas tales como avanzar en la ejecución, mostrar valores de los registros en el punto actual de la ejecución, retroceder en la ejecución un máximo de N instrucciones, y mostrar más información sobre las instrucciones ejecutadas.


La solución obtenida es básicamente un interprete que recibe un input de usuario para:
+ Ejecutar una instrucción y mostrar en pantalla los valores utilizados
+ Mostrar los valores de los 32 registros de RIsc-V
+ Saltar un bloque de instrucciones
+ 
## Estructura del Documento
% Quizás no necesito esta sección

---
# Cap 2: Desarrollo de la Solución
## Requisitos
Para que el producto de esta memoria cumpla con las expectativas esperadas, se presentan los siguientes requisitos que debe cumplir.
#### Debe ser abierto
El depurador debe ser un proyecto de carácter abierto, permitiendo así continuar su desarrollo aún sin el autor, recibiendo modificaciones de desarrolladores contribuyentes.
#### Debe soportar archivos binarios ELFs compilados para rv32im
El depurador debe ser capaz de procesar archivos ejecutables binarios en formato ELF compilados para utilizar con una arquitectura rv32im. Utilizando este formato estandarizado para distribuciones de Linux, facilita a los usuarios el uso del depurador.
#### Debe ser capaz de avanzar y retroceder en la ejecución de un programa
Esta es una de las funcionalidades básicas esperadas de un depurador inverso. Ejecutar hacia adelante permite al usuario verificar el correcto funcionamiento del código. En cambio, ejecutar hacia atrás permite al usuario entender y identificar errores y su origen en el flujo de la ejecución.
#### Debe ser capaz de retroceder independiente del largo de la ejecución
Es necesario que el depurador pueda retroceder en la ejecución, aún cuando la ejecución del programa tome muchas instrucciones en completarse. El objetivo es que al momento de encontrar un error dentro de la ejecución del programa, el depurador permita al usuario retroceder para encontrar el origen de este error, independiente de en qué momento de la ejecución se encuentre el error.
#### Debe soportar un programa con errores en tiempo de ejecución
Como depurador, debe ser capaz de procesar y ejecutar un programa que presente errores al momento de ejecutarse, permitiendo así que el usuario identifique los errores y encuentre una solución.
## Desarrollo
### Metodología de Desarrollo
El desarrollo de la solución se realizo implementando cada modulo de la solución, comprobando que este funcionara de forma esperada para después integrar sus funcionalidades al conjunto completo de la solución. El primer módulo implementado fue el más grande y la base de todo los demás, e interprete de instrucciones de rv32im.  Este interprete en un principio fue pensado para recibir instrucciones de un archivo de texto con instrucciones en formato hexadecimal, el cual era generado por otra aplicación que procesaba un binario compilado para Risc-v. Una vez este estuvo en funcionamiento, se probó utilizando códigos de prueba que las instrucciones funcionarán de la forma esperada.

Una vez el interprete estuvo completo, de decidió que debía ser capaz de procesar archivos binarios compilados para rv32im. Estos archivos específicamente en el formato de ELF, por sus siglas Executable Linked Format. Para esto fue necesario utilizar una librería capaz de manipular y leer estos archivos, la librería "libelf". Aprender a utilizar esta librería consumió una parte importante del desarrollo de la solución. Además, para poder implementar este modulo a la solución era necesario también implementar la abstracción de memoria que se utilizaría, puesto que se tenían que cargar a memoria partes del archivo a interpretar, y el interprete debía ser capaz de utilizar esta memoria también. 

La implementación de la abstracción de memoria fue sugerida por el profesor guía, utilizando un comportamiento similar al de paginamiento. Una vez obtenido el código que define esta estructura de memoria y sus funciones principales para utilizarse se comprobó su funcionamiento ejecutando tests que comprobaran los casos bordes y verificaran los resultados de todas las funciones dentro de la estructura.

Una vez obtenido un código capaz de obtener toda la información de los archivos ELFs necesaria para interpretar y cargar esta información a la abstracción de memoria, se modificó el interprete para que fuera capaz de recibir la información procesada de los archivos ELFs, e interpretar sin problemas. Una vez estuvo completa la integración de ambos módulos, se comprobó su funcionamiento utilizando un código que calcula el factorial de 10 utilizando recursión. Esto levantó varios errores en el interprete, lo que llevo a depurar estos errores hasta obtener una solución estable. Es en este punto donde la solución ya es un producto mínimo viable, pero no es el depurador que estamos esperando, es un interprete de binarios compilados para rv32im.

Tomando el estado del interprete, lo siguiente era la funcionalidad de procesar el input del usuario para poder realizar diferentes tareas de depurador, tal como: avanzar la ejecución en una instrucción, revisar los valores dentro de los registros y terminar la ejecución. Una vez fue implementado, la solución ya constituía un depurador de binarios compilados para rv32im con funcionalidades básicas.

Entonces el siguiente paso fue implementar las funcionalidades para retroceder en la ejecución del depurador. Esto significaba crear una estructura que guardara todos los valores necesarios para restaurar el estado del programa a uno anterior en la ejecución, y una estructura que guardara estas estructuras utilizando memoria de forma circular, permitiendo así que se pudieran agregar estos datos por cada instrucción ejecutada, eliminando aquellas más antiguas para escribir las nuevas. Para comprobar el funcionamiento de esta última parte se utilizo otro código simple en c que intenta acceder a una dirección de memoria no inicializada, produciendo un segmentation fault. Este nuevo compilado levanto nuevos errores en la solución, las cuales se intentaron resolver.
### Intérprete de instrucciones rv32im
El intérprete es una funcionalidad esencial en el funcionamiento del depurador. Una vez todas la información importante del archivo ELF a depurar está cargada en memoria, comienza la interpretación del mismo. 

La primera instrucción a ejecutar es aquella en la dirección especificada por el símbolo '\_start', a partir de ahí se avanza la ejecución leyendo de a 4 bytes o 32 bits en la abstracción de memoria.

Una instrucción de rv32im esta representada por 32 bits seccionado para componer las diferentes partes de la instrucción. Los 7 bits menos significativos de la instrucción representan el código de operación. Este indica que operación debe ejecutarse, en este caso por el interprete. Hay 47 posibles operaciones, entre las que están: Jump And Link, Multiply, Addition, Addition Immediate, Branch Lower Than, Load Byte, Store Word, Load Upper Immediate, etc. (Ver documentación instrucciones de RISC-V) Cada una de estas operaciones tiene asociado un código de 7 bits, y con esto comparando los 7 bits de la instrucción se puede obtener la operación específica de una instrucción.

Aparte de el código de operación, dentro de la instrucción hay secciones que tienen diferentes largos y propósitos según la operación a ejecutar. Por ejemplo está el registro de destino (rd) representado por los bits 19 a 15 de la instrucción. Este se utiliza en la gran mayoría de operaciones, e indica en qué registro almacenar cierta información, como lo puede ser con Load Word, que carga al registro destino el valor en una dirección de memoria específica. En cambio, operaciones tal como Branch o Store no se utiliza el registro de destino y los bits designados al registro de destino se utilizan con otro propósito. Otros valores importantes representados por diferentes grupos de bits en la instrucción son:
+ Func-3: usado para identificar una operación dentro de una familia de operaciones, como lo puede ser Load Half Word dentro de la familia de operaciones de Load. Este es representado por los bits 14 a 12 de la instrucción.
+ Registro fuente 1 y 2: usados para identificar dos registros que utilizar en una operación, por ejemplo al comparar dos registros en un Branch Greater or Equal. Estos son representados por los bits 24 a 20 para el registro fuente 2, y 19 a 15 para el registro fuente 1.
+ Valor inmediato: usado para representar un número constante y realizar una operación inmediata, que se utiliza principalmente en todas las operaciones inmediatas de aritmética o lógica, aunque también se utiliza como offset para obtener una dirección de memoria específica. Este tiene la particularidad de que no siempre es representado por los mismos bits. El conjunto de bits y orden de los mismos es dependiente de la operación que lo utiliza, como por ejemplo: Load Upper Immediate lo representa con los 20 bits más importantes de la instrucción, mientras que Jump And Link utiliza una reorganización de los bits 31 al 12 de la instrucción para representar el valor inmediato.

Una vez identificados los valores necesarios dentro de los 32 bits de una instrucción, es necesario ejecutar la operación correspondiente utilizando los valores especificados por la instrucción. Para esto se hace uso de la instrucción 'Switch-Case' de C, la cual determina cuál de las 47 es la operación representada por el código de operación obtenido en la instrucción. Y después se ejecuta esta operación utilizando los valores correspondientes, también obtenidos de los 32 bits de la instrucción. Es importante notar que hay dos operaciones que no tienen implementación en la solución actual: ECALL, EBREAK, y FENCE. 

FENCE es una operación utilizada para ordenar lectura y escritura en memoria, asegurando que todas las operaciones antes de esta sean ejecutadas antes de cualquier operación después de esta. ECALL es utilizada para realizar llamadas a sistema. Y EBREAK es utilizada para gatillar un breakpoint en la ejecución y entrar en modo debug.

Cada operación esta implementada de acuerdo a la documentación de Risc-V, con algunas modificaciones para adecuarlas al entorno en el que se está trabajando.Por ejemplo, hay que notar que también es necesario mantener una particularidad de los registros en Risc-V. En específico, el registro 0 de Risc-V se debe mantener con valor 0. Para eso es necesario que cada vez que se quiera asignar un valor a un registro, verificar que el registro a modificar no sea el registro 0, y si lo es, descartar el valor a asignar. Esto es implementado con una función que realiza la verificación y modifica los valores siempre que corresponda, pero además retorna el valor previo a modificar. Esta última funcionalidad se utiliza para almacenar la traza de ejecución. (ver Almacenamiento de Traza)

Además, al ejecutar cada instrucción, se le muestra al usuario que operación se ejecutó, y los valores utilizados para ejecutarla, tal como constantes o registros utilizados. Un ejemplo de ejecutar la operación inmediata de conjunción aritmética ("and") en bits:

>"ANDI imm: 15 rs1: 14 rd: 15"

En el ejemplo se puede ver representado:
+ "ANDI": operación realizada. Conjunción aritmética de bits utilizando constante o valor inmediato.
+ "imm: 15": valor de la constante utilizada.
+ "rs1: 14": registro fuente 1 utilizado.
+ "rd: 15": registro destino donde se guarda el resultado.

Al terminar la ejecución de una instrucción, la siguiente instrucción a ejecutar es aquella en la dirección de la estructura de memoria definida por la variable 'next_pc'. Como cada instrucción es representada con 32 bits, o 4 bytes, 'next_pc' de forma general es la dirección en memoria de la instrucción actual más 4. En el caso de realizar un salto o ramificación en la ejecución del código, se modifica la variable 'next_pc' para que apunte a la instrucción en la dirección del salto.

También es necesario verificar que el valor de 'next_pc' jamás apunte a una dirección en memoria no inicializada o un valor que no represente una instrucción ejecutable. Para esto se utilizan los valores de 'max_vaddr' o máxima dirección virtual, y 'min_vaddr' o mínima dirección virtual. Al finalizar cada ciclo se verifica que 'next_pc' esté dentro del rango entre el máximo y mínimo valor de direcciones virtuales.
### Manejo de archivos ELFs
El depurador debe ser capaz de depurar archivos binarios compilados para utilizar la arquitectura rv32im, y en Linux el formato binario por defecto es ELF, Executable and Linkable Format. Por lo que, antes de comenzar con la interpretación es necesario procesar la información presente en el binario ELF, asegurando que el archivo cumpla ciertos requisitos como: ser de formato ELF, ser ejecutable, contener información estándar importante como la dirección "\_start", cumplir con tamaños y formato esperados, entre otros. 

Para lograr esto fue necesario estudiar el formato ELF y encontrar las partes necesarias para la ejecución, intentando minimizar la cantidad de información que se tiene que cargar a memoria. Además, dentro del código fue necesario hacer uso de la librería "libelf", la cual brinda herramientas y funciones útiles para manipular y leer este formato, facilitando el desarrollar el procesador necesario para la solución.

Dentro del formato ELF, están los segmentos y las secciones. Ambas son organizaciones lógicas dentro del formato, pero se diferencian en que las secciones son utilizadas por el linker, y los segmentos son utilizados en tiempo de ejecución. La solución desarrollada carga a memoria todos los segmentos marcados con "PT_LOAD", ya que son estos segmentos los que se utilizan para la ejecución. Al cargar estos segmentos, se tiene el cuidado de mantener las mismas direcciones virtuales dentro de la estructura de memoria donde se carga la información, simplificando el uso de memoria en el resto del desarrollo y dejando la información en la misma distribución en la que se encuentran.

Agregado a cargar los segmentos a memoria, se obtienen cuáles son los valores mínimos y máximos de las direcciones virtuales de los segmentos cargados en memoria. Estos valores se utilizan en el interprete para asegurar que siempre se ejecuten direcciones de memoria que contengan instrucciones ejecutables.

Por último es necesario obtener la dirección virtual de la primera instrucción que se debe ejecutar. Esta dirección esta enlazada al símbolo "\_start" dentro de la sección ".symbol", la cuál se obtiene recorriendo los valores dentro de la sección hasta encontrarla y se guarda para utilizarla al comenzar la interpretación del archivo. 

Con los segmentos cargados en memoria, el valor de las direcciones virtuales máximas y mínimas, y la dirección de la primera instrucción a ejecutar, entonces se puede comenzar con la interpretación y otras funciones del depurador.
### Estructura de Memoria
El depurador necesita de una estructura de memoria que guarde la información que se debe cargar para procesar los binarios ELFs, y que permita implementar las funcionalidades de las operaciones LOAD y STORE de Risc-V. Entonces se implementó una estructura de memoria adecuada a las necesidades de la solución.

Con el fin de optimizar los accesos y el uso de espacio, se implementó una estructura de memoria que utiliza paginamiento con bloques de 2MB y una tabla con 2048 entradas para bloques. De esta forma, la tabla tiene $2^{11}$ entradas para bloques de 2MB ó $2^{21}$ bytes, dejando un total $2^{32}$ posibles direcciones, las cuales son todas las posibles direcciones a las que se puede acceder en un sistema de 32 bits. 

La optimización de los accesos a la memoria se realiza aprovechando que la cantidad de entradas en la tabla y el tamaño de los bloques son ambas potencias de 2. Una dirección tiene 32 bits, de estos 32 bits se utilizan los primeros 11 bits para definir la entrada en la tabla de bloques. Los 21 bits restantes de la dirección se utilizan para definir el offset desde el inicio del bloque. 

La optimización de memoria se da debido a que solo se inicializan los bloques de memoria en los que se escribe información. Es decir, la memoria parte con todas sus entradas sin inicializar, y al momento de escribir en una dirección dentro de una de estas entradas sin inicializar, se le asignan 2 MB de espacio a la entrada. Así, el tamaño utilizado por la estructura de memoria crece en bloques de 2MB cada vez que se escribe en una dirección dentro de un bloque sin inicializar.
### Funcionalidad Step y Ver Registros
Para que la solución cumpla las funcionalidades de un depurador, es necesario que sea capaz de permitirle al usuario avanzar en la ejecución de a una o más ejecuciones, mostrando información relevante relacionada a la ejecución, y permitirle al usuario verificar los valores de las variables dentro del programa. En el caso de Risc-V, las variables son 32 registros, los cuales según la documentación cumplen ciertos roles en específico. 

Entonces, antes de que el interprete inicie su ejecución, el programa le presenta el siguiente texto al usuario:

> "Input: break(B) next-step(n) back-step(b) next-breakpoint(N) View-Registers-Values(v) Toggle-instruction-details(d)"

Las cuales hacen referencia a las siguientes opciones:
+ Terminar el programa.
+ Avanzar la ejecución en una instrucción.
+ Retroceder la ejecución en una instrucción.
+ Avanzar la ejecución en varias instrucciones.
+ Ver los valores de los registros en el punto actual de la ejecución.
+ Configurar el interprete para mostrar más información por instrucción.

Una vez el usuario ingresa su input, se verifica que este haya sido válido, y se prosigue la ejecución de acuerdo al comando ingresado por el usuario.

En el caso de terminar el programa, se realizan las limpiezas de memoria y operaciones necesarias de limpieza y se finaliza. Avanzar una instrucción en la ejecución ejecuta el interprete con la siguiente instrucción correspondiente. Retroceder una instrucción utiliza el almacenamiento de Traza y la funcionalidad de back-step para restaurar el estado de la ejecución hasta $N$ instrucciones anteriores. Avanzar varias instrucciones, ejecuta 100 instrucciones. Ver los valores de los registros muestra al usuario una tabla de $2\times16$ con los valores de los registros correspondientes. Y por último, mostrar más información por instrucción añade información sobre dirección en memoria de la instrucción, representación numérica de la instrucción y código de operación de la instrucción
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

---
# Cap 3: Validación
La validación tiene como objetivo comprobar que el producto sea capaz de interpretar correctamente un binario compilado para Risc-V usando específicamente el set de instrucciones rv32im, y cumpliendo con funcionalidades básicas de un depurador tales como: mostrarle al usuario el detalle de las instrucciones interpretadas, mostrarle al usuario los valores de los registros, y la funcionalidad de avanzar y retroceder en la interpretación del binario manteniendo el estado de ejecución consistente. Además debería presentar las falencias que tiene y donde la solución no termina de cubrir los requisitos.

La evaluación se realizó utilizando dos códigos escritos en c, compilados para arquitectura rv32im. El primero de estos archivos es "fact10.c", que calcula el factorial de 10 usando una función recursiva, y al finalizar el cálculo imprime el resultado en la salida estándar. El objetivo de utilizar este código es que el interprete ejecute un código con un poco de complejidad y comprobar que la mayor cantidad de operaciones funcionan correctamente. Además de comprobar que el depurador es capaz de ejecutar saltos, llamadas a funciones y ramificaciones en la ejecución correctamente.

El segundo es el archivo "seg_fault.c", el cual crea un arreglo de 5 enteros e intenta acceder a una dirección de memoria fuera de rango, y al ejecutarlo da un error de 'segmentation fault'. El objetivo de evaluar utilizando este código es comprobar que el depurador es capaz de trabajar con códigos que compilan pero sin embargo tienen errores en su ejecución, y es aquí donde las funcionalidades de avanzar y retroceder en la ejecución del código es lo más útil.
## Método de Validación
## Resultados de la Validación

---
# Cap 4: Conclusiones y Trabajo a Futuro
## Trabajo a Futuro