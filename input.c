/** @file
 * Implementacja pomocniczych funkcji do obsługi wejścia
 * oraz reagowania na błędy w tekście przyjmowanym przez
 * program
 *
 * @author Kacper Sołtysiak <ks418388@students.mimuw.edu.pl>
 * @copyright Uniwersytet Warszawski
 * @date 13.05.2020
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include "charvector.h"
#include "gamma.h"

void print_error(uint32_t line)
{
	fprintf(stderr, "%s%d\n", "ERROR ", line);
}

bool check_string(char * target_string)
{
	if(target_string == NULL)
	{
		return false;
	}

	bool correct = true;

	for(int i = 0; target_string[i] != '\0' && correct; i++)
	{
		correct = (isdigit(target_string[i]) != 0);
	}

	return correct && !(strtoul(target_string, NULL, 10) > UINT32_MAX);
}

bool get_new_input_line(char_v * input_v, bool * f_end)
{
	int32_t input_char = getchar();

	if(input_char == '\n')
	{
		add_char(input_v, (char) input_char);
	}

	while(input_char != '\n' && input_char != EOF)
	{
		add_char(input_v, (char) input_char);
		input_char = getchar();
	}

	add_char(input_v, '\0');
	
	if(input_char == EOF)
	{
		*f_end = true;
	}

	return (input_char == '\n');
}


gamma_t * detect_mode(char_v * input_v, uint32_t line, bool * batch, 
							bool * interactive, bool endline)
{
	char * array = input_v->vector_array;

	if(input_v->char_count == 1)
	{
		return NULL;
	}
	else if(!endline)
	{
		print_error(line);
		return NULL;
	}
	else if(array[0] == '\n' || array[0] == '#')
	{
		return NULL;
	}
	else if(array[0] != 'B' && array[0] != 'I')
	{
		print_error(line);
		return NULL;
	}

	char separators[] = " \t\v\f\r";

	char * first = strtok(array, separators);
	if(first == NULL)
	{
		print_error(line);
		return NULL;
	}
	if((strcmp(first, "B") != 0) && (strcmp(first, "I") != 0))
	{
		print_error(line);
		return NULL;
	}
	char * second = strtok(NULL, separators);
	if(!check_string(second))
	{
		print_error(line);
		return NULL;
	}
	char * third = strtok(NULL, separators);
	if(!check_string(third))
	{
		print_error(line);
		return NULL;
	}
	char * fourth = strtok(NULL, separators);
	if(!check_string(fourth))
	{
		print_error(line);
		return NULL;
	}
	char * fifth = strtok(NULL, separators);
	if(!check_string(fifth))
	{
		print_error(line);
		return NULL;
	}
	char * error_arg = strtok(NULL, separators);
	if(error_arg != NULL)
	{
		print_error(line);
		return NULL;
	}

	uint32_t arg_first = strtoull(second, NULL, 10);
	uint32_t arg_second = strtoull(third, NULL, 10);
	uint32_t arg_third = strtoull(fourth, NULL, 10);
	uint32_t arg_fourth = strtoull(fifth, NULL, 10);

	if(arg_first == 0 || arg_second == 0 || arg_third == 0 || arg_fourth == 0)
	{
		print_error(line);
		return NULL;
	}

	gamma_t * new_gamma = gamma_new(arg_first, arg_second, arg_third, arg_fourth);

	if(new_gamma == NULL)
	{
		print_error(line);
	}
	else if(array[0] == 'B')
	{
		*batch = true;
		fprintf(stdout, "%s%d\n", "OK ", line);
	}
	else
	{
		*interactive = true;
	}
	return new_gamma;
}