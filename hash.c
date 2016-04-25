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
	nodo_hash_t** vector;
	nodo_hash_t* actual;
	int pos;
};


// Funciones auxiliares.

// Función de hashing multiplicativa extraida del "Kernighan and Ritchie"
size_t aux_hashear_posicion(const char *clave) {
		size_t len = strlen(clave);
		unsigned int hash = 0;
    for(unsigned int i = 0; i < len; i++) {
    	hash = 31 * hash + clave[i];
    }
    return hash % TAM_HASH;
}


// Empieza buscando la clave en donde debería estar según la función de hashing
// Si no está, va avanzando de a una posición hasta encontrarla. Devuelve la posición real.
// Si llega a un lugar vacío antes de encontrarla, devuelve la posición de ese lugar vacío.
size_t aux_encontrar_posicion(const hash_t *hash, const char *clave) {
	size_t posicion = aux_hashear_posicion(clave);
	//printf("la posicion de hash es: %zu\n", posicion);
	while (hash->vector[posicion] != NULL) {
		if (strcmp(hash->vector[posicion]->clave, clave) == 0 && !hash->vector[posicion]->borrado)
			break;
		posicion++;
		if (posicion == hash->posiciones_ocupadas) posicion = 0;
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
			char* copia_clave = strdup(aux[i]->clave);
			void* copia_dato = aux[i]->dato;
			if (hash->destruir_dato) {
				void* dato_a_liberar = aux[i]->dato;
				hash->destruir_dato(dato_a_liberar);
			}

			hash_guardar(hash, copia_clave, copia_dato);
			//size_t posicion = aux_encontrar_posicion(hash, copia_clave);
			//nodo_hash_t* nodo_a_guardar = malloc(sizeof(nodo_hash_t));
			//nodo_a_guardar->clave = copia_clave;
			//nodo_a_guardar->dato = copia_dato;
			//nodo_a_guardar->borrado = false;
			//hash->vector[posicion] = nodo_a_guardar; 
			//hash->vector[posicion]->clave = copia_clave;
			//hash->vector[posicion]->dato = copia_dato;
			free(aux[i]);		
		}
	}
	free(aux);
	hash->posiciones_totales = tam;
}




// Primitivas de hash.


hash_t *hash_crear(hash_destruir_dato_t destruir_dato) {
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
    //printf("La posicion de %s es.....................: %zu\n", clave, aux_hashear_posicion(clave));
	nodo_hash_t* nodo_a_guardar = malloc(sizeof(nodo_hash_t));
	if (!nodo_a_guardar) return NULL;
	nodo_a_guardar->clave = clave;
	nodo_a_guardar->dato = dato;
	nodo_a_guardar->borrado = false;
	size_t posicion = aux_encontrar_posicion(hash, clave);
    /*printf("La REAL de %s es.....................: %zu\n", clave, posicion);
	if (posicion != aux_hashear_posicion(clave)) {
		printf("COLISIONNNNNN !!!!!!!!!!!!\n");
		colisiones++;
	}*/
	if (hash_pertenece(hash, clave)) {
		if (hash->destruir_dato) {
			void* dato_a_liberar = hash->vector[posicion]->dato;
			hash->destruir_dato(dato_a_liberar);
			//printf("holuu\n");
		}
		hash->vector[posicion]->dato = dato; // Reemplazo el dato
		return true;
	}
	hash->vector[posicion] = nodo_a_guardar;
	hash->posiciones_ocupadas++;
	if ((float)hash->posiciones_ocupadas/(float)hash->posiciones_totales >= 0.7){ 
		aux_redimensionar(hash, hash->posiciones_ocupadas * 2);
	}	
	return true;
}


void* hash_borrar(hash_t* hash, const char* clave){

	if (!hash_pertenece(hash, clave) || clave == NULL) return NULL;
	size_t posicion = aux_encontrar_posicion(hash,clave);
	void* dato = hash->vector[posicion]->dato;
	if (hash->destruir_dato) {
		hash->destruir_dato(dato);
	}
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
	//printf("la posicion real va a ser: %zu\n", posicion);
	if (hash->vector[posicion] == NULL) return false;
	return true;
}


size_t hash_cantidad(const hash_t *hash) {
	return hash->posiciones_ocupadas;
}


void hash_destruir(hash_t *hash){

	for (int i = 0; i < hash->posiciones_totales; i++){
		if (hash->vector[i] != NULL){
			if (hash->destruir_dato) {
				void* dato_a_liberar = hash->vector[i]->dato;
				hash->destruir_dato(dato_a_liberar);
			}
			free(hash->vector[i]);
		}
	}
	free(hash->vector);
	free(hash);
}


hash_iter_t* hash_iter_crear(const hash_t* hash){

	hash_iter_t* iter = malloc(sizeof(hash_iter_t));
	if(!iter) return NULL;
	iter->vector = hash->vector;
	int act = 0;

	while (hash->vector[act] == NULL && act < TAM_HASH){
		act++;
	}
	if (act == TAM_HASH){
		iter->pos = -1;
		iter->actual = NULL;
		return iter;
	}

	iter->pos = act;
	iter->actual = hash->vector[act];
	return iter;
}


bool hash_iter_avanzar (hash_iter_t* iter){

	if(hash_iter_al_final(iter) || iter->pos > TAM_HASH) return false;
	int act = 0;
	while(iter->vector[act] == NULL){
		act++;
	}

	iter->actual = iter->vector[act];
	return true;
}


const char* hash_iter_ver_actual(const hash_iter_t *iter){

	if(hash_iter_al_final(iter)) return NULL;

	return iter->actual->clave;
}

bool hash_iter_al_final(const hash_iter_t *iter){

	if (iter->actual == NULL && iter->pos == -1){
		return true;
	}
	return false;
}


void hash_iter_destruir(hash_iter_t* iter){
	free(iter);
}
