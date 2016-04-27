#include "hash.h"
#include <stdlib.h>
#include <string.h>

#include <stdio.h>
#include "testing.h"

#define TAM_HASH 104
int colisiones = 0;//NO REDIMENSIONAR CON REALLOC!!!!!! 


// Definción de estructuras.


typedef struct nodo_hash {
	const char* clave;
	void* dato;
	bool borrado;
} nodo_hash_t;


struct hash {
	nodo_hash_t** vector;
	size_t posiciones_totales;
	size_t posiciones_ocupadas; // También es la cantidad de elementos, ya que es un hash cerrado
	hash_destruir_dato_t destruir_dato;
};


struct hash_iter {
	const hash_t* hash;
	int pos;
};


// Funciones auxiliares.

// Función de hashing multiplicativa extraida del "Kernighan and Ritchie"
size_t aux_hashear_posicion(const char *clave, const hash_t *hash) {
		size_t len = strlen(clave);
		unsigned int n = 0;
    for(unsigned int i = 0; i < len; i++) {
    	n = 31 * n + clave[i];
    }
    return n % hash->posiciones_totales;
}


// Empieza buscando la clave en donde debería estar según la función de hashing
// Si no está, va avanzando de a una posición hasta encontrarla. Devuelve la posición real.
// Si llega a un lugar vacío antes de encontrarla, devuelve la posición de ese lugar vacío.
size_t aux_encontrar_posicion(const hash_t *hash, const char *clave) {
	size_t posicion = aux_hashear_posicion(clave, hash);
	//printf("la posicion de hash es: %zu\n", posicion);
	while (hash->vector[posicion] != NULL) {
		if (strcmp(hash->vector[posicion]->clave, clave) == 0 && !hash->vector[posicion]->borrado)
			break;
		posicion++;
		if (posicion == hash->posiciones_totales) posicion = 0;
	}
	return posicion;
}


char* strdup(const char *old) {
	char *new;
	if((new = malloc(strlen(old) + 1)) == NULL)
	return NULL;
	strcpy(new, old);
	return new;
}


void aux_redimensionar(hash_t *hash, size_t tam){

	size_t aux3 = hash->posiciones_totales;
	nodo_hash_t** vec_nuevo = calloc(tam, sizeof(nodo_hash_t*));
	nodo_hash_t** aux = hash->vector;
	hash->vector = vec_nuevo; 
	hash->posiciones_ocupadas = 0;
	hash->posiciones_totales = tam;

	for (int i = 0; i < aux3; i++){
		if (aux[i] != NULL && aux[i]->borrado == false){
			const char* copia_clave = aux[i]->clave;
			void* copia_dato = aux[i]->dato;
			/*if (hash->destruir_dato) {
				void* dato_a_liberar = aux[i]->dato;
				hash->destruir_dato(dato_a_liberar);
			}*/

			hash_guardar(hash, copia_clave, copia_dato);
		}
		//free(aux[i]);
	}
	free(aux);
}

nodo_hash_t* aux_nodo_crear(const char* clave, void* dato){

	nodo_hash_t* nodo = malloc(sizeof(nodo_hash_t));
	if (!nodo) return NULL;
	nodo->clave = strdup(clave);
	nodo->dato = dato;
	nodo->borrado = false;
	return nodo;
}


// Primitivas de hash.


hash_t* hash_crear(hash_destruir_dato_t destruir_dato) {
	hash_t* hash = malloc(sizeof(hash_t));
	if (!hash) return NULL;
	hash->vector = calloc(TAM_HASH, sizeof(nodo_hash_t*));
	if (!hash->vector) {
		free(hash);
		return NULL;
	}
	hash->posiciones_totales = TAM_HASH;
	hash->posiciones_ocupadas = 0;
	hash->destruir_dato = destruir_dato;
	return hash;
}


/* Guarda un elemento en el hash, si la clave ya se encuentra en la
 * estructura, la reemplaza. De no poder guardarlo devuelve false.
 * Pre: La estructura hash fue inicializada
 * Post: Se almacenó el par (clave, dato)
 */
bool hash_guardar(hash_t *hash, const char *clave, void *dato) {
	
	nodo_hash_t* nodo_a_guardar = aux_nodo_crear(clave, dato);
	size_t posicion = aux_encontrar_posicion(hash, clave);
	if (hash_pertenece(hash, clave)) {
		if (hash->destruir_dato) {
			void* dato_a_liberar = hash->vector[posicion]->dato;
			hash->destruir_dato(dato_a_liberar);
		}
		hash->vector[posicion]->dato = dato; // Reemplazo el dato
		return true;
	}
	hash->vector[posicion] = nodo_a_guardar;
	hash->posiciones_ocupadas++;
	if ((float)hash->posiciones_ocupadas/(float)hash->posiciones_totales >= 0.7){ 
		aux_redimensionar(hash, hash->posiciones_totales * 2);
	}	
	return true;
}


void* hash_borrar(hash_t* hash, const char* clave) {
	if (!hash_pertenece(hash, clave) || clave == NULL) return NULL;
	size_t posicion = aux_encontrar_posicion(hash,clave);
	void* dato = hash->vector[posicion]->dato;
	if (hash->destruir_dato) {
		hash->destruir_dato(dato);
	}
	//free((char*)hash->vector[posicion]->clave);
	hash->vector[posicion]->borrado = true;
	hash->posiciones_ocupadas--;
	return dato;
}


void* hash_obtener(const hash_t *hash, const char *clave) {
	if (!hash_pertenece(hash, clave)) return NULL;
	size_t posicion_real = aux_encontrar_posicion(hash, clave);
	return hash->vector[posicion_real]->dato;
}


bool hash_pertenece(const hash_t *hash, const char *clave) {
	size_t posicion = aux_encontrar_posicion(hash, clave);
	if (hash->vector[posicion] == NULL) return false;
	return true;
}


size_t hash_cantidad(const hash_t *hash) {
	return hash->posiciones_ocupadas;
}


void hash_destruir(hash_t *hash){

	for (int i = 0; i < hash->posiciones_totales; i++){
		if (hash->vector[i] != NULL){
			if (hash->destruir_dato && !hash->vector[i]->borrado) {
				void* dato_a_liberar = hash->vector[i]->dato;
				hash->destruir_dato(dato_a_liberar);
			}
			//free((char*)hash->vector[i]->clave);
			free(hash->vector[i]);
		}
	}
	free(hash->vector);
	free(hash);
}


hash_iter_t* hash_iter_crear(const hash_t* hash){

	hash_iter_t* iter = malloc(sizeof(hash_iter_t));
	if(!iter) return NULL;

	iter->hash = hash;
	int act = 0;
	while (iter->hash->vector[act] == NULL && act < iter->hash->posiciones_totales){
		act++;
	}
	if (act == iter->hash->posiciones_totales){
		iter->pos = -1;
		return iter;
	}
	iter->pos = act;
	return iter;
}


bool hash_iter_avanzar (hash_iter_t* iter){

	if(hash_iter_al_final(iter)) return false;
	
	int act = iter->pos +1;
	while(iter->hash->vector[act] == NULL  && act < iter->hash->posiciones_totales){
		act++;	
	}

	if(act == iter->hash->posiciones_totales){
		iter->pos = act + 1;
		return false;
	}

	iter->pos = act;
	return true;
}


const char* hash_iter_ver_actual(const hash_iter_t *iter){

	int act = iter->pos;
	if(hash_iter_al_final(iter) || iter->pos > iter->hash->posiciones_totales) return NULL;

	return iter->hash->vector[act]->clave;
}

bool hash_iter_al_final(const hash_iter_t *iter){

	if (iter->pos > iter->hash->posiciones_totales){
		return true;
	}
	return false;
}


void hash_iter_destruir(hash_iter_t* iter){
	free(iter);
}