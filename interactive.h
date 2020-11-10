/** @file
 * Interfejs trybu interaktywnego gry Gamma
 *
 * @author Kacper Sołtysiak <ks418388@students.mimuw.edu.pl>
 * @copyright Uniwersytet Warszawski
 * @date 13.05.2020
 */

#ifndef INTERACTIVE_H
#define INTERACTIVE_H

#include "gamma.h"
#include <stdint.h>

/** @brief Obsługuje tryb interaktywny gry Gamma.
 * @param[in] g - wskaźnik na strukturę gry
 */
void interactive_mode(gamma_t * g);

#endif