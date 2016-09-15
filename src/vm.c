/*
The MIT License (MIT)

Copyright (c) 2015 - Latino

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/
#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "compat.h"
#include "vm.h"
#include "libmem.h"
#include "libmath.h"
#include "libstring.h"
#include "libio.h"
#include "liblist.h"
#include "libnet.h"
#include "parse.h"
#include "gc.h"

static const char *const bycode_nombre[] = {
    "NOP",
    "HALT",
    "UNARY_MINUS",
    "BINARY_ADD",
    "BINARY_SUB",
    "BINARY_MUL",
    "BINARY_DIV",
    "BINARY_MOD",
    "OP_GT",
    "OP_GE",
    "OP_LT",
    "OP_LE",
    "OP_EQ",
    "OP_NEQ",
    "OP_AND",
    "OP_OR",
    "OP_NOT",
    "OP_INC",
    "OP_DEC",
    "CONCAT",
    "LOAD_CONST",
    "LOAD_NAME",
    "STORE_NAME",
    "JUMP_ABSOLUTE",
    "POP_JUMP_IF_FALSE",
    "POP_JUMP_IF_TRUE",
    "SETUP_LOOP",
    "POP_BLOCK",
    "CALL_FUNCTION",
    "RETURN_VALUE",
    "MAKE_FUNCTION",    
    "INC",    
    "DEC", 
    "LOAD_ATTR",
    "BUILD_LIST",
    "STORE_SUBSCR",
    "BINARY_SUBSCR",
};


static void __registrar_cfuncion(lat_mv* vm, char *palabra_reservada, void (*function)(lat_mv* vm), int num_params)
{
    lat_objeto *ctx = lat_obtener_contexto(vm);
    lat_objeto *nombre = lat_cadena_nueva(vm, palabra_reservada);
    lat_objeto *cfun = lat_definir_cfuncion(vm, function);
    cfun->nombre_cfun = palabra_reservada;
    cfun->num_params = num_params;
    lat_asignar_contexto_objeto(ctx, nombre, cfun);    
    //se agregan funciones para que se eliminen con el Colector de basura
    //__colector_agregar(vm, nombre);
    //__colector_agregar(vm, cfun);    
}

const char* __obtener_bytecode_nombre(int inst){
    return bycode_nombre[inst];
}

lat_objeto* __lista_obtener_elemento(lista* list, int pos)
{
    if (pos < 0 || pos >= __lista_longitud(list))
    {
        lat_fatal_error("Indice fuera de rango");
    }
    int i = 0;
    LIST_FOREACH(list, primero, siguiente, cur) {
        if (i == pos)
        {
            return (lat_objeto *)cur->valor;
        }
        i++;
    }
    return NULL;
}

lista_nodo* __lista_obtener_nodo(lista* list, int pos)
{
    if (pos < 0 || pos >= __lista_longitud(list))
    {
        lat_fatal_error("Indice fuera de rango");
    }
    int i = 0;
    LIST_FOREACH(list, primero, siguiente, cur) {
        if (i == pos)
        {
            return cur;
        }
        i++;
    }
    return NULL;
}

lat_mv* lat_mv_crear()
{
    //printf("creando mv\n");
    lat_mv* mv = (lat_mv*)__memoria_asignar(sizeof(lat_mv));
    mv->memoria_usada = sizeof(lat_mv);
    mv->modulos = lat_lista_nueva(mv, __lista_crear());    
    mv->gc_objetos = lat_lista_nueva(mv, __lista_crear());    
    mv->pila = lat_lista_nueva(mv, __lista_crear());
    mv->objeto_verdadero = lat_logico_nuevo(mv, true);
    mv->objeto_falso = lat_logico_nuevo(mv, false);
    memset(mv->contexto_pila, 0, 256);
    mv->contexto_pila[0] = lat_contexto_nuevo(mv);
    mv->apuntador_ctx = 0;    

    /**
     * 10 Operadores
     * 20 funciones matematicas
     * 30 funciones para cadenas (string)
     * 40 entrada y salida
     * 50 conversion de tipos de dato
     * 60
     * 70
     * 99 otras funciones // a crear una categoria para ellas
     *
     */

    /*10 Operadores */

    /*20 funciones matematicas */    
    __registrar_cfuncion(mv, "arco_coseno", lat_arco_coseno, 1);
    __registrar_cfuncion(mv, "arco_seno", lat_arco_seno, 1);
    __registrar_cfuncion(mv, "arco_tangente", lat_arco_tangente, 1);
    __registrar_cfuncion(mv, "arco_tangente2", lat_arco_tangente2, 2);
    __registrar_cfuncion(mv, "coseno", lat_coseno, 1);
    __registrar_cfuncion(mv, "coseno_hiperbolico", lat_coseno_hiperbolico, 1);
    __registrar_cfuncion(mv, "seno", lat_seno, 1);
    __registrar_cfuncion(mv, "seno_hiperbolico", lat_seno_hiperbolico, 1);
    __registrar_cfuncion(mv, "tangente", lat_tangente, 1);
    __registrar_cfuncion(mv, "tangente_hiperbolica", lat_tangente_hiperbolica, 1);
    __registrar_cfuncion(mv, "exponente", lat_exponente, 1);
    __registrar_cfuncion(mv, "logaritmo_natural", lat_logaritmo_natural, 1);
    __registrar_cfuncion(mv, "logaritmo_base10", lat_logaritmo_base10, 1);
    __registrar_cfuncion(mv, "potencia", lat_potencia, 2);
    __registrar_cfuncion(mv, "raiz_cuadrada", lat_raiz_cuadrada, 1);
    __registrar_cfuncion(mv, "redondear_arriba", lat_redondear_arriba, 1);
    __registrar_cfuncion(mv, "valor_absoluto", lat_valor_absoluto, 1);
    __registrar_cfuncion(mv, "redondear_abajo", lat_redondear_abajo, 1);    
    __registrar_cfuncion(mv, "modulo", lat_modulo_decimal, 1);    

    /*30 funciones para cadenas (string)*/
    __registrar_cfuncion(mv, "comparar", lat_comparar, 2);
    __registrar_cfuncion(mv, "concatenar", lat_concatenar, 2);
    __registrar_cfuncion(mv, ".", lat_concatenar, 2);
    __registrar_cfuncion(mv, "contiene", lat_contiene, 2);
    __registrar_cfuncion(mv, "termina_con", lat_termina_con, 2);
    __registrar_cfuncion(mv, "es_igual", lat_es_igual, 2);
    __registrar_cfuncion(mv, "indice", lat_indice, 2);
    __registrar_cfuncion(mv, "insertar", lat_insertar, 3);
    __registrar_cfuncion(mv, "ultimo_indice", lat_ultimo_indice, 2);
    __registrar_cfuncion(mv, "rellenar_izquierda", lat_rellenar_izquierda, 3);
    __registrar_cfuncion(mv, "rellenar_derecha", lat_rellenar_derecha, 3);
    __registrar_cfuncion(mv, "eliminar", lat_eliminar, 2);
    __registrar_cfuncion(mv, "esta_vacia", lat_esta_vacia, 1);
    __registrar_cfuncion(mv, "longitud", lat_longitud, 1);
    __registrar_cfuncion(mv, "reemplazar", lat_reemplazar, 3);
    __registrar_cfuncion(mv, "empieza_con", lat_empieza_con, 2);
    __registrar_cfuncion(mv, "subcadena", lat_subcadena, 3);
    __registrar_cfuncion(mv, "minusculas", lat_minusculas, 1);
    __registrar_cfuncion(mv, "mayusculas", lat_mayusculas, 1);
    __registrar_cfuncion(mv, "quitar_espacios", lat_quitar_espacios, 1);
    __registrar_cfuncion(mv, "es_numerico", lat_es_numerico, 1);
    __registrar_cfuncion(mv, "es_alfanumerico", lat_es_alfanumerico, 1);
    __registrar_cfuncion(mv, "ejecutar", lat_ejecutar, 1);
    __registrar_cfuncion(mv, "ejecutar_archivo", lat_ejecutar_archivo, 1);

    /*40 entrada / salida */       
    __registrar_cfuncion(mv, "imprimir", lat_imprimir, 1);
    __registrar_cfuncion(mv, "escribir", lat_imprimir, 1);        
    __registrar_cfuncion(mv, "leer", lat_leer, 0);
    __registrar_cfuncion(mv, "leer_archivo", lat_leer_archivo, 1);
    __registrar_cfuncion(mv, "escribir_archivo", lat_escribir_archivo, 2);
    __registrar_cfuncion(mv, "salir", lat_salir, 0);

    /*50 conversion de tipos de dato*/
    __registrar_cfuncion(mv, "tipo", lat_tipo, 1);
    __registrar_cfuncion(mv, "logico", lat_logico, 1);
    __registrar_cfuncion(mv, "decimal", lat_decimal, 1);
    __registrar_cfuncion(mv, "cadena", lat_cadena, 1);
    
    /*60 funciones para listas*/
    __registrar_cfuncion(mv, "agregar", lat_agregar, 2);
    __registrar_cfuncion(mv, "extender", lat_extender, 2);
    __registrar_cfuncion(mv, "eliminar_indice", lat_eliminar_indice, 2);    
    __registrar_cfuncion(mv, "invertir", lat_invertir, 1);

    /*99 otras funciones */        
    __registrar_cfuncion(mv, "sistema", lat_sistema, 1);
    
#ifdef __linux__
    //__registrar_cfuncion(vm, "peticion", lat_peticion);
#endif
    return mv;
}

void lat_destruir_mv(lat_mv* mv){
    lat_eliminar_objeto(mv, mv->gc_objetos);
    lat_eliminar_objeto(mv, mv->pila);
    lat_eliminar_objeto(mv, mv->objeto_verdadero);
    lat_eliminar_objeto(mv, mv->objeto_falso); 
    /*lat_objeto* ctx = lat_obtener_contexto(mv);
    lat_eliminar_objeto(mv, ctx);
     mv->contexto_pila[0]
     */
    if(mv->contexto_pila[0] != NULL){
        lat_eliminar_objeto(mv, mv->contexto_pila[0]);        
    }
    __memoria_liberar(mv);
}

void lat_apilar(lat_mv* vm, lat_objeto* o)
{
    __lista_apilar(lat_obtener_lista(vm->pila), (void*)o);
}

lat_objeto* lat_desapilar(lat_mv* vm)
{
    return (lat_objeto*)__lista_desapilar(lat_obtener_lista(vm->pila));
}

lat_objeto* lat_tope(lat_mv* vm)
{
    return (lat_objeto*)vm->pila->datos.lista->ultimo->valor;
}

void lat_apilar_contexto(lat_mv* vm)
{
    //printf("apilando contexto...\n");
    if (vm->apuntador_ctx >= MAX_STACK_SIZE)
    {
        lat_fatal_error("Namespace desborde de la pila");
    }
    vm->contexto_pila[vm->apuntador_ctx + 1] = lat_clonar_objeto(vm, vm->contexto_pila[vm->apuntador_ctx]);
    vm->apuntador_ctx++;
}

void lat_desapilar_contexto(lat_mv* vm)
{
    //printf("...desapilando contexto\n");
    if (vm->apuntador_ctx == 0)
    {
        lat_fatal_error("Namespace pila vacia");
    }
    lat_eliminar_objeto(vm, vm->contexto_pila[vm->apuntador_ctx--]);
}

lat_objeto* lat_obtener_contexto(lat_mv* vm)
{
    return vm->contexto_pila[vm->apuntador_ctx];
}

lat_objeto* lat_definir_funcion(lat_mv* vm, lat_bytecode* inslist)
{
    lat_objeto* ret = lat_funcion_nueva(vm);
    lat_function* fval = (lat_function*)__memoria_asignar(sizeof(lat_function));
    fval->bcode = inslist;
    ret->datos.fun_usuario = fval;
    return ret;
}

lat_objeto* lat_definir_cfuncion(lat_mv* vm, void (*function)(lat_mv* vm))
{
    lat_objeto* ret = lat_cfuncion_nueva(vm);
    ret->datos.c_funcion = function;
    return ret;
}

void __imprimir_objeto(lat_mv* vm, lat_objeto* in)
{    
    char *tmp1 = NULL;
    if(in->tipo != T_STR){
        tmp1 = __objeto_a_cadena(in);
        fprintf(stdout, "%s", tmp1);
    }else{
        fprintf(stdout, "%s", lat_obtener_cadena(in));
    }    
    __memoria_liberar(tmp1);
}

void lat_imprimir(lat_mv* vm)
{
    lat_objeto* in = lat_desapilar(vm);
    __imprimir_objeto(vm, in);
    printf("\n");    
}

void __imprimir_lista(lat_mv* vm, lista* l)
{
    fprintf(stdout, "%s", __lista_a_cadena(l));
}

void lat_ejecutar(lat_mv *vm)
{
    int status;
    lat_objeto *func = nodo_analizar_arbol(vm, lat_analizar_expresion(lat_obtener_cadena(lat_desapilar(vm)), &status));
    lat_llamar_funcion(vm, func);    
}

void lat_ejecutar_archivo(lat_mv *vm)
{
    char *input = lat_obtener_cadena(lat_desapilar(vm));
    char *dot = strrchr(input, '.');
    char *extension;
    if (!dot || dot == input)
    {
        extension = "";
    }
    else
    {
        extension = dot + 1;
    }
    if (strcmp(extension, "lat") == 0)
    {
        int status;
        ast *tree = lat_analizar_archivo(input, &status);
        if (!tree)
        {
            lat_fatal_error("Error al leer el archivo: %s", input);
        }
        lat_objeto *func = nodo_analizar_arbol(vm, tree);
        lat_llamar_funcion(vm, func);        
    }
}

void lat_menos_unario(lat_mv* vm)
{
    lat_objeto* o = lat_tope(vm);
    o->datos.numerico = (-1) * lat_obtener_decimal(o);
}

void lat_sumar(lat_mv* vm)
{
    lat_objeto* b = lat_desapilar(vm);
    lat_objeto* a = lat_desapilar(vm);
    lat_objeto* r = lat_decimal_nuevo(vm, lat_obtener_decimal(a) + lat_obtener_decimal(b));
    lat_apilar(vm, r);
    __colector_agregar(vm, r);
}

void lat_restar(lat_mv* vm)
{
    lat_objeto* b = lat_desapilar(vm);
    lat_objeto* a = lat_desapilar(vm);
    lat_objeto* r = lat_decimal_nuevo(vm, lat_obtener_decimal(a) - lat_obtener_decimal(b));
    lat_apilar(vm, r);
    __colector_agregar(vm, r);
}

void lat_multiplicar(lat_mv* vm)
{
    lat_objeto* b = lat_desapilar(vm);
    lat_objeto* a = lat_desapilar(vm);
    lat_objeto* r = lat_decimal_nuevo(vm, lat_obtener_decimal(a) * lat_obtener_decimal(b));
    lat_apilar(vm, r);
    __colector_agregar(vm, r);
}

void lat_dividir(lat_mv* vm)
{
    lat_objeto* b = lat_desapilar(vm);
    lat_objeto* a = lat_desapilar(vm);
    if(lat_obtener_decimal(b) == 0){
        lat_fatal_error("Linea %d, %d: %s", b->num_linea, b->num_columna,  "Division entre cero");
    }
    lat_objeto* r = lat_decimal_nuevo(vm, (lat_obtener_decimal(a) / lat_obtener_decimal(b)));
    lat_apilar(vm, r);
    __colector_agregar(vm, r);
}

void lat_modulo_decimal(lat_mv *vm)
{
    lat_objeto *b = lat_desapilar(vm);
    lat_objeto *a = lat_desapilar(vm);
    lat_objeto* r = lat_decimal_nuevo(vm, fmod(lat_obtener_decimal(a), lat_obtener_decimal(b)));
    lat_apilar(vm, r);
    __colector_agregar(vm, r);
}

void lat_diferente(lat_mv* vm)
{
    lat_objeto* b = lat_desapilar(vm);
    lat_objeto* a = lat_desapilar(vm);
    lat_objeto* r = NULL;
    if (a->tipo == T_BOOL && b->tipo == T_BOOL)
    {
        r = lat_obtener_logico(a) != lat_obtener_logico(b) ? vm->objeto_verdadero : vm->objeto_falso;
        lat_apilar(vm, r);
        return;
    }
    if (b->tipo == T_NUMERIC && b->tipo == T_NUMERIC)
    {
        r = (lat_obtener_decimal(a) != lat_obtener_decimal(b)) ? vm->objeto_verdadero : vm->objeto_falso;
        lat_apilar(vm, r);
        return;
    }
    if (a->tipo == T_STR && b->tipo == T_STR)
    {
        r = strcmp(lat_obtener_cadena(a), lat_obtener_cadena(b)) != 0 ? vm->objeto_verdadero : vm->objeto_falso;
        lat_apilar(vm, r);
        return;
    }
    lat_apilar(vm, vm->objeto_falso);
}

void lat_igualdad(lat_mv* vm)
{
    lat_objeto* b = lat_desapilar(vm);
    lat_objeto* a = lat_desapilar(vm);
    lat_objeto* r = NULL;
    if (a->tipo == T_BOOL && b->tipo == T_BOOL)
    {
        r = lat_obtener_logico(a) == lat_obtener_logico(b) ? vm->objeto_verdadero : vm->objeto_falso;
        lat_apilar(vm, r);
        return;
    }
    if (a->tipo == T_NUMERIC && b->tipo == T_NUMERIC)
    {
        r = (lat_obtener_decimal(a) == lat_obtener_decimal(b)) ? vm->objeto_verdadero : vm->objeto_falso;
        lat_apilar(vm, r);
        return;
    }
    if (a->tipo == T_STR && b->tipo == T_STR)
    {
        r = strcmp(lat_obtener_cadena(a), lat_obtener_cadena(b)) == 0 ? vm->objeto_verdadero : vm->objeto_falso;
        lat_apilar(vm, r);
        return;
    }
    r= vm->objeto_falso;
    lat_apilar(vm, r);
}

void lat_menor_que(lat_mv* vm)
{
    lat_objeto* b = lat_desapilar(vm);
    lat_objeto* a = lat_desapilar(vm);
    lat_objeto* r = NULL;
    if (b->tipo == T_NUMERIC && b->tipo == T_NUMERIC)
    {
        r = (lat_obtener_decimal(a) < lat_obtener_decimal(b)) ? vm->objeto_verdadero : vm->objeto_falso;
        lat_apilar(vm, r);
        return;
    }
    if (a->tipo == T_STR && b->tipo == T_STR)
    {
        r = strcmp(lat_obtener_cadena(a), lat_obtener_cadena(b)) < 0 ? vm->objeto_verdadero : vm->objeto_falso;
        lat_apilar(vm, r);
        return;
    }
    lat_fatal_error("Linea %d, %d: %s", a->num_linea, a->num_columna,  "Intento de aplicar operador \"<\" en tipos invalidos");
}

void lat_menor_igual(lat_mv* vm)
{
    lat_objeto* b = lat_desapilar(vm);
    lat_objeto* a = lat_desapilar(vm);
    lat_objeto* r = NULL;
    if (b->tipo == T_NUMERIC && b->tipo == T_NUMERIC)
    {
        r = (lat_obtener_decimal(a) <= lat_obtener_decimal(b)) ? vm->objeto_verdadero : vm->objeto_falso;
        lat_apilar(vm, r);
        return;
    }
    if (a->tipo == T_STR && b->tipo == T_STR)
    {
        r = strcmp(lat_obtener_cadena(a), lat_obtener_cadena(b)) <= 0 ? vm->objeto_verdadero : vm->objeto_falso;
        lat_apilar(vm, r);
        return;
    }
    lat_fatal_error("Linea %d, %d: %s", a->num_linea, a->num_columna,  "Intento de aplicar operador \"<=\" en tipos invalidos");
}

void lat_mayor_que(lat_mv* vm)
{
    lat_objeto* b = lat_desapilar(vm);
    lat_objeto* a = lat_desapilar(vm);
    lat_objeto* r = NULL;
    if (b->tipo == T_NUMERIC && b->tipo == T_NUMERIC)
    {
        r = (lat_obtener_decimal(a) > lat_obtener_decimal(b)) ? vm->objeto_verdadero : vm->objeto_falso;
        lat_apilar(vm, r);
        return;
    }
    if (a->tipo == T_STR && b->tipo == T_STR)
    {
        r = strcmp(lat_obtener_cadena(a), lat_obtener_cadena(b)) > 0 ? vm->objeto_verdadero : vm->objeto_falso;
        lat_apilar(vm, r);
        return;
    }
    lat_fatal_error("Linea %d, %d: %s", a->num_linea, a->num_columna,  "Intento de aplicar operador \">\" en tipos invalidos");
}

void lat_mayor_igual(lat_mv* vm)
{
    lat_objeto* b = lat_desapilar(vm);
    lat_objeto* a = lat_desapilar(vm);
    lat_objeto* r = NULL;
    if (b->tipo == T_NUMERIC && b->tipo == T_NUMERIC)
    {
        r = (lat_obtener_decimal(a) >= lat_obtener_decimal(b)) ? vm->objeto_verdadero : vm->objeto_falso;
        lat_apilar(vm, r);
        return;
    }
    if (a->tipo == T_STR && b->tipo == T_STR)
    {
        r = strcmp(lat_obtener_cadena(a), lat_obtener_cadena(b)) >= 0 ? vm->objeto_verdadero : vm->objeto_falso;
        lat_apilar(vm, r);
        return;
    }
    lat_fatal_error("Linea %d, %d: %s", a->num_linea, a->num_columna,  "Intento de aplicar operador \">=\" en tipos invalidos");
}

void lat_y(lat_mv* vm)
{
    lat_objeto* b = lat_desapilar(vm);
    lat_objeto* a = lat_desapilar(vm);
    lat_objeto* r =  (lat_obtener_logico(a) && lat_obtener_logico(b)) == true ? vm->objeto_verdadero : vm->objeto_falso;
    lat_apilar(vm, r);
    __colector_agregar(vm, r);
}

void lat_o(lat_mv* vm)
{
    lat_objeto* b = lat_desapilar(vm);
    lat_objeto* a = lat_desapilar(vm);
    lat_objeto* r = (lat_obtener_logico(a) || lat_obtener_logico(b)) == true ? vm->objeto_verdadero : vm->objeto_falso;
    lat_apilar(vm, r);
    __colector_agregar(vm, r);
}

void lat_no(lat_mv* vm)
{
    lat_objeto* o = lat_desapilar(vm);
    lat_objeto* r = (lat_obtener_logico(o) == false) ? vm->objeto_verdadero : vm->objeto_falso;
    lat_apilar(vm, r);
    __colector_agregar(vm, r);
}

void lat_logico(lat_mv* vm)
{
    lat_objeto* a = lat_desapilar(vm);
    switch (a->tipo)
    {
    case T_NUMERIC:
        if ((int)a->datos.numerico == 0)
        {
            lat_apilar(vm, vm->objeto_falso);
        }
        else
        {
            lat_apilar(vm, vm->objeto_verdadero);
        }
        break;
    case T_STR:
        if (strcmp(a->datos.cadena, "") == 0)
        {
            lat_apilar(vm, vm->objeto_falso);
        }
        else
        {
            lat_apilar(vm, vm->objeto_verdadero);
        }
        break;
    default:
        lat_fatal_error("Linea %d, %d: %s", a->num_linea, a->num_columna,  
                "Conversion de tipo de dato incompatible");
        break;
    }
}

void lat_decimal(lat_mv* vm)
{
    lat_objeto* a = lat_desapilar(vm);
    switch (a->tipo)
    {
    case T_BOOL:
        if (a->datos.logico == false)
        {
            lat_apilar(vm, lat_decimal_nuevo(vm, 0));
        }
        else
        {
            lat_apilar(vm, lat_decimal_nuevo(vm, 1));
        }
        break;    
    case T_NUMERIC:
        lat_apilar(vm, lat_decimal_nuevo(vm, a->datos.numerico));
        break;
    case T_STR:
    {
        char *ptr;
        double ret;
        ret = strtod(a->datos.cadena, &ptr);
        if (strcmp(ptr, "") == 0)
        {
            lat_apilar(vm, lat_decimal_nuevo(vm, ret));
        }
        else
        {
            lat_fatal_error("Linea %d, %d: %s", a->num_linea, a->num_columna,  
                    "Conversion de tipo de dato incompatible");
        }
    }
    break;
    default:
       lat_fatal_error("Linea %d, %d: %s", a->num_linea, a->num_columna,  
               "Conversion de tipo de dato incompatible");
        break;
    }
}

void lat_cadena(lat_mv* vm)
{
    lat_objeto* a = lat_desapilar(vm);
    switch (a->tipo)
    {
    case T_BOOL:
        lat_apilar(vm, lat_cadena_nueva(vm, __str_logico_a_cadena(a->datos.logico)));
        break;    
    case T_NUMERIC:
        lat_apilar(vm, lat_cadena_nueva(vm, __str_decimal_a_cadena(a->datos.numerico)));
        break;
    default:
        lat_apilar(vm, a);
        break;
    }
}

char* __tipo(int tipo){
    switch (tipo)
    {
    case T_BOOL:
        return "logico";
        break;    
    case T_NUMERIC:
        return "decimal";
        break;
    case T_STR:
        return "cadena";
        break;    
    case T_LIST:
        return "lista";
        break;
    case T_DICT:
        return "diccionario";
        break;
    default:
        return "nulo";
        break;
    }
}

void lat_tipo(lat_mv* vm)
{
    lat_objeto* a = lat_desapilar(vm);
    lat_apilar(vm, lat_cadena_nueva(vm, __tipo(a->tipo)));
}

void lat_salir(lat_mv* vm)
{
    //lat_apilar(vm, lat_entero_nuevo(vm, 0L));
    exit(0);
}

lat_bytecode lat_bc(int i, int a, int b, void* meta)
{
    lat_bytecode ret;
    ret.ins = i;
    ret.a = a;
    ret.b = b;
    ret.meta = meta;
    return ret;
}

void lat_llamar_funcion(lat_mv* vm, lat_objeto* func)
{
    if (func->tipo == T_FUNC)
    {
#if DEPURAR_MV
        printf("\n.::Ejecutando funcion::.\n");
#endif
        lat_asignar_contexto_objeto(lat_obtener_contexto(vm), lat_cadena_nueva(vm, "lat_main"), func);
        lat_bytecode* inslist = ((lat_function*)func->datos.fun_usuario)->bcode;
        lat_bytecode cur;
        int pos;
        for (pos = 0, cur = inslist[pos]; cur.ins != HALT; cur = inslist[++pos])
        {
#if DEPURAR_MV
            printf("%i\t", pos);
            printf("%s\t", __obtener_bytecode_nombre(cur.ins));            
#endif
            switch (cur.ins)
            {
            case HALT:
                return;
                break;
            case NOP:
                break;
            case UNARY_MINUS:{
                lat_menos_unario(vm);
            }
            break;
            case INC:{
                lat_objeto *name =  (lat_objeto*)cur.meta;
                lat_objeto *ctx =  lat_obtener_contexto(vm);
                lat_objeto *val = lat_obtener_contexto_objeto(ctx, name);
                lat_objeto *tmp = lat_clonar_objeto(vm, val);
                tmp->datos.numerico++;
                lat_asignar_contexto_objeto(ctx, name, tmp);
                __colector_agregar(vm, tmp);
            }
            break;
            case DEC:{
                lat_objeto *name =  (lat_objeto*)cur.meta;
                lat_objeto *ctx =  lat_obtener_contexto(vm);
                lat_objeto *val = lat_obtener_contexto_objeto(ctx, name);
                lat_objeto *tmp = lat_clonar_objeto(vm, val);
                tmp->datos.numerico--;
                lat_asignar_contexto_objeto(ctx, name, tmp);
                __colector_agregar(vm, tmp);
            }
            break;
            case BINARY_ADD:{
                lat_sumar(vm);
            }
            break;
            case BINARY_SUB:{
                lat_restar(vm);
            }
            break;
            case BINARY_MUL:{
                lat_multiplicar(vm);
            }
            break;
            case BINARY_DIV:{
                lat_dividir(vm);
            }
            break;
            case BINARY_MOD:{
                lat_modulo_decimal(vm);
            }
            break;
            case OP_GT:{
                lat_mayor_que(vm);
            }
            break;
            case OP_GE:{
                lat_mayor_igual(vm);
            }
            break;
            case OP_LT:{
                lat_menor_que(vm);
            }
            break;
            case OP_LE:{
                lat_menor_igual(vm);
            }
            break;
            case OP_EQ:{
                lat_igualdad(vm);
            }
            break;
            case OP_NEQ:{
                lat_diferente(vm);
            }
            break;
            case OP_AND:{
                lat_y(vm);
            }
            break;
            case OP_OR:{
                lat_o(vm);
            }
            break;
            case OP_NOT:{
                lat_no(vm);
            }
            break;
            case CONCAT:{
                lat_concatenar(vm);
            }
            break;
            case LOAD_CONST:
            {                
                lat_objeto *o = (lat_objeto*)cur.meta;
#if DEPURAR_MV
                __imprimir_objeto(vm, o);
                printf("\t");
#endif
                lat_apilar(vm, o);
            }
                break;
            case STORE_NAME:{
                lat_objeto *val = lat_desapilar(vm);
                lat_objeto *name =  (lat_objeto*)cur.meta;
                lat_objeto *ctx =  lat_obtener_contexto(vm);
#if DEPURAR_MV
                __imprimir_objeto(vm, name);
                printf("\t");
#endif
                if(name->es_constante){
                    lat_objeto *tmp = lat_obtener_contexto_objeto(ctx, name);
                    if(tmp != NULL){
                        lat_fatal_error("Linea %d, %d: Intento de reasignar valor a constante '%s'", 
                                name->num_linea, name->num_columna, lat_obtener_cadena(name));
                    }
                }
                //asigna el numero de parametros
                if(name->nombre_cfun){
                    val->num_params = name->num_params;
                    val->nombre_cfun = name->nombre_cfun;                    
                }
                lat_asignar_contexto_objeto(ctx, name, val);
            }
                break;
            case LOAD_NAME:{
                lat_objeto *name =  (lat_objeto*)cur.meta;
                lat_objeto *ctx =  lat_obtener_contexto(vm);
#if DEPURAR_MV
                __imprimir_objeto(vm, name);
                printf("\t");
#endif
                lat_objeto *val = lat_obtener_contexto_objeto(ctx, name);
                if(val == NULL){
                    lat_fatal_error("Linea %d, %d: Variable \"%s\" indefinida", 
                            name->num_linea, name->num_columna, lat_obtener_cadena(name));                        
                }
                val->num_linea = name->num_linea;
                val->num_columna = name->num_columna;
                lat_apilar(vm, val);
            }
                break;
            case POP_JUMP_IF_FALSE:{
                lat_objeto *o = lat_desapilar(vm);
                if(lat_obtener_logico(o) == false){
                    pos = cur.a;
                }
            }
                break;
                case POP_JUMP_IF_TRUE:{
                lat_objeto *o = lat_desapilar(vm);
                if(lat_obtener_logico(o) == true){
                    pos = cur.a;
                }
            }
                break;
            case JUMP_ABSOLUTE:
                pos = cur.a;
                break;
            case CALL_FUNCTION: {          
#if DEPURAR_MV
                printf("\n=> ");
#endif
                int num_args = cur.a;
                lat_objeto *fun = lat_desapilar(vm);                
                if(num_args != fun->num_params ){
                    lat_fatal_error("Linea %d, %d: Numero invalido de argumentos en funcion '%s'. se esperaban %i valores.\n", 
                            fun->num_linea, fun->num_columna, fun->nombre_cfun, fun->num_params);
                }                
                lat_apilar_contexto(vm);
                vm->num_callf++;                
                if(vm->num_callf >= MAX_CALL_FUNCTION){
                    lat_fatal_error("Linea %d, %d: Numero maximo de llamadas a funciones recursivas excedido en '%s'\n", 
                            fun->num_linea, fun->num_columna, fun->nombre_cfun);
                }
                lat_llamar_funcion(vm, fun);
                vm->num_callf--;
                lat_desapilar_contexto(vm);
            }
            break;
            case RETURN_VALUE: {
                return;                
            }
            break;
            case MAKE_FUNCTION: {
                lat_objeto *fun = lat_definir_funcion(vm, (lat_bytecode*)cur.meta);
                lat_apilar(vm, fun);
            }
            break; 
            case SETUP_LOOP: 
                //lat_apilar_contexto(vm); 
                break; 
            case POP_BLOCK: 
                //lat_desapilar_contexto(vm); 
                break;                
            case BUILD_LIST: {
                int num_elem = cur.a;
                int i;
                lista* l = __lista_crear();                
                for(i=0; i < num_elem; i++){
                    lat_objeto* tmp = lat_desapilar(vm);
                    __lista_apilar(l, tmp);
                }
                lat_objeto* obj = lat_lista_nueva(vm, l);
                lat_apilar(vm, obj);
            }
            break; 
            case LOAD_ATTR: {                
                lat_objeto *attr = lat_desapilar(vm);
                lat_objeto *name =  (lat_objeto*)cur.meta;
                lat_objeto *ctx =  lat_obtener_contexto(vm);
#if DEPURAR_MV
                __imprimir_objeto(vm, name);
                printf("\t");
#endif
                lat_objeto *val = lat_obtener_contexto_objeto(ctx, name);
                if(val == NULL){
                    lat_fatal_error("Linea %d, %d: Objeto \"%s\" no tiene un atributo \"%s\" definido. ", 
                            name->num_linea, name->num_columna, __tipo(attr->tipo), lat_obtener_cadena(name));
                }
                val->num_linea = name->num_linea;
                val->num_columna = name->num_columna;                 
                lista* list = __lista_crear();
                int i;
                for(i=0; i < val->num_params-1; i++){
                    __lista_insertar_inicio(list, (void*)lat_desapilar(vm));
                }                
                lat_apilar(vm, attr);
                LIST_FOREACH(list, primero, siguiente, cur){
                    lat_apilar(vm, (lat_objeto*)cur->valor);                    
                }
                lat_apilar(vm, val);
                __memoria_liberar(list);
            }
            break;
            case STORE_SUBSCR:{
                lat_objeto* pos = lat_desapilar(vm);                
                lat_objeto* lst = lat_desapilar(vm);
                lat_objeto* exp = lat_desapilar(vm);
                int ipos = lat_obtener_decimal(pos);
                if(lst->tipo == T_STR){
                    char* slst = lat_obtener_cadena(lst);
                    if(ipos < 0 || ipos >= strlen(slst)){
                        lat_fatal_error("Linea %d, %d: Indice fuera de rango.", 
                                pos->num_linea, pos->num_columna);
                    }   
                    char* sexp = __str_duplicar(lat_obtener_cadena(exp));
                    if(strlen(sexp) == 0){
                        sexp = " ";
                    }
                    slst[ipos] = sexp[0];
                    lst->datos.cadena = slst;
                }
                if(lst->tipo == T_LIST){
                    if(ipos < 0 || ipos >= __lista_longitud(lat_obtener_lista(lst))){
                        lat_fatal_error("Linea %d, %d: Indice fuera de rango.", 
                                pos->num_linea, pos->num_columna);
                    }
                    __lista_modificar_elemento(lat_obtener_lista(lst), exp, ipos);
                }
            }
            break;            
            case BINARY_SUBSCR:{                
                lat_objeto* lst = lat_desapilar(vm);
                lat_objeto* pos = lat_desapilar(vm);
                int ipos = lat_obtener_decimal(pos);
                lat_objeto* o = NULL;
                if(lst->tipo == T_STR){
                    char* slst = lat_obtener_cadena(lst);
                    if(ipos < 0 || ipos >= strlen(slst)){
                        lat_fatal_error("Linea %d, %d: Indice fuera de rango.", 
                                pos->num_linea, pos->num_columna);
                    }
                    char c[2] = {slst[ipos], '\0' };
                    o = lat_cadena_nueva(vm, c);
                }
                if(lst->tipo == T_LIST){
                    if(ipos < 0 || ipos >= __lista_longitud(lat_obtener_lista(lst))){
                        lat_fatal_error("Linea %d, %d: Indice fuera de rango.", 
                                pos->num_linea, pos->num_columna);
                    }
                    o = __lista_obtener_elemento(lat_obtener_lista(lst), lat_obtener_decimal(pos));
                }
                lat_apilar(vm, o);
            }
            break;                    
            

            }   //fin de switch
            
#if DEPURAR_MV            
            __imprimir_lista(vm, lat_obtener_lista(vm->pila));
            printf("\n");
#endif            
        }   //fin for
    }   //fin if (T_FUNC)
    else if (func->tipo == T_CFUNC)
    {
        ((void (*)(lat_mv*))(func->datos.fun_usuario))(vm);
    }
    else
    {
        lat_fatal_error("Linea %d, %d: %s", func->num_linea, func->num_columna,  "El objeto no es una funcion");
    }
}


/* funciones para listas */
void lat_agregar(lat_mv *vm){    
    lat_objeto* elem = lat_desapilar(vm);
    lat_objeto* lst = lat_desapilar(vm);
    __lista_apilar(lat_obtener_lista(lst), elem);
}

void lat_extender(lat_mv *vm){    
    lat_objeto* lst2 = lat_desapilar(vm);
    lat_objeto* lst = lat_desapilar(vm);    
    if(lst->tipo != T_LIST){
        lat_fatal_error("Linea %d, %d: %s", lst->num_linea, lst->num_columna,  "El objeto no es una lista");
    }    
    if(lst2->tipo != T_LIST){
        lat_fatal_error("Linea %d, %d: %s", lst2->num_linea, lst2->num_columna,  "El objeto no es una lista");
    }    
    lista* _lst2 = lat_obtener_lista(lst2);
    lista* _lst = lat_obtener_lista(lst);
    __lista_extender(_lst, _lst2);    
}

int __lista_obtener_indice(lista* list, void* data){
    int i=0;
    lat_objeto* find = (lat_objeto*) data;
    LIST_FOREACH(list, primero, siguiente, cur) {
        //if (memcmp(cur->valor, data, sizeof(cur->valor)) == 0)
        lat_objeto* tmp = (lat_objeto*)cur->valor;
        if (__es_igual(find, tmp))
        {
            return i;
        }
        i++;
    }
    return -1;
}

void __lista_insertar_elemento(lista* list, void* data, int pos){
    int len = __lista_longitud(list);
    if (pos < 0 || pos > len)   //permite insertar al ultimo
    {
        lat_fatal_error("Indice fuera de rango");
    }
    if(pos == 0){
        __lista_insertar_inicio(list, data);        
        return;
    }
    if(pos == len){
        __lista_apilar(list, data);        
        return;
    }
    //FIX: For performance
    lista* tmp1 = __lista_crear();
    lista* tmp2 = __lista_crear();
    int i=0;
    LIST_FOREACH(list, primero, siguiente, cur) {
        if (i < pos)
        {
            __lista_apilar(tmp1, cur->valor);
        }else{
            __lista_apilar(tmp2, cur->valor);
        }
        i++;
    }    
    lista* new = __lista_crear();
    __lista_extender(new, tmp1);
    __lista_apilar(new, data);
    __lista_extender(new, tmp2);
    *list = *new;
    __memoria_liberar(tmp1);
    __memoria_liberar(tmp2);    
}

void lat_eliminar_indice(lat_mv* vm)
{
    lat_objeto* b = lat_desapilar(vm);
    lat_objeto* a = lat_desapilar(vm);
    lista* lst = lat_obtener_lista(a);
    int pos = lat_obtener_decimal(b);
    if (pos < 0 || pos >= __lista_longitud(lst))
    {
        lat_fatal_error("Linea %d, %d: %s", a->num_linea, a->num_columna,  "Indice fuera de rango");        
    }
    if(pos >= 0){
        lista_nodo *nt = __lista_obtener_nodo(lst, pos);
        __lista_eliminar_elemento(lst, nt);
    }    
}

void lat_invertir(lat_mv* vm){
    lat_objeto* a = lat_desapilar(vm);
    lista* lst = lat_obtener_lista(a);
    lista* new = __lista_crear();
    //FIX: For performance
    int i;
    int len = __lista_longitud(lst)-1;
    for(i=len; i >= 0; i--){
        __lista_apilar(new, __lista_obtener_elemento(lst, i));
    }
    lat_apilar(vm, lat_lista_nueva(vm, new));
    
}