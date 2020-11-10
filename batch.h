/** @file
 * Interfejs trybu wsadowego
 *
 * @author Kacper Sołtysiak <ks418388@students.mimuw.edu.pl>
 * @copyright Uniwersytet Warszawski
 * @date 13.05.2020
 */

#ifndef BATCH_H
#define BATCH_H

#include <stdint.h>
#include "gamma.h"

/** @brief Obsługuje tryb wsadowy gry Gamma.
 * @param[in] g - wskaźnik na strukturę gry
 * @param[in] line - numer następnego wiersza do wczytania,
 *					 dodatnia liczba całkowita
 */
void batch_mode(gamma_t * g, uint32_t line);

#endif