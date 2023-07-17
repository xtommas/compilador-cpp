#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "util.h"
#include "codigos.h"
#include "error.h"
#include "sistejec.h"

int gen = 1;


void scanner()
{	
	int i;
	for(; (i = yylex()) != NADA && lookahead() == SEGUIR;);

	if(i == NADA)
		sbol->codigo = CEOF;
	/* yylex retorna 0 si llego a fin de archivo */

	liberar = linea;
	linea = (char *) malloc (strlen(linea) + strlen (token1.lexema) + 3);
	strcpy(linea, liberar);
	strcat(linea, token1.lexema);
	free((void *) liberar);
}


void init_parser(int argc, char *argv[])
{
	linea = (char *) malloc (2);
	strcat(linea, "");
	nro_linea = 0;

	if(argc != 3)
	{
		error_handler(6);
		error_handler(COD_IMP_ERRORES);
		exit(1);
	}
	else
	{
		if(strcmp(argv[1], "-c") == 0)
		{
			if((yyin = fopen(argv[2], "r")) == NULL)
			{
				error_handler(8);
				error_handler(COD_IMP_ERRORES);
				exit(1);
			}
		}
		else
		{
			if(strcmp(argv[1], "-o") == 0){ // el segundo argumento es -o
				if(cargar_codgen(argv[2])){
					interprete();
					printf("\n\n");
					impr_codgen();
					printf("\n\n");
					exit(1);
				}
				else{
					error_handler(8); // el archivo no existe
				}
			}
		
		/*Aquí debe habilitarse la opción "-o" que carga el programa objeto en la máquina abstracta C 
		e inmediatamente debe procederse a la ejecución del mismo. Para ello, deben utilizarse de forma conveniente,
		las funciones "cargar_codgen", "impr_codgen" e "interprete" provistas en el archivo "sistejec.c"
		*/
		
			error_handler(7);
			error_handler(COD_IMP_ERRORES);
			exit(1);
		}
	}

	sbol = &token1;

	scanner();
}


void match(set codigo, int ne)
{
    if(lookahead() & codigo)
        scanner();
    else{
		gen = 0; // falló el match, entonces hay un error y no se debe generar código
		error_handler(ne);
	}
}


set lookahead()
{
	return sbol->codigo;
}


set lookahead_in(set conjunto)
{
	return lookahead() & conjunto;
}
