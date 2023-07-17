#include "parser.h"
#include <string.h>


int main(int argc, char *argv[])
{
	code[0] = INPP;
	cindice++;
	char archivo[50];
	// arma el nombre del archivo .o
	strcpy(archivo, argv[2]);
	int length = strlen(archivo);
	archivo[length - 1] = 'o';

	init_parser(argc, argv);
	inic_tablas();
	pushTB();
	desplazamiento = 0;
	gen = 1;
	//printf("GEN BEFORE: %d\n", gen);
	unidad_traduccion();
	
	match(CEOF, 9);

	if(flagMain==0){
		printf("\t Error 84: Falta declarar la funcion main()\n"); // No funciona aquí el error handler por algún motivo
		gen=0;
	}

	last_call=1;

	error_handler(COD_IMP_ERRORES);

	//printf("\nGEN: %d\n", gen);
	code[cindice] = PARAR;
	if(gen){
		guardar_codgen(code, strings, archivo);
		cargar_codgen(archivo);
		/*printf("\nCODIGO GENERADO\n\n");
		impr_codgen();
		printf("\n");*/
	}
	return 0;
}


/********* funciones del parser ***********/

void unidad_traduccion()
{
	while(lookahead_in(CVOID | CCHAR | CINT | CFLOAT)){
		declaraciones();
	}
}


void declaraciones()
{	
	int tipo = especificador_tipo(); 
	// Guardo el nombre del identificador para no perder el valor
	char ident[9];
	strcpy(ident, sbol->lexema);
	if(en_tabla(ident) != NIL){
		error_handler(72); // Identificador ya declarado
		gen = 0;
	}
	match(CIDENT, 17); 
	especificador_declaracion(tipo, ident);
}


int especificador_tipo()
{	
	switch(lookahead())
	{
		case CVOID:
			scanner();
			return en_tabla("void");
			break;
			
		case CCHAR:
			scanner();
			return en_tabla("char");
			break;
			
		case CINT:
			scanner();
			return en_tabla("int");
			break;
			
		case CFLOAT:
			scanner();
			return en_tabla("float");
			break;
			
		default:
			error_handler(18);
			gen = 0;
			break;
	}
}


void especificador_declaracion(int tipo, char *ident)
{	
	switch(lookahead())
	{
		case CPAR_ABR:
			//pushTB();
			desplazamiento=0;
			definicion_funcion(tipo, ident);
			pop_nivel();
			break;
		
		case CASIGNAC:
		case CCOR_ABR:
		case CCOMA:
		case CPYCOMA:
			if(tipo == en_tabla("void")){
				error_handler(73); // No se permite la declaración de variables o arreglos de tipo void
				gen = 0;
			}
			declaracion_variable(tipo, ident);
			break;
		
		default:
			error_handler(19);
			gen = 0;
			break;
	}
}


void definicion_funcion(int tipo, char *ident)
{
	match(CPAR_ABR, 20);
	int tipoInt = en_tabla("int");
	int tipoFloat = en_tabla("float");
	int tipoChar = en_tabla("char");

	int label;

	if(!strcmp(ident, "main")){ // Si se va a definir la funcion main
		if(flagMain){
			error_handler(101); // si ya se había definifo una funcion main, marca el error
			gen = 0;
		}
		flagMain = 1;
		if(tipo != en_tabla("void")){
			error_handler(85); // el tipo de main debe ser void
			gen = 0;
		}
	}

	// Si la función es distinta de main, se debe ignorar el código generado para la misma
	if(gen && strcmp(ident, "main") != 0){
		label = cindice;
	}
	// Alta de la función en TS
	strcpy(inf_id->nbre, ident);
	inf_id->clase = CLASFUNC;
	inf_id->ptr_tipo = tipo; // posición de la TS donde fue dado de alta el tipo de retorno en la inicialización de la misma
	inf_id->desc.nivel = get_nivel();
	if(tipo == tipoInt){ // Depende del tipo del parámetro, asigno el espacio correspondiente
				inf_id->cant_byte = sizeof(int);
	}else{
		if(tipo == tipoFloat){
			inf_id->cant_byte = sizeof(float);
		}else{
			if(tipo == tipoChar){
				inf_id->cant_byte = sizeof(char);
			}
		}
	}
	// Si no tuviera parámetros, quedaría apuntando a Null
	inf_id->desc.part_var.sub.ptr_inf_res = NULL;
	int posFuncion = insertarTS();
	pushTB();
	if(lookahead_in(CVOID | CCHAR | CINT | CFLOAT)){
		lista_declaraciones_param(posFuncion);	
	}

	if(flagMain){ // Se declaro la funcion main
		if(ts[posFuncion].ets->desc.part_var.sub.cant_par != 0){
			error_handler(86); // main no lleva argumentos
			gen = 0;
		}
	}
	match(CPAR_CIE, 21);
	desplazamiento=0;
	proposicion_compuesta(tipo);

	// Si se generó código para una función que no es main, se ignora volviendo cindice para atrás
	if(gen && strcmp(ident, "main") != 0){
		cindice = label;
	}
}


void lista_declaraciones_param(int posFuncion)
{
	int cantParams = 0;
	int params[50];
	params[0] = declaracion_parametro(posFuncion);
	cantParams++;
	// primero apunta al primer parámetro, act->ptr_sig al próximo, actualizo act para ir "encadenando" los parámetros y finalmente asigno el primero a la entrada de la función en TS, después puede ir haciendo el recorrido como una lista vinculada
	while(lookahead_in(CCOMA))
	{
		scanner();
		params[cantParams] = declaracion_parametro(posFuncion);
		cantParams++;
	}

	// Actualizo la cantidad de parámetros de la función
	ts[posFuncion].ets->desc.part_var.sub.cant_par = cantParams;
	// Asigno la lista de parámetros a la entrada de la función
	//ts[posFuncion].ets->desc.part_var.sub.ptr_inf_res = primero;

	// Cálculo del desplazamiento negativo de los parámetros
	for(int i=cantParams-1; i>=0; i--){
		desplazamiento-= ts[params[i]].ets->cant_byte;
		ts[params[i]].ets->desc.despl = desplazamiento;
	}
	// Armado de la lista de parámetros
	
	// Los punteros act, primero y la variable p son globales
	for(int i=0; i<cantParams; i++){
		p.ptero_tipo = ts[params[i]].ets->ptr_tipo;
		p.tipo_pje = ts[params[i]].ets->desc.part_var.param.tipo_pje;
		p.ptero_tipo_base = ts[params[i]].ets->desc.part_var.param.ptero_tipo_base;
		act = &p;
		if(i==cantParams-1){ // Es el último parámetro
			p.ptr_sig = NULL;
		}
		else{ // Hay más parámetros
			if(i==0){
				act = &p;
				primero = act;
			}
			act = p.ptr_sig;
		}
	}

	ts[posFuncion].ets->desc.part_var.sub.ptr_inf_res = primero;
}


int declaracion_parametro(int posFuncion)
{
	int tipoParam = especificador_tipo();
	if(tipoParam == en_tabla("void")){ // Declara un parámetro de tipo void, que está prohibido
		error_handler(73);
		gen = 0;
	}
	char pasaje = 'v';
	if(lookahead_in(CAMPER)){
		pasaje = 'r';
		scanner();
	}
		
	// Convierto a CIDENT a String 
	char ident[9];
	strcpy(ident, sbol->lexema);
	if(en_nivel_actual(ident) != NIL){
		error_handler(72); // Identificador ya declarado (se repiten nombres de parámetros o coinciden con el nombre de la función)
		gen = 0;
	}
	match(CIDENT, 17);
	
	int flagArray = 0; // flag para corroborar si el parámetro es un arreglo
	if(lookahead_in(CCOR_ABR))
	{
		scanner();
		match(CCOR_CIE, 22);
		flagArray = 1; // El parámetro es un arreglo 
	}

	if(flagArray && pasaje == 'r'){
		error_handler(92); // No se permite <tipo> & <id_arreglo> [] en la definicion de un parametro
		gen = 0;
	}
	// Alta de parámetro individual
	strcpy(inf_id->nbre, ident);
	inf_id->clase = CLASPAR;
	inf_id->ptr_tipo = tipoParam;

	int tipoInt = en_tabla("int");
	int tipoFloat = en_tabla("float");
	int tipoChar = en_tabla("char");
	if(tipoParam == tipoInt){ // Depende del tipo del parámetro, asigno el espacio correspondiente
		inf_id->cant_byte = sizeof(int);
	}else{
		if(tipoParam == tipoFloat){
			inf_id->cant_byte = sizeof(float);
		}else{
			if(tipoParam == tipoChar){
				inf_id->cant_byte = sizeof(char);
			}
		}
	}
	inf_id->desc.nivel = get_nivel();
	inf_id->desc.part_var.param.tipo_pje = pasaje;
	if(flagArray){
		inf_id->desc.part_var.param.ptero_tipo_base = tipoParam;
		inf_id->ptr_tipo = en_tabla("TIPOARREGLO");
	}
	else{
		inf_id->desc.part_var.param.ptero_tipo_base = -1; // Indico que no es un parámetro de tipo arreglo
	}
	int posParam = insertarTS();

	return posParam;
	
}


void lista_declaraciones_init(int tipo)
{
	char ident[9];
	char ident1[9];
	strcpy(ident, sbol->lexema);
	// Controlo si el identificador ya fue declarado en este nivel
	if(en_nivel_actual(ident) != NIL){
		error_handler(72);
		gen = 0; 
	}
	match(CIDENT, 17);
	// Controlo si el tipo a del identificador a declarar es void (no válido en la semántica)
	if(tipo == en_tabla("void")){
		error_handler(73);
		gen = 0;
	}
	declarador_init(tipo, ident);
	
	while(lookahead_in(CCOMA))
	{
		scanner();
		strcpy(ident1, sbol->lexema);
		// Controlo si el identificador ya fue declarado en este nivel
		if(en_nivel_actual(ident1) != NIL){
			error_handler(72);
			gen = 0;
		}
		match(CIDENT, 17);
		// No vuelvo a controlar el tipo, ya que es el mismo
		declarador_init(tipo, ident1);
	}
}


void declaracion_variable(int tipo, char *nombreIdent)
{
	int ret = declarador_init(tipo, nombreIdent);

	if(lookahead_in(CCOMA))
	{
		scanner();
		lista_declaraciones_init(tipo);
	}

	match(CPYCOMA, 23);
}


int declarador_init(int tipo, char *nombreIdent)
{
	int pos;
	flagDeclaracion = 0;
	// Variables auxiliares para realizar la asignación de la cantidad de bytes del objeto
	int tipoInt = en_tabla("int");
	int tipoFloat = en_tabla("float");
	int tipoChar = en_tabla("char");

	// Almacena el tamaño del arreglo, ya sea una constante o se listen los elementos
	char arrsizeAux[100];
	int arrsize = 0;
	int elementos = -1;
	flag = 0;
	char consAux[50];

	int tipoCons = -1;

	float consCRVL;

	switch(lookahead())
	{
		case CASIGNAC:
			scanner();
			flagDeclaracion = 1;
			tipoCons = constante(consAux); // consAux no se utiliza parar nada
			flagDeclaracion = 0;
			// Asignación de la información y alta en tabla de símbolos (caso en el que es una variable)
			if(tipoCons == tipoInt){
				tipoCons = 1; // 1 en CRVL
				consCRVL = atof(consAux);
			}
			else{
				if(tipoCons == tipoFloat){
					tipoCons = 2; // 2 en CRVL
					consCRVL = atof(consAux); 
				}
				else{
					if(tipoCons == tipoChar){
						tipoCons = 0; // 0 en CRVL
						consCRVL = (char) consAux[0];
					}
				}
			}
			strcpy(inf_id->nbre, nombreIdent);
			inf_id->clase = CLASVAR;
			inf_id->ptr_tipo = tipo;
			if(tipo == tipoInt){ // Depende del tipo del parámetro, asigno el espacio correspondiente
				inf_id->cant_byte = sizeof(int);
			}else{
				if(tipo == tipoFloat){
					inf_id->cant_byte = sizeof(float);
				}else{
					if(tipo == tipoChar){
						inf_id->cant_byte = sizeof(char);
					}
				}
			}
			inf_id->desc.nivel = get_nivel();
			inf_id->desc.despl = desplazamiento;
			desplazamiento+=inf_id->cant_byte;
			//printf("IDENTIFICADOR: %s NIVEL ACTUAL: %d DESPL: %d\n", inf_id->nbre, get_nivel(), inf_id->desc.despl);
			pos = insertarTS();
			//printf("Se dio de alta el identificador: %s en la posicion: %d\n", nombreIdent, pos);
			if(gen){
				code[cindice] = ALOC;
				cindice++;
				code[cindice] = ts[pos].ets->cant_byte;
				cindice++;
				code[cindice] = CRCT;
				cindice++;
				code[cindice] = tipoCons;
				cindice++;
				code[cindice] = consCRVL;
				cindice++;
				code[cindice] = ALM;
				cindice++;
				code[cindice] = ts[pos].ets->desc.nivel;
				cindice++;
				code[cindice] = ts[pos].ets->desc.despl;
				cindice++;
				code[cindice] = tipoCons;
				cindice++;
			}
			return 1;
			break;

		case CCOR_ABR:
			scanner();
			if(lookahead_in(CCONS_ENT)){
				flag = 1;
				strcpy(arrsizeAux, sbol->lexema);
				arrsize = atoi(arrsizeAux);
				flagDeclaracion = 1;
				tipoCons = constante(consAux);
				flagDeclaracion = 0;
				if(tipoCons != tipoInt){
					error_handler(74); // Debe usarse una constante entera en la declaración del arreglo
					gen = 0;
				}
			}
			match(CCOR_CIE, 22);
			if(lookahead_in(CASIGNAC))
			{
				scanner();
				match(CLLA_ABR, 24);
				elementos = lista_inicializadores(tipo);
				match(CLLA_CIE, 25);
			}
			// Verifico que el tamaño del arreglo sea mayor que 0
			if((flag==1 && arrsize <= 0) || (flag==0 && elementos <= 0)){
				error_handler(75);
				gen = 0;
			}
			// Verifico que las cantidades coincidan 
			if(flag==1 && elementos!=-1){ // Si se usó una constante y también se declararon elementos 
				if(arrsize < elementos){
					error_handler(76);
					gen = 0;
				}
			}
			
			//Asignación de la información y alta en tabla de símbolos (caso en el que se declara un arreglo)
			strcpy(inf_id->nbre, nombreIdent);
			inf_id->clase = CLASVAR;
			inf_id->ptr_tipo = en_tabla("TIPOARREGLO");
			inf_id->desc.nivel = get_nivel();
			inf_id->desc.part_var.arr.ptero_tipo_base = tipo;
			if(flag){
				inf_id->desc.part_var.arr.cant_elem = arrsize;
			}
			else{
				inf_id->desc.part_var.arr.cant_elem = elementos;
			}
			if(tipo == tipoInt){ // Depende del tipo del parámetro, asigno el espacio correspondiente
				inf_id->cant_byte = sizeof(int) * arrsize;
			}else{
				if(tipo == tipoFloat){
					inf_id->cant_byte = sizeof(float) * arrsize;
				}else{
					if(tipo == tipoChar){
						inf_id->cant_byte = sizeof(char) * arrsize;
					}
				}
			}
			inf_id->desc.despl = desplazamiento;
			desplazamiento+=inf_id->cant_byte;
			//printf("IDENTIFICADOR: %s NIVEL ACTUAL: %d DESPL: %d\n", inf_id->nbre, get_nivel(), inf_id->desc.despl);
			pos = insertarTS();
			//printf("Se dio de alta el identificador: %s en la posicion: %d\n", nombreIdent, pos);
			return 1;
			break;
	}

	strcpy(inf_id->nbre, nombreIdent);
	inf_id->clase = CLASVAR;
	inf_id->ptr_tipo = tipo;
	if(tipo == tipoInt){ // Depende del tipo del parámetro, asigno el espacio correspondiente
				inf_id->cant_byte = sizeof(int);
			}else{
				if(tipo == tipoFloat){
					inf_id->cant_byte = sizeof(float);
				}else{
					if(tipo == tipoChar){
						inf_id->cant_byte = sizeof(char);
					}
				}
			}
	inf_id->desc.nivel = get_nivel();
	inf_id->desc.despl = desplazamiento;
	desplazamiento+=inf_id->cant_byte;
	//printf("IDENTIFICADOR: %s NIVEL ACTUAL: %d DESPL: %d\n", inf_id->nbre, get_nivel(), inf_id->desc.despl);
	pos = insertarTS();
	if(gen){
		code[cindice] = ALOC;
		cindice++;
		code[cindice] = ts[pos].ets->cant_byte;
		cindice++;
	}
	//printf("Se dio de alta el identificador: %s en la posicion %d\n", nombreIdent, pos);
	return 0;
}


int lista_inicializadores(int tipo)
{
	int cantidadElem = 0;
	char cons[100];
	int tipoCons = -1;
	flagDeclaracion = 1;
	tipoCons = constante(cons);
	flagDeclaracion = 0;
	if((tipo == en_tabla("int") && tipoCons != en_tabla("int")) || (tipo == en_tabla("char") && tipoCons != en_tabla("char")) || (tipo == en_tabla("float") && tipoCons != en_tabla("float"))){
		error_handler(77); // El tipo de las constantes en la incialización del arreglo debe coincidir con el tipo con el cual fue declarado el arreglo
		gen = 0;
	}
	cantidadElem++;
	while(lookahead_in(CCOMA))
	{
		scanner();
		flagDeclaracion = 1;
		tipoCons = constante(cons);
		flagDeclaracion = 0;
		if((tipo == en_tabla("int") && tipoCons != en_tabla("int")) || (tipo == en_tabla("char") && tipoCons != en_tabla("char")) || (tipo == en_tabla("float") && tipoCons != en_tabla("float"))){
			error_handler(77); // El tipo de las constantes en la incialización del arreglo debe coincidir con el tipo con el cual fue declarado el arreglo
			gen = 0;
		}
		cantidadElem++;
	}
	return cantidadElem;
}


void proposicion_compuesta(int tipo)
{

	match(CLLA_ABR, 24);
	if(lookahead_in(CVOID | CCHAR | CINT | CFLOAT)){
		lista_declaraciones();
	}

	if(lookahead_in(CLLA_ABR | CMAS | CMENOS | CIDENT | CPAR_ABR | CNEG |
						 CCONS_ENT | CCONS_FLO | CCONS_CAR | CCONS_STR |
						 CIF | CWHILE | CIN | COUT | CPYCOMA | CRETURN)){
							lista_proposiciones(tipo);
						 }
		

	if(tipo != en_tabla("void") && flagRet == 0){
		error_handler(88); // falta return
		gen = 0;
	}
	if(tipo != en_tabla("void") && retornoIf && retornoElse == 0 && flagRet == 0){
		error_handler(88); // falta return
		gen = 0;
	}
	if(tipo != en_tabla("void") && retornoIf==0 && retornoElse && flagRet == 0){
		error_handler(88); // falta return
		gen = 0;
	}
	if(tipo != en_tabla("void") && retornoWhile && flagRet == 0){
		error_handler(88); // falta return
		gen = 0;
	}
	match(CLLA_CIE, 25);
	//popTB();
	flagRet = 0;
	seleccion = 0;
	retornoIf = 0;
	seleccionElse = 0;
	retornoElse = 0;
	iteracion = 0;
	retornoWhile = 0;
}


void lista_declaraciones()
{
	declaracion(); 

	while(lookahead_in(CVOID | CCHAR | CINT | CFLOAT))
		declaracion(); 
}


void declaracion()
{
	int tipo = especificador_tipo();

	lista_declaraciones_init(tipo);

	match(CPYCOMA, 23);
}


void lista_proposiciones(int tipo)
{
	proposicion(tipo);
	while(lookahead_in(CLLA_ABR | CMAS | CMENOS | CIDENT | CPAR_ABR | CNEG |
							 CCONS_ENT | CCONS_FLO | CCONS_CAR | CCONS_STR |
							 CIF | CWHILE | CIN | COUT | CPYCOMA | CRETURN)){
								 proposicion(tipo);
							 }
		
}


void proposicion(int tipo)
{
	switch(lookahead())
	{
		case CLLA_ABR:
			proposicion_compuesta(tipo);
			break;
		
		case CWHILE:
			proposicion_iteracion(tipo);
			break;
		
		case CIF:
			proposicion_seleccion(tipo);
			break;
		
		case CIN: 
		case COUT:
			proposicion_e_s();
			break;
		
		case CMAS:
		case CMENOS:
		case CIDENT:
		case CPAR_ABR:
		case CNEG:
		case CCONS_ENT:
		case CCONS_FLO:
		case CCONS_CAR:
		case CCONS_STR:
		case CPYCOMA:
			proposicion_expresion();
			break;
		
		case CRETURN:
			if(tipo == en_tabla("void")){
				error_handler(89); // un procedimiento no lleva return
				gen = 0;
			}
			flagRet = 1;
			proposicion_retorno(tipo);
			break;
		
		default:
			error_handler(26);
			gen = 0;
			break;
	}
}


void proposicion_iteracion(int tipo)
{
	match(CWHILE, 27);
	iteracion = 1;
	match(CPAR_ABR, 20);
	char id[256];
	strcpy(id, sbol->lexema);

	int labelCond;
	int labelD1;
	if(gen){
		labelCond = cindice;
	}
	int tipoGC = expresion(id);

	if(gen){
		int type;
		if(tipoGC == en_tabla("int")){
			type = 1;
		}
		else{
			if(tipoGC == en_tabla("float")){
				type = 2;
			}
			else{
				if(tipoGC == en_tabla("char")){
					type = 0;
				}
			}
		}
		code[cindice] = BIFF;
		cindice++;
		code[cindice] = type;
		cindice++;
		labelD1 = cindice;
		cindice++;
	}
	match(CPAR_CIE, 21);
	proposicion(tipo);

	if(gen){
		code[cindice] = BIFS;
		cindice++;
		code[cindice] = labelCond - cindice;
		cindice++;
		code[labelD1] = cindice - labelD1;
	}
}


void proposicion_seleccion(int tipo)
{
	match(CIF, 28);
	seleccion = 1;
	match(CPAR_ABR, 20);

	char id[100];
	strcpy(id, sbol->lexema);
	int tipoGC;
	tipoGC = expresion(id);
	match(CPAR_CIE, 21);

	int labelElse;
	int labelFin;
	int conElse = 0;

	if(gen){
		int type;
		if(tipoGC == en_tabla("int")){
			type = 1;
		}
		else{
			if(tipoGC == en_tabla("float")){
				type = 2;
			}
			else{
				if(tipoGC == en_tabla("char")){
					type = 0;
				}
			}
		}
		code[cindice] = BIFF;
		cindice++;
		code[cindice] = type;
		cindice++;
		labelElse = cindice;
		cindice++;
	}

	proposicion(tipo);

	if(lookahead_in(CELSE))
	{
		if(gen){
			code[cindice] = BIFS;
			cindice++;
			labelFin = cindice;
			cindice++;
			code[labelElse] = cindice - labelElse;
		}
		scanner();
		seleccionElse = 1;
		conElse = 1;
		proposicion(tipo);

		if(gen){
			code[labelFin] = cindice - labelFin;
		}
	}

	if(gen && conElse == 0){
		code[labelElse] = cindice - labelElse;
	}

}


void proposicion_e_s()
{
	//flag = 0;
	char id[100];
	//char id2[100];
	int tipo = -1;
	switch(lookahead())
	{
		case CIN:
			scanner();
			match(CSHR, 30);
			strcpy(id, sbol->lexema);

			tipo = variable(id);
			if(tipo != en_tabla("char") && tipo != en_tabla("int") && tipo != en_tabla("float")){
				error_handler(95); // Las propsiciones de E/S solo aceptan variables de tipo int, float o char
				gen = 0;
			}

			if(gen){

				int pos = en_nivel_actual(id);
				if(pos == NIL){
					pos = en_nivel_cero(id); // es global
				}
				cindice = cindice-4; // borro el CRVL generado en variable
				int type;
				if(tipo == en_tabla("int")){
					type = 1;
				}
				else{
					if(tipo == en_tabla("float")){
						type = 2;
					}
					else{
						if(tipo == en_tabla("char")){
							type = 0;
						}
					}
				}

				code[cindice] = LEER;
				cindice++;
				code[cindice] = type;
				cindice++;
				code[cindice] = ALM;
				cindice++;
				code[cindice] = ts[pos].ets->desc.nivel;
				cindice++;
				code[cindice] = ts[pos].ets->desc.despl;
				cindice++;
				code[cindice] = type;
				cindice++;
			}
			
			while(lookahead_in(CSHR))
			{
				scanner();
				strcpy(id, sbol->lexema);
				tipo = variable(id);
				if(tipo != en_tabla("char") && tipo != en_tabla("int") && tipo != en_tabla("float")){
					error_handler(95); // Las propocisiones de E/S solo aceptan variables de tipo int, float o char
					gen = 0;
				}

				if(gen){
					int pos = en_nivel_actual(id);
					if(pos == NIL){
						pos = en_nivel_cero(id); // es global
					}
					cindice = cindice-4; // borro el CRVL generado en variable
					int type;
					if(tipo == en_tabla("int")){
						type = 1;
					}
					else{
						if(tipo == en_tabla("float")){
							type = 2;
						}
						else{
							if(tipo == en_tabla("char")){
								type = 0;
							}
						}
					}

					code[cindice] = LEER;
					cindice++;
					code[cindice] = type;
					cindice++;
					code[cindice] = ALM;
					cindice++;
					code[cindice] = ts[pos].ets->desc.nivel;
					cindice++;
					code[cindice] = ts[pos].ets->desc.despl;
					cindice++;
					code[cindice] = type;
					cindice++;
				}

			}

			match(CPYCOMA, 23);
			
			break;
		
		case COUT:
			scanner();

			match(CSHL, 31);
			strcpy(id, sbol->lexema);
			esString = 0;
			tipo = expresion(id);
			if(tipo != en_tabla("char") && tipo != en_tabla("int") && tipo != en_tabla("float")){
				error_handler(95); // Las proposiciones de E/S solo aceptan expresiones de tipo int, float o char
				gen = 0;
			}

			if(gen){
				int type;
				if(tipo == en_tabla("int")){
					type = 1;
				}
				else{
					if(tipo == en_tabla("float")){
						type = 2;
					}
					else{
						if(tipo == en_tabla("char")){
							type = 0;
						}
					}
				}

				if(esString){
					code[cindice] = IMPRCS;
					cindice++;
				}
				else{
					code[cindice] = IMPR;
					cindice++;
					code[cindice] = type;
					cindice++;
				}
				
			}

			while(lookahead_in(CSHL))
			{
				scanner();
				strcpy(id, sbol->lexema);
				esString = 0;
				tipo = expresion(id);
				if(tipo != en_tabla("char") && tipo != en_tabla("int") && tipo != en_tabla("float")){
					error_handler(95); // Las proposiciones de E/S solo aceptan expresiones de tipo int, float o char
					gen = 0;
				}

				if(gen){
					int type;
					if(tipo == en_tabla("int")){
						type = 1;
					}
					else{
						if(tipo == en_tabla("float")){
							type = 2;
						}
						else{
							if(tipo == en_tabla("char")){
								type = 0;
							}
						}
					}

					if(esString){
						code[cindice] = IMPRCS;
						cindice++;
					}
					else{
						code[cindice] = IMPR;
						cindice++;
						code[cindice] = type;
						cindice++;
					}
				}
			}
			match(CPYCOMA, 23);
			break;
		
		default:
			error_handler(29);
			gen = 0;
			break;
	}
}


void proposicion_retorno(int tipo)
{
	scanner();
	if(seleccion){
		retornoIf = 1;
	}
	if(seleccionElse){
		retornoElse = 1;
	}
	if(iteracion){
		retornoWhile = 1;
	}
	int tipoRet = -1;
	char id[100];
	strcpy(id, sbol->lexema);
	tipoRet = expresion(id);
	if(tipo != tipoRet){
		error_handler(87); // el tipo del valor de retorno no coincide con el especificado en la función
		gen = 0;
	}
	match(CPYCOMA, 23);
}


void proposicion_expresion()
{
	if(lookahead_in(CMAS | CMENOS | CIDENT | CPAR_ABR | CNEG | CCONS_ENT | CCONS_FLO | CCONS_CAR | CCONS_STR)){
		char id[256];
		strcpy(id, sbol->lexema);
		expresion(id);
	}

	match(CPYCOMA, 23);	
}


int expresion(char *id)
{	
	flag = 0;
	esString = 0;
	int flagCopia;
	int tipoOp1;
	int tipoOp2;
	int tipoOp3;
	char id1[100];
	char id2[100];
	char id3[100];
	strcpy(id1, sbol->lexema);
	tipoOp1 = expresion_simple(id1);
	strcpy(id, id1);
	int posOperando;
	int posOperando2;
	int posOperando3;

	char op[4];
	int cast = 0;
	
	int ret;
	if(Clase_Ident(id1) == CLASVAR || Clase_Ident(id1) == CLASFUNC || Clase_Ident(id1) == CLASPAR){
		posOperando = en_nivel_actual(id1);
		if(posOperando == NIL && esString == 0){
			posOperando = en_nivel_cero(id1); // es global
			if(posOperando == NIL || ts[posOperando].ets->desc.nivel != 0){
				error_handler(71); // Identificador no declarado
				gen = 0;
			}
		}
	}
	

	ret = tipoOp1;
	/*if(ret == en_tabla("TIPOARREGLO") && flag && posOperando != NIL){
		ret = ts[posOperando].ets->desc.part_var.arr.ptero_tipo_base;
	}*/


	if((strcmp(sbol->lexema, "=") == 0) && gen){ // cuando vuelve de expresión simple, se genera un CRVL en variable, pero si se va a realizar un asignación debe eliminarse.
		cindice = cindice-4;
	}
	while(lookahead_in(CASIGNAC | CDISTINTO | CIGUAL | CMENOR | CMEIG | CMAYOR | CMAIG))
	{
		switch(lookahead())
		{
			case CASIGNAC:
				scanner();

				// Si el identificador es un arreglo y no se hizo el acceso a un elemento
				if((tipoOp1 == en_tabla("TIPOARREGLO")) && flag == 0){
					error_handler(81); // No se pueden asignar arreglos como un todo
					gen = 0;
				}
				flagCopia = flag;
				flag = 0; // reseteo el flag que indica si se accede a una posición del arreglo
				strcpy(id2, sbol->lexema);
				tipoOp2 = expresion_simple(id2);
				posOperando2 = en_nivel_actual(id2);
				// Si el identificador es un arreglo y no se hizo el acceso a un elemento
				if((tipoOp2 == en_tabla("TIPOARREGLO")) && flag == 0){
					error_handler(81); // No se pueden asignar arreglos como un todo
					gen = 0;
				}

				if(tipoOp2 == en_tabla("TIPOARREGLO") && flag && posOperando2 != NIL){
					tipoOp2 = ts[posOperando2].ets->desc.part_var.arr.ptero_tipo_base;
				}
				// Chequeo de coerciones
				if(tipoOp1 == tipoOp2){
					ret = tipoOp1;
				}			
				else{
					cast = 1;

					if(tipoOp1 == en_tabla("int") && tipoOp2 == en_tabla("char")){
						ret = tipoOp1;
					}
					else{
						if(tipoOp1 == en_tabla("int")){
							error_handler(102); // Asignación no válida
							gen = 0;
						}
						else{
							if(tipoOp1 == en_tabla("char") && tipoOp2 != en_tabla("char")){
								error_handler(102); // Asignación no válida
								gen = 0;
							}
							else{
								if(tipoOp1 == en_tabla("float")){
									ret = tipoOp1;
								}
							}
						}
					}
				}

				if(gen && flagCopia == 0 && flag == 0){
					int type;
					if(tipoOp1 == en_tabla("int")){
						type = 1;
					}
					else{
						if(tipoOp1 == en_tabla("float")){
							type = 2;
						}
						else{
							if(tipoOp1 == en_tabla("char")){
								type = 0;
							}
						}
					}
					int nvl = ts[posOperando].ets->desc.nivel;
					int dsp = ts[posOperando].ets->desc.despl;

					if(cast){
						int type2;
						if(tipoOp2 == en_tabla("int")){
							type2 = 1;
						}
						else{
							if(tipoOp2 == en_tabla("float")){
								type2 = 2;
							}
							else{
								if(tipoOp2 == en_tabla("char")){
									type2 = 0;
								}
							}
						}

						code[cindice] = CAST;
						cindice++;
						code[cindice] = type2;
						cindice++;
						code[cindice] = type;
						cindice++;
					}

					code[cindice] = ALM;
					cindice++;
					code[cindice] = nvl;
					cindice++;
					code[cindice] = dsp;
					cindice++;
					code[cindice] = type;
					cindice++;

				}
				break;
				
			case CDISTINTO:
			case CIGUAL:
			case CMENOR:
			case CMEIG:
			case CMAYOR:
			case CMAIG:
				strcpy(op, sbol->lexema);
				scanner();
				if((tipoOp1 == en_tabla("TIPOARREGLO")) && flag == 0){
					error_handler(96); // No se pueden asignar arreglos como un todo
					gen = 0;
				}
				flag = 0;
				strcpy(id3, sbol->lexema);
				tipoOp3 = expresion_simple(id3);
				posOperando3 = en_nivel_actual(id3);
				// Si el identificador es un arreglo y no se hizo el acceso a un elemento
				if((tipoOp3 == en_tabla("TIPOARREGLO")) && flag == 0){
					error_handler(96); // operadores relacionales solo aceptan char, int o float
					gen = 0;
				}
				if((tipoOp3 == en_tabla("TIPOARREGLO")) && flag && posOperando != NIL){
						tipoOp3 = ts[posOperando].ets->desc.part_var.arr.ptero_tipo_base;
				}
				if((tipoOp1 != en_tabla("int") && tipoOp1 != en_tabla("float") && tipoOp1 != en_tabla("char")) || (tipoOp3 != en_tabla("int") && tipoOp3 != en_tabla("float") && tipoOp3 != en_tabla("char"))){
					error_handler(96); // operadores relacionales solo aceptan char, int o float
					gen = 0;
				}
				if(ret == en_tabla("float") || tipoOp3 == en_tabla("float")){
					ret = en_tabla("float");
				}
				else{
					if(ret == en_tabla("char") && tipoOp3 != en_tabla("char")){
						ret = tipoOp3;
					}
				}

				if(gen){
					int operacion;
					if(strcmp(op, "!=") == 0){
						operacion = 0;
					}
					else{
						if(strcmp(op, "==") == 0){
							operacion = 1;
						}
						else{
							if(strcmp(op, "<") == 0){
								operacion = 2;
							}
							else{
								if(strcmp(op, "<=") == 0){
									operacion = 3;
								}
								else{
									if(strcmp(op, ">") == 0){
										operacion = 4;
									}
									else{
										if(strcmp(op, ">=") == 0){
											operacion = 5;
										}
									}
								}
							}
						}
					}

					int type2;
					if(tipoOp1 == en_tabla("int")){
						type2 = 1;
					}
					else{
						if(tipoOp1 == en_tabla("float")){
							type2 = 2;
						}
						else{
							if(tipoOp1 == en_tabla("char")){
								type2 = 0;
							}
						}
					}

					switch(operacion){
						case 0: // distinto
								code[cindice] = CMNI;
								cindice++;
								code[cindice] = type2;
								cindice++;
								break;
						case 1: // igual
								code[cindice] = CMIG;
								cindice++;
								code[cindice] = type2;
								cindice++;
								break;
						case 2: // menor
								code[cindice] = CMME;
								cindice++;
								code[cindice] = type2;
								cindice++;
								break;
						case 3: // menor o igual
								code[cindice] = CMEI;
								cindice++;
								code[cindice] = type2;
								cindice++;
								break;
						case 4: // mayor
								code[cindice] = CMMA;
								cindice++;
								code[cindice] = type2;
								cindice++;
								break;
						case 5: // mayor o igual
								code[cindice] = CMAI;
								cindice++;
								code[cindice] = type2;
								cindice++;
								break;
					}

				}
				break;
		}
	}

	return ret;
}


int expresion_simple(char *id)
{
	int flagmenos = 0;
	if(lookahead_in(CMAS | CMENOS)){
		if(sbol->codigo == CMENOS){
			flagmenos = 1;
		}
		scanner();
	}

	int tipo = termino(id);
	int tipo2;
	char id2[100];

	int operacion = -1;

	if(gen && flagmenos){
		int type;
		if(tipo == en_tabla("int")){
				type = 1;
		}
		else{
			if(tipo == en_tabla("float")){
				type = 2;
			}
			else{
				if(tipo == en_tabla("char")){
					type = 0;
				}
			}
		}
		code[cindice] = INV;
		cindice++;
		code[cindice] = type;
		cindice++;
	}

	//ASUMO QUE OPERAMOS TÉRMINOS DEL MISMO TIPO
	while(lookahead_in(CMAS | CMENOS | COR))
	{
		if(sbol->codigo == CMAS){
			operacion = 0;
		}
		else{
			if(sbol->codigo == CMENOS){
				operacion = 1;
			}
			else{
				operacion = 2;
			}
		}
		scanner();
		tipo2 = termino(id2);
		if(operacion == 2 && tipo != en_tabla("char") && tipo != en_tabla("int") && tipo != en_tabla("float") && tipo2 != en_tabla("char") && tipo2 != en_tabla("int") && tipo2 != en_tabla("float")){
			error_handler(96); // el tipo de los operandos de los operadores logicos puede ser char, int o float
			gen = 0;
		}

		if(gen){
			int type2;
			if(tipo == en_tabla("int")){
				type2 = 1;
			}
			else{
				if(tipo == en_tabla("float")){
					type2 = 2;
				}
				else{
					if(tipo == en_tabla("char")){
						type2 = 0;
					}
				}
			}
			
			switch(operacion){
				case 0: //suma
						code[cindice] = SUM;
						cindice++;
						code[cindice] = type2;
						cindice++;
						break;
				case 1: // resta
						code[cindice] = SUB;
						cindice++;
						code[cindice] = type2;
						cindice++;
						break;
				case 2: // or
						code[cindice] = OR;
						cindice++;
						code[cindice] = type2;
						cindice++;
						break;
			}
		}
		
	}

	return tipo;
}


int termino(char *id)
{	
	int tipo = factor(id);
	int tipo2;
	int tipoRet;
	char id2[100];
	int operacion = -1;

	// ASUMO QUE OPERAMOS CON FACTORES DEL MISMO TIPO
	while(lookahead_in(CMULT | CDIV | CAND))
	{
		if(sbol->codigo == CMULT){
			operacion = 0;
		}
		else{
			if(sbol->codigo == CDIV){
				operacion = 1;
			}
			else{
				operacion = 2;
			}
		}
		scanner();
		tipo2 = factor(id2);
		if(operacion == 2 && tipo != en_tabla("char") && tipo != en_tabla("int") && tipo != en_tabla("float") && tipo2 != en_tabla("char") && tipo2 != en_tabla("int") && tipo2 != en_tabla("float")){
			error_handler(96); // el tipo de los operandos de los operadores logicos puede ser char, int o float
			gen = 0;
		}

		if(gen){
			int tipoGen;
			if(tipo == en_tabla("int")){
				tipoGen = 1;
			}
			else{
				if(tipo == en_tabla("float")){
					tipoGen = 2;
				}
				else{
					if(tipo == en_tabla("char")){
						tipoGen = 0;
					}
				}
			}
			switch(operacion){
				case 0: //multiplicacion
						code[cindice] = MUL;
						cindice++;
						code[cindice] = tipoGen;
						cindice++;
						break;
				case 1: //division
						code[cindice] = DIV;
						cindice++;
						code[cindice] = tipoGen;
						cindice++;
						break;
				case 2: // and
						code[cindice] = AND;
						cindice++;
						code[cindice] = tipoGen;
						cindice++;
						break;
			}
		}
		
	}

	return tipo;
}


int factor(char *in)
{
	int clase; // almacena la clase del identificador (retornada por la TS). Se utiliza para resolver si se debe bifurcar a llamada_funcion() o a variable()
	int tipo=-1;
	char ident[100];
	int sindiceaux;
	switch(lookahead())
	{
		case CIDENT:
			clase = Clase_Ident(sbol->lexema);
			if(clase == NIL){
				flagNoDeclarado = 1;
				error_handler(71);
				gen = 0;
			}
			switch(clase){
					case CLASVAR:  tipo = variable(in);
								   return tipo; 
								   break;
					case CLASPAR:  	par = 1;
									tipo = variable(in); // Si es un parámetro, también puede usarse dentro de la función
									par = 0;
								   return tipo; 
								   break;
					case CLASFUNC: 	// Es llamada a función, se debe dar de alta un nuevo bloque
									//pushTB();
									flagEsFuncion = 1;
									tipo = llamada_funcion();
									// Vuelve de la llamada, se debe dar de baja el bloque
									//pop_nivel();
									flagEsFuncion = 0;
									in = "funcion";
									return tipo;
									break;
					default: match(CIDENT, 17);
							 tipo = -1; 
							 return tipo;
							 break;
			}
			/********************************************/
			/* El alumno debera evaluar con consulta a TS
			si bifurca a variable o llamada a funcion */
			// HECHO
			break;
		
		case CCONS_ENT:
		case CCONS_FLO:
		case CCONS_CAR:
			tipo = constante(in);
			return tipo;
			break;
		
		case CCONS_STR:
			strcpy(in, sbol->lexema);
			scanner();
			if(gen){
				sindiceaux = sindice;
				for(int i=0; i<strlen(in); i++){
					strings[sindice] = in[i];
					sindice++;
				}
				strings[sindice] = '\0';
				sindice++;
				code[cindice] = CRCTS;
				cindice++;
				code[cindice] = sindiceaux;
				cindice++;
			}
			esString = 1;
			return en_tabla("char");
			break;
		
		case CPAR_ABR:
			scanner();
			strcpy(in, sbol->lexema);
			tipo = expresion(in);
			match(CPAR_CIE, 21);
			return tipo;
			break;
			
		case CNEG:
			scanner();
			strcpy(in, sbol->lexema);
			tipo = expresion(in);
			if(tipo != en_tabla("int") && tipo != en_tabla("float") && tipo != en_tabla("char")){
				error_handler(96); // Los operadores lógicos aceptan solo char, int o float
				gen = 0;
			}
			if(gen){
				int tiponeg = -1;

				if(tipo == en_tabla("int")){
					tiponeg = 1;
				}else{
					if(tipo == en_tabla("float")){
						tiponeg = 2;
					}else{
						if(tipo == en_tabla("char")){
							tiponeg = 0;
						}
					}
				}

				code[cindice] = NEG;
				cindice++;
				code[cindice] = tiponeg;
				cindice++;
			}
			return tipo;
			break;
			
		default:
			error_handler(32);
			gen = 0;
			return tipo;
			break;
	}
}


int variable(char *ident)
{
	flagNoDeclarado = 0;
	strcpy(ident, sbol->lexema);
	int posArr = en_nivel_actual(ident);
	if(posArr == NIL){
		posArr = en_nivel_cero(ident); // es global
		if(posArr == NIL || ts[posArr].ets->desc.nivel != 0){
			error_handler(71); // Identificador no declarado
			flagNoDeclarado = 1;
			gen = 0;
		}
	}
	match(CIDENT, 17);
	int tipoRet = -1;
	int cantElem = 0;
	if(flagNoDeclarado==0){
		tipoRet = Tipo_Ident(ident);
	}
	/* El alumno debera verificar con una consulta a TS
	si, siendo la variable un arreglo, corresponde o no
	verificar la presencia del subindice */

	if(lookahead_in(CCOR_ABR))
	{
		flag = 1;
		scanner();
		if(flagNoDeclarado==0){
			if(ts[posArr].ets->ptr_tipo != en_tabla("TIPOARREGLO")){ // Se quiere acceder a un objeto que no es un arreglo
				error_handler(78);
				gen = 0;
			}
			if(par == 0){
				cantElem = ts[posArr].ets->desc.part_var.arr.cant_elem;
				tipoRet = ts[posArr].ets->desc.part_var.arr.ptero_tipo_base;
			}else{
				tipoRet = ts[posArr].ets->desc.part_var.param.ptero_tipo_base;
			}
		}
		char cons[256];
		strcpy(cons, sbol->lexema);
		int tipo = expresion(cons);
		if(tipo != en_tabla("int")){
			error_handler(80); // El índice del arreglo debe ser una constante entera
			gen = 0;
		}
		// Falta acceder al valor de la constante para verificar si el índice es válido
		if(flagNoDeclarado == 0 && par == 0){
			if(tipo == en_tabla("int")){
				int acceso = atoi(cons);
				if(acceso < 0 || acceso>= cantElem){
					error_handler(80);
					gen = 0;
				}
			}
		}
		match(CCOR_CIE, 22);
	}

	if(tipoRet == en_tabla("TIPOARREGLO") && flag == 0 && flagEsFuncion==0){
		error_handler(79); // En una expresión, se debe acceder a una componente del arreglo
		gen = 0;
	}

	if(gen && flag == 0){ // si flag = 1, se quiso acceder a un arreglo, aquí solo consideramos variables simples. 
		int tipo;
		code[cindice] = CRVL;
		cindice++;
		code[cindice] = ts[posArr].ets->desc.nivel;
		cindice++;
		code[cindice] = ts[posArr].ets->desc.despl; 
		cindice++;
		if(tipoRet == en_tabla("int")){
			tipo = 1;
		}else{
			if(tipoRet == en_tabla("float")){
				tipo = 2;
			}else{
				tipo = 0;
			}
		}
		code[cindice] = tipo;
		cindice++;
	}
	return tipoRet;
	
}


int llamada_funcion()
{
	flagNoDeclarado = 0;
	int tipo = -1;
	char ident[10];
	strcpy(ident, sbol->lexema);
	int posFuncion = en_tabla(ident);
	if( posFuncion == NIL){
		flagNoDeclarado = 1;
		error_handler(71); // identificador no declarado
		gen = 0;
	}
	if(flagNoDeclarado == 0 && Clase_Ident(ident) != CLASFUNC){
		error_handler(99); // El identificador debe ser una función
		gen = 0;
	}
	match(CIDENT, 17);

	match(CPAR_ABR, 20);
	int parametros = 0;
	if(lookahead_in(CMAS | CMENOS | CIDENT | CPAR_ABR | CNEG | CCONS_ENT | CCONS_FLO | CCONS_CAR | CCONS_STR)){
		parametros = lista_expresiones();
	}
	if(posFuncion != NIL && ts[posFuncion].ets->desc.part_var.sub.cant_par != parametros){
		error_handler(90); // La cantidad de parámetros reales y formales no coincide
		gen = 0;
	}
	if(posFuncion != NIL){
		tipo = ts[posFuncion].ets->ptr_tipo;
	}
	else{
		tipo = -1;
	}
	match(CPAR_CIE, 21);
	return tipo;
}


int lista_expresiones()
{
	int parametros = 0;
	char id[100];
	strcpy(id, sbol->lexema);
	expresion(id);
	parametros++;
	
	while(lookahead_in(CCOMA))
	{
		scanner();
		strcpy(id, sbol->lexema);
		expresion(id);
		parametros++;
	}

	return parametros;
}


int constante(char *cons)
{
	switch(lookahead())
	{
		case CCONS_ENT:
			strcpy(cons, sbol->lexema);
			scanner();
			if(gen && flagDeclaracion == 0){ // si flagdeclaracion es 1, se genera el código en declaracion
				code[cindice] = CRCT;
				cindice++;
				code[cindice] = 1;
				cindice++;
				code[cindice] = atof(cons);
				cindice++;
			}
			return en_tabla("int");
			break;
		
		case CCONS_FLO:
			strcpy(cons, sbol->lexema);
			scanner();
			if(gen && flagDeclaracion == 0){
				code[cindice] = CRCT;
				cindice++;
				code[cindice] = 2;
				cindice++;
				code[cindice] = atof(cons);
				cindice++;
			}
			return en_tabla("float");
			break;
		
		case CCONS_CAR:
			strcpy(cons, sbol->lexema);
			scanner();
			if(gen && flagDeclaracion == 0){
				code[cindice] = CRCT;
				cindice++;
				code[cindice] = 2;
				cindice++;
				code[cindice] = (char) cons[0];
				cindice++;
			}
			return en_tabla("char");
			break;
		
		default:
			error_handler(33);
			gen = 0;
			break;
	}
}
