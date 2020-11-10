/** @file
 * Interfejs dynamicznej tablicy przechowującej znaki
 *
 * @author Kacper Sołtysiak <ks418388@students.mimuw.edu.pl>
 * @copyright Uniwersytet Warszawski
 * @date 13.05.2020
 */


#ifndef CHARVECTOR_H
#define CHARVECTOR_H

#include <stdint.h>

/**
 * Struktura wektora przechowywującego znaki
 */
typedef struct char_v char_v;

/** @struct char_v
 * Definicja struktury char_v
 */
struct char_v
{
	char * vector_array;
	/**< Wskaźnik na tablicę znaków */
	int32_t size_of_array;
	/**< Rozmiar tablicy znaków */
	int32_t char_count;
	/**< Liczba znaków przechowywanych w tablicy */
};

/** @brief Usuwa z pamięci wektor.
 * @param[in] vector - wskaźnik na strukturę
 */
void dispose_of_vector(char_v * vector);

/** @brief Dodaje znak do wektora.
 * @param[in] vector - wskaźnik na strukturę wektora
 * @param[in] c - znak dodawany do wektora
 */
void add_char(char_v * vector, char c);

/** @brief Przywraca wektor do ustawień domyślnych
 * @param[in] vector - wskaźnik na resetowany wektor
 */
void reset_vector(char_v * vector);

/** @brief Tworzy nowy wektor.
 * @return Wskaźnik na strukturę przechowującą utworzony wektor
 */
char_v * create_new_vector();

#endif