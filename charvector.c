/** @file
 * Implementacja dynamicznej tablicy przechowującej znaki
 *
 * @author Kacper Sołtysiak <ks418388@students.mimuw.edu.pl>
 * @copyright Uniwersytet Warszawski
 * @date 13.05.2020
 */

#include <stdlib.h>
#include "charvector.h"

/** Domyślny rozmiar wektora */
#define default_size 30

/** @brief Ustawia domyślne parametry  wektora.
 * @param[in] created_v - wskaźnik na wektor
 */
static void set_default_params(char_v * created_v)
{
	created_v->vector_array = malloc(default_size * sizeof(char));
	if(created_v->vector_array == NULL)
	{
		exit(1);
	}
	created_v->size_of_array = default_size;
	created_v->char_count = 0;
}

/** @brief Sprawdza, czy potrzebna jest rozszerzenie wektora.
 * Dokonuje powiększenia rozmiaru w przypadku, gdy jest
 * to konieczne. 
 * @param[in] target_v - wskaźnik na wektor
 */
static void check_resize(char_v * target_v)
{
	int32_t array_len = target_v->size_of_array;
	int32_t filled = target_v->char_count;
	if(filled > array_len - 2)
	{
		target_v->vector_array = realloc(target_v->vector_array, 2*array_len*sizeof(char));
		if(target_v->vector_array == NULL)
		{
			exit(1);
		}
		target_v->size_of_array = 2 * array_len;
	}
}

/** @brief Aktualizuje parametry wektora po dodaniu do niego znaku.
 * @param[in] target_v - wskaźnik na wektor
 * @param[in] target_char - znak dodawany do wektora
 */
static void insert_char(char_v * target_v, char target_char)
{
	int32_t filled = target_v->char_count;
	char * v_array = target_v->vector_array;
	v_array[filled] = target_char;
	target_v->char_count = filled + 1;
}

void dispose_of_vector(char_v * target_v)
{
	if(target_v != NULL)
	{
		char * array_ptr = target_v->vector_array;
		free(array_ptr);
		free(target_v);
	}
}

void add_char(char_v * target_v, char target_char)
{
	if(target_v != NULL)
	{
		check_resize(target_v);
		insert_char(target_v, target_char);
	}
}

void reset_vector(char_v * target_v)
{
	if(target_v != NULL)
	{
		free(target_v->vector_array);
		set_default_params(target_v);
	}
}

char_v * create_new_vector()
{
	char_v * new_vector = malloc(sizeof(char_v));
	if(new_vector == NULL)
	{
		return NULL;
	}
	set_default_params(new_vector);
	if(new_vector->vector_array == NULL)
	{
		free(new_vector);
		return NULL;
	}
	return new_vector;
}