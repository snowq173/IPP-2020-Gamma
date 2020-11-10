/** @file
 * Interfejs wczytywania danych oraz reagowania na błędy
 * w tekście przyjmowanym przez program
 *
 * @author Kacper Sołtysiak <ks418388@students.mimuw.edu.pl>
 * @copyright Uniwersytet Warszawski
 * @date 13.05.2020
 */

#ifndef MISC_H
#define MISC_H

#include <stdbool.h>
#include <stdint.h>
#include "charvector.h"

/** @brief Wypisuje komunikat o błędzie w wierszu na
 * standardowe wyjście diagnostyczne.
 * @param[in] line - numer wiersza, w którym wystąpił błąd.
 */
void print_error(uint32_t line);

/** @brief Sprawdza, czy podany napis zawiera reprezentację
 * nieujemnej liczby całkowitej, mieszczącej się w zakresie uint32
 * @param[in] t_string - wskaźnik na sprawdzany napis
 * @return Wartość @p true, gdy podany napis jest poprawny w sensie
 * opisu funkcji, a wartość @p false w przeciwnym wypadku.
 */
bool check_string(char * t_string);

/** @brief Wczytuje następną linię wejścia.
 * @param[in] t_vector - wektor, do którego wczytujemy wiersz
 * @param[in] file_end - wskaźnik na zmienną logiczną przechowującą
 * informację, czy wczytaliśmy już znak końca pliku.
 */
bool get_new_input_line(char_v * t_vector, bool * file_end);

/** Funkcja wykrywająca tryb gry. Przetwarza linię wejścia w celu
 * napotkania poprawnej komendy definiującej tryb gry.
 * @param[in] input_v - wskaźnik na wektor przechowujący zapis linii
 * @param[in] line - numer przetwarzanego wiersza
 * @param[in] batch - wskaźnik na zmienną logiczną przechowującą informację,
 * czy tryb to będzie tryb wsadowy
 * @param[in] interactive - wskaźnik na zmienną logiczną przechowującą informację,
 * czy tryb to będzie tryb interaktywny
 * @param[in] endline - czy przetwarzana linia została zakończona znakiem końca wiersza
 * @return Wartość @p NULL, gdy dana linia jest ignorowana, błędna lub alokacja
 * pamięci na strukturę gry nie powiodła się. W przeciwnym wypadku wskaźnik
 * na utworzoną strukturę gry.
 */
gamma_t * detect_mode(char_v * input_v, uint32_t line, bool * batch, 
				bool* interactive, bool endline);

#endif