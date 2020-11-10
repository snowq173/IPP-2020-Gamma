/** @file
 * Implementacja trybu interaktywnego gry Gamma
 *
 * @author Kacper Sołtysiak <ks418388@students.mimuw.edu.pl>
 * @copyright Uniwersytet Warszawski
 * @date 13.05.2020
 */

#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <ctype.h>
#include "gamma.h"

/** Makro do przesuwania kursora w prawo */
#define move_right(x) printf("\033[%uC", x)

/** Makro do przesuwania kursora w lewo */
#define move_left(x) printf("\033[%uD", x)

/** Makro do przesuwania kursora w górę */
#define move_up(x) printf("\033[%uA", x)

/** Makro do przesuwania kursora w dół */
#define move_down(x) printf("\033[%uB", x)

/** Kod strzałki w lewo */
#define LEFT 68

/** Kod strzałki w górę */
#define UP 65

/** Kod strzałki w prawo */
#define RIGHT 67

/** Kod strzałki w dół */
#define DOWN 66

/** Kod kombinacji Ctrl-D */
#define CTRLD '\4'

/** Aktualna kolumna względem planszy gry */
static uint32_t pos_x;
/** Aktualny wiersz względem planszy gry */
static uint32_t pos_y;
/** Aktualna kolumna względem terminala */
static uint32_t x;
/** Aktualny wiersz względem terminala */
static uint32_t y;
/** Liczba cyfr w zapisie dziesiętnym liczby
	graczy uczestniczących w grze */
static uint32_t digits_count;
/** Wysokość planszy gry */
static uint32_t height;
/** Szerokość planszy gry */
static uint32_t width;
/** Liczba graczy biorących udział w grze */
static uint32_t players_count;

/** @brief Wypisuje stan planszy.
 * @param[in] g - wskaźnik na strukturę gry
 */
static inline void print_board(gamma_t * g)
{
	char * p_board = gamma_board(g);
	if(p_board == NULL)
	{
		exit(1);
	}
	printf("%s\n", p_board);
	free(p_board);
}

/** @brief Wyznacza liczbę cyfr dodatniej liczby całkowitej
 * @param[in] t_int - dana liczba
 * @return Dodatnia liczba całkowita odpowiadająca liczbie cyfr
 * podanej liczby całkowitej.
 */
static uint32_t digits(uint32_t t_int)
{
	uint32_t dig = 0;
	while(t_int)
	{
		t_int/=10;
		dig++;
	}
	return dig;
}

/** @brief Czyści ekran.
 */
static inline void clear_screen()
{
	printf("\e[1;1H\e[2J");
}

/** @brief Czyści aktualny wiersz.
 */
static inline void clear_current_line()
{
	printf("%c[2K", 27);
}

/** @brief Wypisuje informacje o graczu.
 * @param[in] g - wskaźnik na strukturę gry
 * @param[in] player - numer gracza
 */
static inline void print_player_info(gamma_t * g, uint32_t player)
{
	printf("PLAYER %u", player);
	printf(" Busy fields: %lu", gamma_busy_fields(g, player));
	printf(" Free fields: %lu", gamma_free_fields(g, player));
	if(gamma_golden_possible(g, player))
	{
		printf("\x1B[33m Golden move possible \x1B[0m\n");
	}
	else
	{
		printf("\x1B[31m Golden move not possible \x1B[0m\n");
	}
}

/** @brief Sprawdza, czy możliwa jest dalsza gra.
 * @param[in] g - wskaźnik na strukturę gry
 * @return Wartość @p true, gdy możliwa jest dalsza rozgrywka, a
 * wartość @p false w przeciwnym wypadku.
 */
static inline bool game_possible(gamma_t * g)
{
	bool c_possible = false;

	for(uint32_t i = 0; i < players_count; i++)
	{
		bool first_condition = gamma_free_fields(g, i+1) > 0;
		first_condition = first_condition || gamma_golden_possible(g, i+1);
		c_possible = c_possible || first_condition;
	}
	return c_possible;
}

/** @brief Przesuwa wskaźnik na pole znajdujące się po lewej
 * stronie aktualnego pola, o ile to możliwe.
 */
static inline void move_cursor_left()
{
	if(pos_x > 1)
	{
		if(players_count < 10)
		{
			move_left(1);
			x--;
		}
		else
		{
			move_left(digits_count+1);
			x -= (digits_count+1);
		}
		pos_x--;
	}
}

/** @brief Przesuwa wskaźnik na pole znajdujące się po prawej
 * stronie aktualnego pola, o ile to możliwe.
 */
static inline void move_cursor_right()
{
	if(pos_x < width)
	{
		if(players_count < 10)
		{
			move_right(1);
			x++;
		}
		else
		{
			move_right(digits_count+1);
			x += (digits_count+1);
		}
		pos_x++;
	}
}
/** @brief Przesuwa wskaźnik na pole znajdujące się poniżej
 * aktualnego pola, o ile to możliwe.
 */
static inline void move_cursor_down()
{
	if(pos_y < height)
	{
		move_down(1);
		pos_y++;
		y++;
	}
}

/** @brief Przesuwa wskaźnik na pole znajdujące się powyżej
 * aktualnego pola, o ile to możliwe.
 */
static inline void move_cursor_up()
{
	if(pos_y > 1)
	{
		move_up(1);
		pos_y--;
		y--;
	}
}

/** @brief Wykonuje zwykły ruch.
 * @param[in] g - wskaźnik na strukturę gry
 * @param[in] player - numer gracza wykonującego ruch
 * @param[in] flag - wskaźnik na zmienną logiczną, aktualizowaną
 * do wartości logicznej zwróconej przez wywołaną wewnątrz funkcję
 * @ref gamma_move.
 */
static inline void execute_move(gamma_t * g, uint32_t player, bool * flag)
{
	*flag = gamma_move(g, player, pos_x-1, height-pos_y);
	if(*flag)
	{
		printf("%u", player);
	}
}

/** @brief Wykonuje złoty ruch.
 * @param[in] g - wskaźnik na strukturę gry
 * @param[in] player - numer gracza wykonującego ruch
 * @param[in] flag - wskaźnik na zmienną logiczną, aktualizowaną
 * do wartości logicznej zwróconej przez wywołaną wewnątrz funkcję
 * @ref gamma_golden_move.
 */
static inline void execute_golden_move(gamma_t * g, uint32_t player, bool * flag)
{
	*flag = gamma_golden_move(g, player, pos_x-1, height-pos_y);
	if(*flag)
	{
		printf("%u", player);
		if(players_count > 9)
		{
			for(uint32_t i = 0;  i < digits_count - digits(player); i++)
			{
				printf(" ");
			}
		}
	}
}

/** @brief Przetwarza ruch gracza. Oczekuje komendy aż do momentu
 * napotkania możliwej do wykonania (w sensie ruchu - zwykłego lub
 * złotego), rezygnacji z wykonywania ruchu lub zakończenia gry.
 * @param[in] g - wskaźnik na strukturę gry
 * @param[in] player - numer gracza
 * @param[in] game_flag - wskaźnik na zmienną logiczną informującą,
 * czy gra ma się toczyć dalej.
 */
static inline void proceed_player(gamma_t * g, uint32_t player,
									bool * game_flag)
{
	bool flag = false;
	int32_t c;
	while(!flag)
	{
		c = getchar();

		if(c == LEFT)
		{
			move_cursor_left();
		}
		else if(c == RIGHT)
		{
			move_cursor_right();
		} 
		else if(c == UP)
		{
			move_cursor_up();
		}
		else if(c == DOWN)
		{
			move_cursor_down();
		}
		else if(c == ' ')
		{
			execute_move(g, player, &flag);
		}
		else if(c == 'G' || c == 'g')
		{
			execute_golden_move(g, player, &flag);
		}
		else if(c == 'C' || c == 'c')
		{
			flag = true;
		}
		else if(c == CTRLD)
		{
			*game_flag = false;
			flag = true;
		}
		else if(c == EOF)
		{
			clear_screen();
			printf("ERROR\n");
			exit(1);
		}
	}
}

/** Funkcja wypisująca stan planszy oraz "wyniki" graczy
 * po zakońćzeniu rozgrywki.
 * @param[in] g - wskaźnik na strukturę gry
 */
static void print_after_game(gamma_t * g)
{
	print_board(g);
	for(uint32_t i = 0; i < players_count; i++)
	{
		printf("\x1b[36mPLAYER %u \x1b[0mFields: \x1b[32m%lu\n\x1b[0m", 
							i+1, gamma_busy_fields(g, i+1));
	}
}

/** @brief Sprawdza, czy terminal ma rozmiar wystarczający na wyświetlenie
 * stanu gry.
 * @return Wartość @p true, gdy wymiary terminala są wystarczające, a @p false
 * w przeciwnym wypadku.
 */
static bool check_terminal_size()
{
    struct winsize w;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
	if(players_count < 10)
	{
		if(width > w.ws_col || height >= (uint32_t) w.ws_row - 1)
		{
			printf("Zbyt duzy rozmiar planszy.\n");
			return false;
		}
		return true;
	}
	else
	{
		bool wrong_width = width*(digits_count+1) > ((uint32_t) w.ws_col);
		if(wrong_width || height >= (uint32_t) w.ws_row - 1)
		{
			printf("Zbyt duzy rozmiar planszy.\n");
			return false;
		}
		return true;
	}
}

void interactive_mode(gamma_t * g)
{
    struct termios oldt, newt;

	tcgetattr(STDIN_FILENO, &oldt);
	newt = oldt;
	newt.c_lflag &= ~(ICANON|ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);

	height = get_board_height(g);
	width = get_board_width(g);
	players_count = get_players_count(g);
	digits_count = digits(players_count);

	bool size_check = check_terminal_size();
	bool game_flag = size_check;

	if(!size_check)
	{
		tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
	}

	pos_x = 1;
	pos_y = 1;
	x = 1;
	y = 1;

	if(game_flag)
	{
		clear_screen();
		print_board(g);
	}

    while(game_flag) 
   	{
   		for(uint32_t i = 0; i < players_count && game_possible(g) && game_flag; i++)
   		{
   			if(!gamma_golden_possible(g, i+1) && (gamma_free_fields(g, i+1) == 0))
   			{
   				continue;
   			}

	        printf("\033[%u;%uf", height+1, 0);
	        clear_current_line();
	        print_player_info(g, i+1);
			printf("\033[%u;%uf", y, x);

	        proceed_player(g, i+1, &game_flag);	
	    }

	    if(!game_possible(g))
	    {
	    	game_flag = false;
	    }
    }

    if(size_check)
    {
		tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
		clear_screen();
		print_after_game(g);
	}
}