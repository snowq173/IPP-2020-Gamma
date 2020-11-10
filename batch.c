/** @file
 * Implementacja trybu wsadowego gry Gamma
 *
 * @author Kacper Sołtysiak <ks418388@students.mimuw.edu.pl>
 * @copyright Uniwersytet Warszawski
 * @date 13.05.2020
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "charvector.h"
#include "batch.h"
#include "gamma.h"
#include "input.h"

/** Zmienna statyczna na pierwszy argument 
	przekazany w poleceniu trybu wsadowego */
static char * first;
/** Zmienna statyczna na drugi argument 
	przekazany w poleceniu trybu wsadowego */
static char * second;
/** Zmienna statyczna na trzeci argument 
	przekazany w poleceniu trybu wsadowego */
static char * third;
/** Zmienna statyczna na czwarty argument 
	przekazany w poleceniu trybu wsadowego */
static char * fourth;
/** Zmienna statyczna na dodatkowy, niedozwolony argument 
	przekazany w poleceniu trybu wsadowego */
static char * error_arg;

/** @brief Wywołuje funkcję gamma_golden_possible.
 * @param[in] g - wskaźnik na strukturę gry
 * @param[in] line - numer wiersza, w którym pojawiło się polecenie
 * wywołania funkcji.
 */
static inline void execute_gamma_golden_possible(gamma_t * g, uint32_t line)
{
	if(strcmp(first, "q") != 0 || third != NULL || !check_string(second))
	{
		print_error(line);
	}
	else
	{
		if(gamma_golden_possible(g, strtoull(second, NULL, 10)))
		{
			fprintf(stdout, "1\n");
		}
		else
		{
			fprintf(stdout, "0\n");
		}
	}
}

/** @brief Wywołuje funkcję gamma_free_fields.
 * @param[in] g - wskaźnik na strukturę gry
 * @param[in] line - numer wiersza, w którym pojawiło się polecenie
 * wywołania funkcji.
 */
static inline void execute_gamma_free_fields(gamma_t * g, uint32_t line)
{
	if(strcmp(first, "f") != 0 || third != NULL || !check_string(second))
	{
		print_error(line);
	}
	else
	{
		fprintf(stdout, "%li\n", gamma_free_fields(g, strtoull(second, NULL, 10)));
	}
}

/** @brief Wywołuje funkcję gamma_busy_fields.
 * @param[in] g - wskaźnik na strukturę gry
 * @param[in] line - numer wiersza, w którym pojawiło się polecenie
 * wywołania funkcji.
 */
static inline void execute_gamma_busy_fields(gamma_t * g, uint32_t line)
{
	if(strcmp(first, "b") != 0 || third != NULL || !check_string(second))
	{
		print_error(line);
	}
	else
	{

		fprintf(stdout, "%li\n", gamma_busy_fields(g, strtoull(second, NULL, 10)));
	}	
}

/** @brief Wywołuje funkcję gamma_golden_move.
 * @param[in] g - wskaźnik na strukturę gry
 * @param[in] line - numer wiersza, w którym pojawiło się polecenie
 * wywołania funkcji.
 */
static inline void execute_gamma_golden_move(gamma_t * g, uint32_t line)
{
	if(strcmp(first, "g") != 0 || !check_string(second) || !check_string(third)
														|| !check_string(fourth))
	{
		print_error(line);
	}
	else
	{
		uint32_t player = strtoull(second, NULL, 10);
		uint32_t x = strtoull(third, NULL, 10);
		uint32_t y = strtoull(fourth, NULL, 10);

		if(gamma_golden_move(g, player, x, y))
		{
			fprintf(stdout, "1\n");
		}
		else
		{
			fprintf(stdout, "0\n");
		}
	}
}

/** @brief Wywołuje funkcję gamma_move.
 * @param[in] g - wskaźnik na strukturę gry
 * @param[in] line - numer wiersza, w którym pojawiło się polecenie
 * wywołania funkcji.
 */
static inline void execute_gamma_move(gamma_t * g, uint32_t line) 
{
	if(strcmp(first, "m") != 0 || !check_string(second) || !check_string(third)
														|| !check_string(fourth))
	{
		print_error(line);
	}
	else
	{
		uint32_t player = strtoull(second, NULL, 10);
		uint32_t x = strtoull(third, NULL, 10);
		uint32_t y = strtoull(fourth, NULL, 10);

		if(gamma_move(g, player, x, y))
		{
			fprintf(stdout, "1\n");
		}
		else
		{
			fprintf(stdout, "0\n");
		}
	}
}

/** @brief Wywołuje funkcję gamma_board.
 * @param[in] g - wskaźnik na strukturę gry
 * @param[in] line - numer wiersza, w którym pojawiło się polecenie
 * wywołania funkcji.
 */
static inline void execute_gamma_board(gamma_t * g, uint32_t line)
{
	if(strcmp(first, "p") != 0 || second != NULL)
	{
		print_error(line);
	}
	else
	{
		char * board = gamma_board(g);
		if(board == NULL)
		{
			print_error(line);
		}
		else
		{
			fprintf(stdout, "%s", board);
			free(board);
		}
	}
}

/** @brief Przetwarza linię wejścia w trybie wsadowym.
 * @param[in] g - wskaźnik na strukturę gry
 * @param[in] endline - czy przetwarzany wiersz był zakończony znakiem nowej linii
 * @param[in] input_v - wskaźnik na wektor, do którego wczytujemy wiersz
 * @param[in] line - numer przetwarzanego wiersza, dodatnia liczba całkowita
 */
static void process_line_batch_mode(gamma_t * g, bool endline,
									char_v * input_v, uint32_t line)
{
	char * array = input_v->vector_array;
	char separators[] = " \t\v\f\r";

	if(array[0] == '\n' || array[0] == '#')
	{
		return;
	}
	
	if(input_v->char_count == 1)
	{
		return;
	}
	else if(!endline)
	{
		print_error(line);
		return;
	}
	else if(array[0] != 'q' && array[0] != 'f' && array[0] != 'b'
				&& array[0] != 'p' && array[0] != 'g' && array[0] != 'm')
	{
		print_error(line);
		return;
	}

	first = strtok(array, separators);
	second = strtok(NULL, separators);
	third = strtok(NULL, separators);
	fourth = strtok(NULL, separators);
	error_arg = strtok(NULL, separators);

	if(error_arg != NULL)
	{
		print_error(line);
		return;
	}

	switch(array[0])
	{
		case 'q':
		{
			execute_gamma_golden_possible(g, line);
			break;
		}
		case 'f':
		{
			execute_gamma_free_fields(g, line);
			break;
		}
		case 'b':
		{
			execute_gamma_busy_fields(g, line);
			break;
		}
		case 'g':
		{
			execute_gamma_golden_move(g, line);
			break;
		}
		case 'm':
		{
			execute_gamma_move(g, line);
			break;
		}
		case 'p':
		{
			execute_gamma_board(g, line);
			break;
		}
		default:
		{
			print_error(line);
			break;
		}
	}
}

void batch_mode(gamma_t * g, uint32_t line)
{
	uint32_t cur_line = line;
	char_v * input_vector = create_new_vector();
	bool end_of_file = false;
	while(!end_of_file)
	{
		cur_line++;
		bool cur = get_new_input_line(input_vector, &end_of_file);
		process_line_batch_mode(g, cur, input_vector, cur_line);
		reset_vector(input_vector);
	}
	dispose_of_vector(input_vector);
}