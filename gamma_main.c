/** @file
 * Główny plik, zawierający funkcję główną programu
 *
 * @author Kacper Sołtysiak <ks418388@students.mimuw.edu.pl>
 * @copyright Uniwersytet Warszawski
 * @date 13.05.2020
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include "charvector.h"
#include "gamma.h"
#include "interactive.h"
#include "batch.h"
#include "input.h"

/** @brief Funkcja główna programu.
 * @return Wartość 0, gdy gra przebiegła prawidłowo bez błędów krytycznych,
 * a wartość 1, gdy wystąpił błąd krytyczny.
 */
int main(void)
{
	char_v * input_v = create_new_vector();
	
	bool batch = false;
	bool interactive = false;
	bool file_end = false;

	uint32_t line_count = 0;

	gamma_t * game = NULL;

	while(!batch && !interactive && !file_end)
	{
		line_count++;
		bool current_line = get_new_input_line(input_v, &file_end);
		game = detect_mode(input_v, line_count, &batch, &interactive,
								current_line);
		reset_vector(input_v);
	}

	if(batch)
	{
		batch_mode(game, line_count);
	}
	else if(interactive)
	{
		interactive_mode(game);
	}
	
	gamma_delete(game);
	dispose_of_vector(input_v);
}