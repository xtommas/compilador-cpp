# Compilador

Compilador para un subconjunto del lenguaje C++ realizado para el curso _Diseño y Construcción de Compiladores_ de la Licenciatura en Ciencias de la Computación. El subconjunto particular de C++ se especifica en la siguiente gramática expresada en BNFE:

```bnf
<unidad de traducción> ::= { <declaraciones> }

<declaraciones> ::= <especificador de tipo> ident <especificador de declaración>

<especificador de tipo> ::= void | char | int | float

<especificador de declaración> ::= <definición de función> | <declaración de variable>

<definición de función> ::= ( [ <lista declaración de parámetros> ] ) <proposición compuesta>

<lista declaración de parámetros> ::= <declaración de parámetro> { , <declaración de parámetro> }

<declaración de parámetro> ::= <especificador de tipo> [ & ] ident [ [ ] ]

<declaración de variable> ::= <declarador init> [ , <lista declaraciones init> ] ;

<lista declaraciones init> ::= ident <declarador init> { , ident <declarador init> }

<declarador init> ::= [ = <constante> |
[ [ cons_ent ] ] [ = { <lista de inicializadores> } ] ]

<lista de inicializadores> ::= <constante> { , <constante> }

<proposición compuesta> ::= { [ <lista de declaraciones> ] [ <lista de proposiciones> ] }

<lista de declaraciones> ::= <declaración> { <declaración> }

<declaración> ::= <especificador de tipo> <lista declaraciones init> ;

<lista de proposiciones> ::= <proposición> { <proposición> }

<proposición> ::= <proposición expresión>
                | <proposición compuesta>
                | <proposición de iteración>
                | <proposición de selección>
                | <proposición de retorno>
                | <proposición de entrada/salida>

<proposición de iteración> ::= while ( <expresión> ) <proposición>

<proposición de selección> ::= if ( <expresión> ) <proposición> [ else <proposición> ]

<proposición de retorno> ::= return <expresión> ;

<proposición de entrada/salida> ::= cin >> <variable> { >> <variable> } ;
| cout << <expresión> { << <expresión> } ;

<proposición expresión> ::= [ <expresión> ] ;

<expresión> ::= <expresión simple> { = <expresión simple> | <relación> <expresión simple> }

<relación> ::= != | == | < | <= | >= | >

<expresión simple> ::= [ + | ] <término> { ( + | | || ) <término> }

<término> ::= <factor> { ( * | / | && ) <factor> }

<factor> ::= <variable>
          | <constante>
          | ! <expresión>
          | ( <expresión> )
          | <llamada a función>
          | cte_str

<variable> ::= ident | ident [ <expresión> ]

<llamada a función> ::= ident ( [ <lista de expresiones> ] )

<lista de expresiones> ::= <expresión> { , <expresión> }

<constante> ::= cons_ent | cons_float | cons_car
```

## Módulos del compilador

### Módulo Parser

Se compone de `parser.c` y su header `parser.h`, quien a su vez incluye otros headers, por ejemplo `var_globales.h`. El archivo `parser.c` es la implementación del parser descendente recursivo (PDR) para el subconjunto de la gramática de C++. El mismo es el eje central del compilador, dado que es el encargado de establecer el orden de las invocaciones al resto de los módulos. El archivo, `var_globales.h`, como su nombre lo indica, contiene las declaraciones de las variables globales.

### Módulo Scanner

Este módulo incluye el archivo `lex.yy.c`, el cual se obtiene a partir del archivo `scanner.token.lex` al compilarlo con lex. El `parser.c` se encarga de llamar a la función `yylex()` a través de la función `scanner()` definida en él. Además, está el archivo `codigos.h` que contiene los códigos devueltos por el scanner definidos como constantes.

### Módulo Errores

Se compone de los archivos `error.c` y `error.h`. A su vez, `error.c` posee dos funciones que son invocadas desde `parser.c`, `lex.tt.c` y `ts.c`:

```c
void error_handler(int ne)
```

Maneja la salida del compilador. Cuando la función recibe como argumentos `COD_IMP_ERRORES` imprimirá la línea actual y sucesivamente los errores correspondientes. Si un código de error es pasado como argumento, éste es agregado a la estructura definida en `error.h`, la cual va soportando los errores para la línea actual. `error_handler(COD_IMP_ERRORES)` es invocada principalmente desde `lex.yy.c` cuando reconoce el caracter _new line_.

```c
void error_print(int ne)
```

Función que contiene los posibles mensajes de errores en la etapa de compilación.

### Módulo Tabla de Símbolos

Este módulo contiene las funciones necesarias para la administración de la Tabla de Símbolos. Dichas funciones son las encargadas de manejar los objetos de datos definidos en el archivo fuente a compilar. El módulo de la Tabla de Símbolos está compuesto de un archivo header `ts.h` y un archivo `ts.c`.

### Módulo Sistema de Ejecución

Mantiene las estructuras soporte para el tiempo de ejecución e implementa las instrucciones de la _máquina abstracta C_. Está conformado por los archivos `sistejec.h` y `sistejec.c`.

## Instrucciones de uso

### Compilar el compilador

Para compilar y generar el ejecutable `ucc`, posicionado en el directorio que contiene el código fuente, se debe ejecutar en la terminal

```bash
make
```

Para eliminar los archivos objeto generados y el ucc se puede utilizar

```bash
make clean
```

### Utilizar el compilador

Para compilar un archivo fuente `.c` escrito en el sublenguaje especificado se utiliza

```bash
./ucc -c lote_de_pruba.c
```

Para ejecutar un archivo objeto `.o` generado por `ucc` a partir de una compilación exitosa

```bash
./ucc -o lote_de_prueba.o
```
