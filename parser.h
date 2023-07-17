#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "codigos.h"
#include "util.h"
#include "error.h"
#include "ts.h"
#include "sistejec.h"


int last_call=0;

int flag=0;
int flagRet=0;
int flagMain=0;
int flagNoDeclarado=0;
int flagEsFuncion = 0;
int seleccion = 0;
int retornoIf = 0;
int seleccionElse = 0;
int retornoElse = 0;
int iteracion = 0;
int retornoWhile = 0;
int desplazamiento = 0;
int par = 0;
int esString = 0;

tipo_inf_res *act;
tipo_inf_res *primero;
tipo_inf_res p;

int gen;

int flagDeclaracion = 0;

float code[TAM_PROG];
int cindice = 0;

char strings[TAM_CTES];
int sindice = 0;


/*********** prototipos *************/

void unidad_traduccion();
void declaraciones();
int especificador_tipo();
void especificador_declaracion(int tipo, char *ident);
void definicion_funcion(int tipo, char *ident);
void declaracion_variable(int tipo, char *ident);
void lista_declaraciones_param(int posFuncion);
int declaracion_parametro(int posFuncion);
int declarador_init(int tipo, char *ident);
void lista_declaraciones_init(int tipo);
int lista_inicializadores(int tipo);
void lista_proposiciones(int tipo);
void lista_declaraciones();
void declaracion();
void proposicion(int tipo);
void proposicion_expresion();
void proposicion_compuesta(int tipo);
void proposicion_seleccion(int tipo);
void proposicion_iteracion(int tipo);
void proposicion_e_s();
void proposicion_retorno(int tipo);
int expresion(char *id);
int expresion_simple(char *id);
int termino(char *id);
int factor(char *in);
int variable(char *idRet);
int llamada_funcion();
int lista_expresiones();
int constante(char *cons);
