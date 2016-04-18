#include "hash.h"
#include <stdlib.h>


#define TAM_INICIAL_HASH

//NO REDIMENSIONAR CON REALLOC!!!!!! 


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


// Funciones auxiliares.

// Empieza buscando la clave en donde debería estar según la función de hashing
// Si no está, va avanzando de a una posición hasta encontrarla. Reemplaza el dato por el nuevo.
void aux_reemplazar_dato(hash_t *hash, const char *clave_nueva, void *dato_nuevo, size_t posicion) {
	while (hash->vector[posicion]->clave != clave_nueva) {
		posicion++;
	}
	hash->vector[posicion]->dato = dato;
}

void aux_redimensionar(hash_t *hash);
size_t aux_hashear_posicion(const char *clave);
size_t aux_buscar_posicion_alternativa(hash, size_t posicion_original);

// Primitivas de hash.


hash_t *hash_crear(hash_destruir_dato_t destruir_dato) {
	hash_t* hash = malloc(sizeof(hash_t));
	if (!hash) return NULL;
	hash->vector = calloc(sizeof((nodo_hash_t*) * TAM_INICIAL_HASH);
	if (!hash->vector) {
		free(hash);
		return NULL;
	}
	hash->posiciones_totales = TAM_INICIAL_HASH;
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
	nodo_hash_t* nodo_a_guardar = malloc(sizeof(nodo_hash_t));
	if (!nodo_a_guardar) return NULL;
	nodo_a_guardar->clave = clave;
	nodo_a_guardar->dato = dato;
	nodo_a_guardar = borrado = false;
	size_t posicion = aux_hashear_posicion(clave); // Función de hashing.
	if (hash_pertenece(hash, clave)) {
		aux_reemplazar_dato(hash, clave, dato, posicion); // Si la clave ya existía, busco la posición y reemplazo el dato.
		return true;
	}
	if (hash->vector[posicion] == NULL) {
		hash->vector[posicion]->clave = clave;
		hash->vector[posicion]->dato = dato;
	} else { // Colisión
		size_t posicion_alternativa = aux_buscar_posicion_alternativa(hash, posicion);
		hash->vector[posicion_alternativa]->clave = clave;
		hash->vector[posicion_alternativa]->dato = dato;
	}
	hash->posiciones_ocupadas++
	if ((float)hash->posiciones_ocupadas/(float)hash->posiciones_totales >= 0.7) 
		aux_redimensionar(hash);
	return true;
}


/* Determina si clave pertenece o no al hash.
 * Pre: La estructura hash fue inicializada
 */
bool hash_pertenece(const hash_t *hash, const char *clave);


size_t hash_cantidad(const hash_t *hash) {
	return hash->posiciones_ocupadas;
}