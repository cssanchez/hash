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
	while (hash->vector[posicion] != NULL && strcmp(hash->vector[posicion]->clave, clave) != 0) {
		posicion++;
		if (posicion == TAM_HASH) posicion = 0;
	}
	return posicion;
}

void aux_redimensionar(hash_t *hash);


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
    printf("La posicion de %s es.....................: %zu\n", clave, aux_hashear_posicion(clave));
	nodo_hash_t* nodo_a_guardar = malloc(sizeof(nodo_hash_t));
	if (!nodo_a_guardar) return NULL;
	nodo_a_guardar->clave = clave;
	nodo_a_guardar->dato = dato;
	nodo_a_guardar->borrado = false;
	size_t posicion = aux_encontrar_posicion(hash, clave);
    printf("La REAL de %s es.....................: %zu\n", clave, posicion);
	if (posicion != aux_hashear_posicion(clave)) {
		printf("COLISIONNNNNN !!!!!!!!!!!!\n");
		colisiones++;
	}
	if (hash_pertenece(hash, clave)) {
		if (hash->destruir_dato) {
			void* dato_a_liberar = hash->vector[posicion]->dato;
			hash->destruir_dato(dato_a_liberar);
			printf("holuu\n");
		}
		hash->vector[posicion]->dato = dato; // Reemplazo el dato
		return true;
	}
	hash->vector[posicion] = nodo_a_guardar;
	hash->posiciones_ocupadas++;
	/*if ((float)hash->posiciones_ocupadas/(float)hash->posiciones_totales >= 0.7) 
		aux_redimensionar(hash);*/
	return true;
}


void* hash_borrar(hash_t *hash, const char *clave);


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


void hash_destruir(hash_t *hash);


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


static void prueba_iterar_hash_vacio()
{
    hash_t* hash = hash_crear(NULL);
    hash_iter_t* iter = hash_iter_crear(hash);
    print_test("Prueba hash iter crear iterador hash vacio", iter);
    print_test("Prueba hash iter esta al final", hash_iter_al_final(iter));
    print_test("Prueba hash iter avanzar es false", !hash_iter_avanzar(iter));
    print_test("Prueba hash iter ver actual es NULL", !hash_iter_ver_actual(iter));

    hash_iter_destruir(iter);
    //hash_destruir(hash);
}


























void pruebas() {
	int dato0 = 0;
	int dato1 = 1;
	int dato2 = 2;
	int dato3 = 3;
	int dato99 = 99;
	int dato88 = 88;

	hash_t *hash = hash_crear(NULL);
	print_test("Cantidad es 0", hash_cantidad(hash) == 0);
	print_test("Pertenece es false", !hash_pertenece(hash, "Panda"));
	print_test("Obtener es NULL", !hash_obtener(hash, "Panda"));
	print_test("Guardo Panda", hash_guardar(hash, "Panda", &dato0));
	print_test("Cantidad es 1", hash_cantidad(hash) == 1);
	print_test("Pertenece Panda es true", hash_pertenece(hash, "Panda"));
	print_test("Obtener Panda es dato0", hash_obtener(hash, "Panda") == &dato0);
	print_test("Guardo Panda otra vez", hash_guardar(hash, "Panda", &dato1));
	print_test("Cantidad sigue siendo 1", hash_cantidad(hash) == 1);
	print_test("Pertenece Panda es true", hash_pertenece(hash, "Panda"));
	print_test("Obtener Panda es dato1", hash_obtener(hash, "Panda") == &dato1);
	print_test("Guardo Inti", hash_guardar(hash, "Inti", &dato2));
	print_test("Guardo Toto", hash_guardar(hash, "Toto", &dato3));
	print_test("Cantidad es 3", hash_cantidad(hash) == 3);
	print_test("Pertenece Inti es true", hash_pertenece(hash, "Inti"));
	print_test("Pertenece Toto es true", hash_pertenece(hash, "Toto"));
	print_test("Obtener Inti es dato2", hash_obtener(hash, "Inti") == &dato2);
	print_test("Obtener Toto es dato3", hash_obtener(hash, "Toto") == &dato3);
	print_test("Guardo Pachita", hash_guardar(hash, "Pachita", &dato2));
	print_test("Guardo Masche", hash_guardar(hash, "Masche", &dato2));
	print_test("Guardo Lio", hash_guardar(hash, "Lio", &dato2));
	print_test("Guardo Felix", hash_guardar(hash, "Felix", &dato2));
	print_test("Guardo Miedo", hash_guardar(hash, "Miedo", &dato99));
	print_test("Guardo Skyler", hash_guardar(hash, "Skyler", &dato2));
	print_test("Guardo Peludito", hash_guardar(hash, "Peludito", &dato2));
	print_test("Cantidad es 10", hash_cantidad(hash) == 10);
	print_test("Pertenece Miedo es true", hash_pertenece(hash, "Miedo"));
	print_test("Obtener Miedo es dato99", hash_obtener(hash, "Miedo") == &dato99);
	print_test("Guardo Miedo otra vez", hash_guardar(hash, "Miedo", &dato0));
	print_test("Cantidad sigue siendo 10", hash_cantidad(hash) == 10);
	print_test("Obtener Miedo es dato0", hash_obtener(hash, "Miedo") == &dato0);
	print_test("Guardo sdfgh", hash_guardar(hash, "sdfgh", &dato2));
	print_test("Guardo ojhi7fv", hash_guardar(hash, "ojhi7fv", &dato2));
	print_test("Guardo mvjgv", hash_guardar(hash, "mvjgv", &dato2));
	print_test("Guardo campania", hash_guardar(hash, "campania", &dato2));
	print_test("Guardo tagliafico", hash_guardar(hash, "tagliafico", &dato2));
	print_test("Guardo toledo", hash_guardar(hash, "toledo", &dato2));
	print_test("Guardo pellerano", hash_guardar(hash, "pellerano", &dato2));
	print_test("Guardo cuesta", hash_guardar(hash, "cuesta", &dato2));
	print_test("Guardo mendez", hash_guardar(hash, "mendez", &dato2));
	print_test("Guardo ortiz", hash_guardar(hash, "ortiz", &dato2));
	print_test("Guardo denis", hash_guardar(hash, "denis", &dato88));
	print_test("Cantidad es 21", hash_cantidad(hash) == 21);
	print_test("Obtener denis es dato88", hash_obtener(hash, "denis") == &dato88);
	print_test("Guardo yoyo", hash_guardar(hash, "yoyo", &dato88));
	print_test("Guardo jojop", hash_guardar(hash, "jojop", &dato88));
	print_test("Guardo gugu", hash_guardar(hash, "gugu", &dato88));
	print_test("Guardo hair", hash_guardar(hash, "hair", &dato88));
	print_test("Guardo lede", hash_guardar(hash, "lede", &dato88));
	print_test("Guardo garata", hash_guardar(hash, "garata", &dato88));
	print_test("Guardo herete", hash_guardar(hash, "herete", &dato88));
	print_test("Guardo poloto", hash_guardar(hash, "poloto", &dato88));
	print_test("Guardo gogog", hash_guardar(hash, "gogog", &dato88));
	print_test("Guardo cvbnm", hash_guardar(hash, "cvbnm", &dato88));
	print_test("Guardo ghjkl", hash_guardar(hash, "ghjkl", &dato88));
	print_test("Guardo poiuy", hash_guardar(hash, "poiuy", &dato88));
	print_test("Guardo uhbnji", hash_guardar(hash, "uhbnji", &dato88));
	print_test("Guardo qwert", hash_guardar(hash, "qwert", &dato88));
	print_test("Guardo xdrcft", hash_guardar(hash, "xdrcft", &dato88));
	print_test("Guardo pqmz", hash_guardar(hash, "pqmz", &dato88));
	print_test("Guardo edcrfv", hash_guardar(hash, "edcrfv", &dato88));
	print_test("Guardo kolpse", hash_guardar(hash, "kolpse", &dato88));
	print_test("Guardo aquino", hash_guardar(hash, "aquino", &dato88));
	print_test("Guardo 852741", hash_guardar(hash, "852741", &dato88));
	print_test("Guardo osky", hash_guardar(hash, "osky", &dato88));
	print_test("Guardo caro", hash_guardar(hash, "caro", &dato88));
	print_test("Guardo fernet", hash_guardar(hash, "fernet", &dato88));

	printf("Colisiones: %i\n", colisiones);




}

int main() {
	pruebas();
	prueba_iterar_hash_vacio();
}