/** @file
 * Implementacja interfejsu gry gamma
 *
 * @author Kacper Sołtysiak <ks418388@students.mimuw.edu.pl>
 * @copyright Uniwersytet Warszawski
 * @date 12.04.2020
 */

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include "gamma.h"


/** @struct gamma
 * Definicja struktury gamma
 */
struct gamma
{
	uint32_t width; 	
	/**< Zmienna reprezentująca szerokość planszy gry */
	uint32_t height; 	
	/**< Zmienna reprezentująca wysokość planszy gry */
	uint32_t players_count; 	
	/**< Zmienna reprezentująca liczbę graczy
	w grze */
	uint32_t maximum_area_count; 	
	/**< Zmienna reprezentująca maksymalną
	dopuszczalną liczbę rozłącznych obszarów
	na planszy w posiadaniu gracza */
	uint32_t * game_array;
	/**< Tablica rozmiaru równego całkowitej powierzchni planszy,
	przechowuje stan gry, w komórce tej tablicy znajduje się 0,
	jeśli pole o wskazanym indeksie jest wolne, w przeciwnym wypadku
	dodatnia liczba całkowita - numer gracza zajmującego dane pole */
	uint32_t * areas_array;
	/**< Tablica wymiaru @p players_count, przechowująca informację
	 o tym, ile obszarów posiada każdy z graczy*/
	uint64_t * occupied_fields_array;
	/**< Tablica rozmiaru równego liczbie graczy, przechowuje
	informację ile różnych pól sąsiadujących z polami danego gracza
	jest wolnych */
	uint64_t * free_neighbours;
	/**< Tablica rozmiaru równego całkowitej powierzchni planszy,
	przechowuje indeks, pod którym znajduje się "rodzic" obszaru,
	do którego przynależy dane pole */
	uint64_t * parent_array;
	/**< Tablica rozmiaru równemu całkowitej powierzchni planszy,
	przechowuje informacje o rzędach pól, wykorzystywanych
	przy spajaniu obszarów */
	uint64_t * rank_array;
	/**< Tablica wymiaru @p players_count, przechowująca informację
	 o tym, ile pól posiada każdy z graczy */
	uint64_t busy_fields_count; 	
	/**< Zmienna reprezentująca
	 liczbę zajętych pól na planszy gry */
	bool * golden_moves_array;
	/**< Tablica wymiaru @p players_count, przechowująca informację,
	 który z graczy wykonał już złoty ruch*/
};

/** @brief Znajduje główne pole obszaru, do którego przynależy
 * pole o podanym numerze.
 * @param[in] g 				– wskaźnik na strukturę gry
 * @param[in] pos 				– numer pola
 * @return Liczba całkowita będąca numerem poszukiwanego
 * głównego pola.
 */
static uint64_t find_field(gamma_t * g, uint64_t pos)
{
	if((g->parent_array)[pos] == pos)
	{
		return pos;
	}
	(g->parent_array)[pos] = find_field(g, (g->parent_array)[pos]);
	return (g->parent_array)[pos];
}

/** @brief Łączy obszary, do których należą dane pola w spójny obszar.
 * @param[in] g 				  – wskaźnik na strukturę gry
 * @param[in] first_p 		 	  – numer pierwszego pola
 * @param[in] second_p 		 	  – numer drugiego pola
 */
static void connect_fields(gamma_t * g, uint64_t first_p, uint64_t second_p)
{
	uint64_t first = find_field(g, first_p);
	uint64_t second = find_field(g, second_p);
	uint64_t first_rank = (g->rank_array)[first];
	uint64_t second_rank = (g->rank_array)[second];
	if(first_rank > second_rank)
	{
		(g->parent_array)[second] = first;
	}
	else if(first_rank < second_rank)
	{
		(g->parent_array)[first] = second;
	}
	else if(first != second)
	{
		(g->parent_array)[second] = first;
		(g->rank_array)[first]++;
	}
}

/** @brief Ustawia parametry gry gamma zgodnie z wartościami przekazanymi
 * do funkcji jako argumenty. Następnie wywołuje funkcje
 * @param[in] new_gamma 	 	  – wskaźnik na strukturę gry
 * @param[in] width 		 	  – szerokość planszy
 * @param[in] height 		 	  – wysokość planszy
 * @param[in] players 		 	  – liczba graczy
 * @param[in] areas 		 	  – maksymalna  dopuszczalna liczba 
 *							   	    obszarów należących do jednego gracza
 */
static void set_basic_gamma_params(gamma_t * new_gamma, uint32_t width,
			uint32_t height, uint32_t players, uint32_t areas)
{
	new_gamma->width = width;
	new_gamma->height = height;
	new_gamma->players_count = players;
	new_gamma->maximum_area_count = areas;
	new_gamma->busy_fields_count = 0;
}

/** @brief Ustawia początkowe wartości w tablicach przechowywanych
 * w strukturze gry.
 * @param[in] g 				– wskaźnik na strukturę gry
 */
static void set_arrays(gamma_t * g)
{
	uint64_t a_size = (uint64_t) g->width * (uint64_t) g->height;
	for(uint64_t i = 0; i < a_size; i++)
	{
		(g->rank_array)[i] = 0;
		(g->parent_array)[i] = i;
		(g->game_array)[i] = 0;
	}
	for(uint32_t i = 0; i < g->players_count; i++)
	{
		(g->golden_moves_array)[i] = true;
		(g->areas_array)[i] = 0;
		(g->free_neighbours)[i] = 0;
		(g->occupied_fields_array)[i] = 0;
	}
}

/** @brief Sprawdza, czy funkcja tworząca strukturę gry otrzymała
 * poprawne parametry, tj. każdy z nich jest liczbą całkowitą dodatnią.
 * @param[in] width 		 	  – szerokość planszy w tworzonej grze
 * @param[in] height 		 	  – wysokość planszy w tworzonej grze
 * @param[in] players 		 	  – liczba graczy w tworzonej grze
 * @param[in] areas 		 	  – maksymalna dopuszczalna liczba obszarów 
 *						       		do zajęcia przez jednego gracza
 * @return Wartość @p true, gdy podane parametry są podane oraz
 * wartość @p false w przeciwnym wypadku.
 */
static bool wrong_params(uint32_t width, uint32_t height,
						uint32_t players, uint32_t areas)
{
	return (width == 0 || height == 0
		 || players == 0 || areas == 0);
}

gamma_t * gamma_new(uint32_t width, uint32_t height,
                   uint32_t players, uint32_t areas)
{
	if(wrong_params(width, height, players, areas))
	{
		return NULL;
	}
	gamma_t * new_gamma = malloc(sizeof(gamma_t));
	if(new_gamma == NULL)
	{
		return NULL;
	}
	set_basic_gamma_params(new_gamma, width, height, players, areas);
	uint64_t a_size = (uint64_t) width * (uint64_t) height;
	uint32_t * a_game_array = malloc(a_size * sizeof(uint32_t));
	new_gamma->game_array = a_game_array;
	if(a_game_array == NULL)
	{
		gamma_delete(new_gamma);
		return NULL;
	}
	uint32_t * a_areas_array = malloc(players * sizeof(uint32_t));
	new_gamma->areas_array = a_areas_array;
	if(a_areas_array == NULL)
	{
		gamma_delete(new_gamma);
		return NULL;
	}
	uint64_t * a_occupied_fields_array = malloc(players * sizeof(uint64_t));
	new_gamma->occupied_fields_array = a_occupied_fields_array;
	if(a_occupied_fields_array == NULL)
	{
		gamma_delete(new_gamma);
		return NULL;
	}
	uint64_t * a_free_neighbours = malloc(players * sizeof(uint64_t));
	new_gamma->free_neighbours = a_free_neighbours;
	if(a_free_neighbours == NULL)
	{
		gamma_delete(new_gamma);
		return NULL;
	}
	uint64_t * a_parent_array = malloc(a_size * sizeof(uint64_t));
	new_gamma->parent_array = a_parent_array;
	if(a_parent_array == NULL)
	{
		gamma_delete(new_gamma);
		return NULL;
	}
	uint64_t * a_rank_array = malloc(a_size * sizeof(uint64_t));
	new_gamma->rank_array = a_rank_array;
	if(a_rank_array == NULL)
	{
		gamma_delete(new_gamma);
		return NULL;
	}
	bool * a_golden_moves_array = malloc(players * sizeof(bool));
	new_gamma->golden_moves_array = a_golden_moves_array;
	if(a_golden_moves_array == NULL)
	{
		gamma_delete(new_gamma);
		return NULL;
	}
	set_arrays(new_gamma);
	return new_gamma;
}

void gamma_delete(gamma_t * g)
{
	if(g != NULL)
	{
		if(g->game_array == NULL)
		{
			free(g);
			return;
		}
		free(g->game_array);
		if(g->areas_array == NULL)
		{
			free(g);
			return;
		}
        free(g->areas_array);
        if(g->occupied_fields_array == NULL)
        {
        	free(g);
        	return;
        }
        free(g->occupied_fields_array);
        if(g->free_neighbours == NULL)
        {
        	free(g);
        	return;
        }
        free(g->free_neighbours);
        if(g->parent_array == NULL)
        {
        	free(g);
        	return;
        }
		free(g->parent_array);
		if(g->rank_array == NULL)
		{
			free(g);
			return;
		}
		free(g->rank_array);
		if(g->golden_moves_array == NULL)
		{
			free(g);
			return;
		}
        free(g->golden_moves_array);
        free(g);
	}
}

/** @brief Konwertuje wskazaną pozycję w postaci (x,y) do 
 * liczby całkowitej, będącej indeksem, pod którym w tablicy
 * game_array - polu w strukturze @p g znajduje się
 * wskaźnik na pole przypisane tejże pozycji (x,y).
 * @param[in] g  			 	  – wskaźnik na strukturę gry
 * @param[in] x 			 	  – numer kolumny
 * @param[in] y 			 	  – numer wiersza
 * @return Liczba całkowita - indeks, pod którym w tablicy
 * przechowującej dane o polach na planszy znajduje się pole
 * o podanych współrzędnych.
 */
static inline uint64_t convert_pos(gamma_t * g, uint32_t x, uint32_t y)
{
	uint64_t w = (uint64_t) g->width;
	uint64_t target_position = w * (uint64_t) y + (uint64_t) x;
	return target_position;
}

/** @brief Sprawdza, czy podane współrzędne są zgodne z
 * parametrami zapisanymi w strukturze gry.
 * @param[in] g  			 	  – wskaźnik na strukturę gry
 * @param[in] x 			 	  – numer kolumny
 * @param[in] y 			 	  – numer wiersza
 * @return Wartość @p true, jeśli podane współrzędne są poprawne dla
 * gry zapisanej w strukturze, a @p false w przeciwnym wypadku.
 */
static inline bool check_coordinates(gamma_t * g, uint32_t x, uint32_t y)
{
	uint32_t bound_of_x = g->width;
	uint32_t bound_of_y = g->height;
	return(x < bound_of_x && y < bound_of_y);
}

/** @brief Sprawdza, czy podany numer gracza jest w zakresie
 * zgodnym z wartością ustawioną w strukturze gry.
 * @p struktury @ref gamma_t.
 * @param[in] g  			 	  – wskaźnik na strukturę gry
 * @param[in] target_player  	  – numer gracza
 * @return Wartość @p true, jeśli dany numer gracza jest poprawny,
 * a @p false w przeciwnym wypadku.
 */
static bool is_player_in_range(gamma_t * g, uint32_t target_player)
{
	return (target_player > 0 && target_player <= g->players_count);
}

/** @brief Sprawdza, czy na polu znajduje się pionek wskazanego gracza.
 * @param[in] g 				– wskaźnik na strukturę gry
 * @param[in] x 				– numer kolumny, w której znajduje się
 *								  sprawdzane pole
 * @param[in] y 				– numer wiersza, w którym znajduje się
 * 								  sprawdzane pole
 * @param[in] player 			– gracz, którego pionka
 * 								  obecność na danym polu sprawdzamy
 * @return Wartość @p true, gdy na wskazanym polu znajduje się pionek
 * danego gracza, a @p false w przeciwnym wypadku.
 */
static inline bool is_on_field(gamma_t * g, uint32_t x, uint32_t y, uint32_t player)
{
	if(check_coordinates(g, x, y))
	{
		uint64_t pos = convert_pos(g, x, y);
		return ((g->game_array)[pos] == player);
	}
	else
	{
		return false;
	}
}

/** @brief Sprawdza, czy dany gracz może zająć nowy obszar, czyli
 * czy liczba aktualnie zajętych przez niego obszarów jest
 * mniejsza od dopuszczalnej liczby obszarów w posiadaniu
 * jednego gracza.
 * @param[in] g  			 	  – wskaźnik na strukturę gry
 * @param[in] player        	  – numer gracza
 * @return Wartość @p true, jeśli dany gracz może zająć nowy obszar,
 * a wartość @p false w przeciwnym wypadku
 */
static inline bool can_have_more_areas(gamma_t * g, uint32_t player)
{
	if(g != NULL)
	{
        return (((g->areas_array)[player-1]) < g->maximum_area_count);
	}
	else
	{
		return false;
	}
}

/** @brief Sprawdza, czy pole o podanych współrzędnych jest wolne
 * @param[in] g  			 	  – wskaźnik na strukturę gry
 * @param[in] x 	  		 	  – numer kolumny, w której znajduje się
 * 				        	   	    sprawdzane pole
 * @param[in] y 	  		 	  – numer wiersza, w którym znajduje się
 * 							   		sprawdzane pole
 * @return Wartość @p true, jeśli na polu o podanych współrzędnych nie znajduje się
 * żaden pionek, a @p false jeśli pole to jest zajęte przez jakiegoś gracza lub
 * wskaźnik @p g to NULL lub podane współrzędne są niepoprawne.
 */
static bool is_field_free(gamma_t * g, uint32_t x, uint32_t y)
{
	if(g != NULL && check_coordinates(g, x, y))
	{
		uint64_t pos = convert_pos(g, x, y);
		return ((g->game_array)[pos] == 0);
	}
	else
	{
		return false;
	}
}

/** @brief Łączy pole o podanych współrzędnych z sąsiadującymi polami
 * należącymi do tego samego gracza, o ile wskazane pole jest przez niego
 * zajęte - w przeciwnym wypadku połączenie pól nie następuje.
 * @param[in] g              	  – wskaźnik na strukturę gry
 * @param[in] x 			 	  – numer kolumny, w której 
 *							   	    znajduje się wskazane pole
 * @param[in] y 			 	  – numer wiersza, w którym
 *						   	   		znajduje się wskazane pole
 */
static void union_field(gamma_t * g, uint32_t x, uint32_t y)
{
	uint64_t pos = convert_pos(g, x, y);
	uint32_t current_player = (g->game_array)[pos];
	if(is_player_in_range(g, current_player))
	{
		if(x > 0 && is_on_field(g, x - 1, y, current_player))
		{
			connect_fields(g, pos, convert_pos(g, x - 1, y));
		}
		if(y > 0 && is_on_field(g, x, y - 1, current_player))
		{
			connect_fields(g, pos, convert_pos(g, x, y - 1));
		}
		if((x < g->width - 1) && is_on_field(g, x + 1, y, current_player))
		{
			connect_fields(g, pos, convert_pos(g, x + 1, y));
		}
		if((y < g->height - 1) && is_on_field(g, x, y + 1, current_player))
		{
			connect_fields(g, pos, convert_pos(g, x, y + 1));
		}
	}
}

/** @brief Sprawdza, czy pole o podanych współrzędnych jest izolowane
 * względem gracza o numerze @p player. Przez pole izolowane względem gracza
 * rozumiemy takie pole, że na żadnym z sąsiadujących z nim pól nie znajduje
 * się pionek należący do wskazanego gracza.
 * @param[in] g  			 	  – wskaźnik na strukturę gry
 * @param[in] player  		 	  – numer gracza
 * @param[in] x 	  		 	  – numer kolumny, w której znajduje się
 * 							   		docelowe pole
 * @param[in] y 	  		 	  – numer wiersza, w którym znajduje się
 * 									docelowe pole
 */
static bool is_field_isolated(gamma_t * g, uint32_t player, 
									uint32_t x, uint32_t y)
{
	bool check_upper = true;
	bool check_lower = true;
	bool check_right = true;
	bool check_left = true;
	if(y < g->height - 1)
	{
		check_upper = !is_on_field(g, x, y + 1, player);
	}
	if(y > 0)
	{
		check_lower = !is_on_field(g, x, y - 1, player);
	}
	if(x > 0)
	{
		check_left = !is_on_field(g, x - 1, y, player);
	}
	if(x < g->width - 1)
	{
		check_right = !is_on_field(g, x + 1, y, player);
	}
	return (check_left && check_upper && check_right && check_lower);
}

/** @brief Sprawdza, czy pole o podanych współrzędnych nowym wolnym
 * polem sąsiadującym z polem należącym do gracza @p player.
 * @param[in] g 				– wskaźnik na strukturę gry
 * @param[in] player 			– numer gracza
 * @param[in] x 				– numer kolumny, w której znajduje się
 * 								  sprawdzane pole
 * @param[in] y 				– numer wiersza, w którym znajduje się
 *								  sprawdzane pole
 * @return Wartość @p true, jeśli dane pole jest nowe, a @p false jeśli
 * pole to jest już uznane za wolne pole sąsiadujące z polem należącym
 * do gracza @p player.
 */
static inline bool is_new_free_neighbour(gamma_t * g, uint32_t player,
													uint32_t x, uint32_t y)
{
	return (is_field_free(g, x, y) && is_field_isolated(g, player, x, y));
}

/** @brief Aktualizuje pomocniczą tablicę wyznaczającą liczbę różnych graczy
 * sąsiadujących z polem, którego współrzędne przekazane zostały funkcji
 * wywołującej @ref update_neighbours_touching_fields.
 * @param[in] array[] 			– aktualizowana tablica
 * @param[in] *array_size 		– rozmiar tablicy
 * @param[in] target_value		– liczba, którą chcemy umieścić w tablicy
 */
static void update_temp_neighbour_array(uint32_t array[], uint32_t * array_size, 
									uint32_t target_value)
{
	bool different = true;
	for(uint32_t i = 0; i < *array_size; i++)
	{
		if(array[i] == target_value)
		{
			different = false;
		}
	}
	if(different)
	{
		array[*array_size] = target_value;
		(*array_size)++;
	}
}
					
/** @brief Aktualizuje liczbę wolnych pól przyległych do pól
 * zajętych przez gracza @p player.
 * @param[in] g 			– wskaźnik na strukturę gry
 * @param[in] player 		– numer gracza
 * @param[in] increase		– wartość @p true, gdy zwiększamy,
 *							  a @p false, gdy zmniejszamy
 *							  (liczbę z opisu funkcji)
 */
static inline void update_neighbours_array(gamma_t * g, uint32_t player, bool increase)
{
	if(increase)
	{
		(g->free_neighbours)[player-1]++;
	}
	else
	{
		(g->free_neighbours)[player-1]--;
	}
}

/** @brief Uaktualnia liczbę wolnych pól dla graczy, których
 * pola sąsiadują polem o podanych współrzędnych, zajmowanym w wyniku
 * wykonania złotego ruchu.
 * @param[in] g 			– wskaźnik na strukturę gry
 * @param[in] x 			– numer kolumny, w której znajduje się
 * 							  dane pole
 * @param[in] y 			– numer wiersza, w którym znajduje się
 *							  dane pole
 */
static void update_neighbours_touching_fields(gamma_t * g, uint32_t x, 
														uint32_t y)
{
	uint32_t temp_array[4] = {0};
	uint32_t temp_size = 0;
	if(x > 0 && check_coordinates(g, x - 1, y) && !is_field_free(g, x - 1, y))
	{
		uint32_t at_left = (g->game_array)[convert_pos(g, x - 1, y)];
		update_temp_neighbour_array(temp_array, &temp_size, at_left);
	}
	if((x < g->width - 1) && check_coordinates(g, x + 1, y) 
							&& !is_field_free(g, x + 1, y))
	{
		uint32_t at_right = (g->game_array)[convert_pos(g, x + 1, y)];
		update_temp_neighbour_array(temp_array, &temp_size, at_right);
	}
	if(y > 0 && check_coordinates(g, x, y - 1) && !is_field_free(g, x, y - 1))
	{
		uint32_t at_lower = (g->game_array)[convert_pos(g, x, y - 1)];
		update_temp_neighbour_array(temp_array, &temp_size, at_lower);
	}
	if((y < g->height - 1) && check_coordinates(g, x, y + 1) 
							&& !is_field_free(g, x, y + 1))
	{
		uint32_t at_upper = (g->game_array)[convert_pos(g, x, y + 1)];
		update_temp_neighbour_array(temp_array, &temp_size, at_upper);
	}
	for(uint32_t i = 0; i < temp_size; i++)
	{
		update_neighbours_array(g, temp_array[i], false);
	}
}

/** @brief Uaktualnia liczby wolnych sąsiednich pól graczy po wykonaniu
 * ruchu (zarówno zwykłego, jak i złotego) na pole o danych współrzędnych.
 * @param[in] g 		– wskaźnik na strukturę gry
 * @param[in] player 	– numer gracza, dla którego aktualizujemy
 * 						  liczbę wolnych sąsiednich pól
 * @param[in] x			– numer kolumny, w której znajduje się dane pole
 * @param[in] y 		– numer wiersza, w którym znajduje się dane pole
 * @param[in] increase 	– czy zwiększamy liczbę wolnych sąsiednich pól,
 * 						  wartość @p true jeśli tak, a @p false w 
 * 						  przeciwnym wypadku
 * @param[in] golden 	– czy aktualizacja dotyczy złotego ruchu, wartość
 * 						  @p true jeśli tak, a @p false w przeciwnym wypadku
 */
static void update_neighbours(gamma_t * g, uint32_t player,
							uint32_t x, uint32_t y, bool increase, bool golden)
{
	if(x > 0 && is_new_free_neighbour(g, player, x - 1, y))
	{
		update_neighbours_array(g, player, increase);
	}
	if((x < g->width - 1) && is_new_free_neighbour(g, player, x + 1, y))
	{
		update_neighbours_array(g, player, increase);
	}
	if(y > 0 && is_new_free_neighbour(g, player, x, y - 1))
	{
		update_neighbours_array(g, player, increase);
	}
	if((y < g->height - 1) && is_new_free_neighbour(g, player, x, y + 1))
	{
		update_neighbours_array(g, player, increase);
	}
	if(!golden)
	{
		update_neighbours_touching_fields(g, x, y);
	}
}

/** @brief Wypełnia tablicę pomocniczą. Funkcja pomocnicza, wywoływana przez
 * funkcję @ref player_areas_around.
 * @param[in] g 				– wskaźnik na strukturę gry
 * @param[in] player 			– numer gracza, scharakteryzowany w opisie funkcji
 * 								  @ref player_areas_around
 * @param[in] x 				– numer kolumny, w której znajduje się pole opisane
 * 								  w opisie funkcji @ref player_areas_around
 * @param[in] y 				– numer wiersza, w którym znajduje się pole opisane
 * 								  w opisie funkcji @ref player_areas_around
 * @param[in] array[] 			– wypełniana tablica
 * @param[in] *array_size 		– wskaźnik na rozmiar wypełnianej tablicy
 */
static void set_temp_fields_array(gamma_t * g, uint32_t player, uint32_t x,
	uint32_t y, uint64_t array[], uint32_t * array_size)
{
	if(x > 0 && is_on_field(g, x - 1, y, player))
	{
		array[*array_size] = convert_pos(g, x - 1, y);
		(*array_size)++;
	}
	if((x < g->width -1) && is_on_field(g, x + 1, y, player))
	{
		array[*array_size] = convert_pos(g, x + 1, y);
		(*array_size)++;
	}
	if((y < g->height - 1) && is_on_field(g, x, y + 1, player))
	{
		array[*array_size] = convert_pos(g, x, y + 1);
		(*array_size)++;
	}
	if(y > 0 && is_on_field(g, x, y - 1, player))
	{
		array[*array_size] = convert_pos(g, x, y - 1);
		(*array_size)++;
	}
}

/** @brief Zlicza ile różnych elementów znajduje się w tablicy
 * @p array. Funkcja pomocnicza, wywoływana przez funkcję
 * liczącą @ref player_areas_around
 * @param[in] g 			– wskaźnik na strukturę gry
 * @param[in] array[] 		– wskaźnik na pierwszy element tablicy
 * @param[in] array_size 	– rozmiar tablicy
 * @return Liczba różnych elementów w tablicy array
 */
static uint32_t count_areas(gamma_t * g, uint64_t array[], uint32_t array_size)
{
	uint32_t count_of_areas = 0;
	for(uint32_t i = 0; i < array_size; i++)
	{
		bool different = true;
		for(uint32_t j = 0; j < i && different; j++)
		{
			if(find_field(g, array[i]) == find_field(g, array[j]))
			{
				different = false;
			}
		}
		if(different)
		{
			count_of_areas++;
		}
	}	
	return count_of_areas;
}

/** @brief Liczy z iloma różnymi polami należącymi do gracza o numerze
 * @p player_id sąsiaduje pole o podanych współrzędnych.
 * @param[in] g 			– wskaźnik na strukturę gry
 * @param[in] player_id 	– numer gracza, którego ewentualne obszary 
 *							  przyległe do wskazanego pola chcemy sprawdzić
 * @param[in] x 			– numer kolumny, w której znajduje się wskazane pole
 * @param[in] y 			– numer wiersza, w którym znajduje się wskazane pole
 * @return Liczba różnych obszarów należących do gracza @p player_id, do
 * których przyległe jest pole o współrzędnych (@p x, @p y)
 */
static uint32_t player_areas_around(gamma_t * g, uint32_t player_id,
								 uint32_t x, uint32_t y)
{
	uint64_t temp_array[4];
	uint32_t array_size = 0;
	set_temp_fields_array(g, player_id, x, y, temp_array, &array_size);
	uint32_t p_areas_around = count_areas(g, temp_array, array_size);
	return p_areas_around;
}

/** @brief Aktualizuje parametry gracza oraz pola po wykonaniu zwykłego ruchu.
 * @param[in] g 			– wskaźnik na strukturę gry
 * @param[in] executor      – numer gracza wykonującego ruch
 * @param[in] x 			– numer kolumny, w której znajduje się pole, które
 * 							  zajął w ruchu gracz @p move_executor
 * @param[in] y 			– numer wiersza, w którym znajduje się pole, które
 * 							  zajął w ruchu gracz @p move_executor
 */
static inline void update_after_move(gamma_t * g, uint32_t executor,
				uint32_t x, uint32_t y)
{
	uint64_t current = convert_pos(g, x, y);

	update_neighbours(g, executor, x, y, true, false);
	(g->game_array)[current] = executor;

	g->busy_fields_count++;

    (g->occupied_fields_array)[executor-1]++;
    (g->areas_array)[executor-1]++;
    (g->areas_array)[executor-1] -= player_areas_around(g, executor, x, y);

	union_field(g, x, y);
}

bool gamma_move(gamma_t * g, uint32_t player, uint32_t x, uint32_t y)
{
	if(g != NULL && is_player_in_range(g, player)
		 && check_coordinates(g, x, y) && is_field_free(g, x, y))
	{
		bool check_areas = can_have_more_areas(g, player);
		bool isolated = is_field_isolated(g, player, x, y);
		if((check_areas && isolated) || !isolated)
		{
			update_after_move(g, player, x, y);
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}
}

uint64_t gamma_busy_fields(gamma_t * g, uint32_t player)
{
	if(g != NULL && is_player_in_range(g, player))
	{
        return (g->occupied_fields_array)[player-1];
	}
	else
	{
		return 0;
	}
}
                                                                           
uint64_t gamma_free_fields(gamma_t * g, uint32_t player)
{
	if(g != NULL && is_player_in_range(g, player))
	{
		if(can_have_more_areas(g, player))
		{
			uint64_t b_size = (uint64_t) g->width * (uint64_t) g->height;
			return (b_size - g->busy_fields_count);
		}
		else
		{
			return (g->free_neighbours)[player-1];
		}
	}
	else
	{
		return 0;
	}
}

/** @brief Aktualizuje powiązania między polami należącymi do
 * tego samego spójnego obszaru na planszy.
 * @param[in] g 			– wskaźnik na strukturę gry
 */
static void update_board_golden(gamma_t * g)
{
	if(g != NULL)
	{
		uint64_t board_size = (uint64_t) g->width * (uint64_t) g->height;
		uint64_t * first_array = g->parent_array;
		uint64_t * second_array = g->rank_array;
		for(uint64_t i = 0; i < board_size; i++)
		{
			first_array[i] = i;
			second_array[i] = 0;
		}
		for(uint32_t x = 0; x < g->width; x++)
		{
			for(uint32_t y = 0; y < g->height; y++)
			{
				union_field(g, x, y);
			}
		}
	}
}

/** @brief Aktualizuje parametry gracza po złotym ruchu.
 * Wywoływana na rzecz zarówno gracza wykonującego ten ruch, jak i
 * gracza, którego pionek zostaje zabrany z planszy gry.
 * @param[in] g 				– 	wskaźnik na strukturę gry
 * @param[in] player 			– 	numer gracza, którego parametry aktualizujemy
 * @param[in] x 				– 	numer kolumny, w której znajduje się pole, 
 *							  		na które wykonany został złoty ruch
 * @param[in] y 				– 	numer wiersza, w którym znajduje się pole, 
 *							  		na które wykonany został złoty ruch
 * @param[in] is_move_executor 	– 	czy aktualizujemy parametry gracza
 * 									wykonującego złoty ruch, wartość @p true, 
 *									gdy aktualizacja parametrów dotyczy gracza 
 *									wykonującego ruch, a @p false, gdy
 * 									aktualizujemy parametry gracza, którego
 *								 	pionek został zabrany z planszy.
 */
static inline void update_player_golden(gamma_t * g, uint32_t player,
 				 uint32_t x, uint32_t y, bool is_move_executor)
{
	uint32_t areas = player_areas_around(g, player, x, y);
	if(is_move_executor)
	{
        (g->areas_array)[player-1]++;
        (g->areas_array)[player-1] -= areas;
        (g->occupied_fields_array)[player-1]++;
        (g->golden_moves_array)[player-1] = false;
        update_neighbours(g, player, x, y, true, true);
	}
	else
	{
        (g->areas_array)[player-1] += areas;
        (g->areas_array)[player-1]--;
        (g->occupied_fields_array)[player-1]--;
        update_neighbours(g, player, x, y, false, true);
	}
}

/** @brief Sprawdza, czy w wyniku wykonania złotego ruchu
 * ani gracz wykonujący ten ruch, ani gracz, którego pionek jest
 * zdejmowany z planszy, nie wejdzie w posiadanie zbyt dużej liczby
 * obszarów przekraczającej tę dopuszczalną ustaloną w strukturze
 * @p g gry.
 * @param[in] g 		– wskaźnik na strukturę gry
 * @param[in] executor  – numer gracza wykonującego złoty ruch
 * @param[in] victim 	– numer gracza, którego pole zostaje zajęte
 * 						  przez gracza wykonującego złoty ruch
 * @param[in] x 		– numer kolumny, w której znajduje się pole,
 *						  na które wykonywany jest złoty ruch
 * @param[in] y 		– numer kolumny, w której znajduje się pole, 
 *						  na które wykonywany jest złoty ruch
 * @return Wartość @p true, gdy ruch jest legalny i żaden z graczy
 * nie będzie posiadał zbyt wielu obszarów, a @p false, gdy ruch
 * nie jest legalny, bo któryś z graczy przekroczy dozwoloną
 * liczbę obszarów
 */
static bool area_check_golden(gamma_t * g, uint32_t executor,
		uint32_t victim, uint32_t x, uint32_t y)
{
	uint32_t area_limit = g->maximum_area_count;


	uint32_t exec_around = player_areas_around(g, executor, x, y);
	uint32_t vic_around = player_areas_around(g, victim, x, y);

	uint32_t exec_areas = (g->areas_array)[executor-1] + 1 - exec_around;
	uint32_t vic_areas = (g->areas_array)[victim-1] + vic_around - 1;

	return (exec_areas <= area_limit && vic_areas <= area_limit);
}

/** @brief Funkcja wykrywająca przypadek, gdy gracz chce zająć w złotym ruchu
 * pionek odizolowany od obszarów przez siebie posiadanych, gdy nie ma
 * możliwości zajęcia kolejnego obszaru. W szczególnych przypadkach
 * znacznie skraca czas wykonania funkcji exec_golden
 * (gdy ruch nie może zostać wykonany z opisanego powodu).
 * @param[in] g 		– wskaźnik na strukturę gry
 * @param[in] executor  – numer gracza chcącego wykonać
 *						  złoty ruch
 * @param[in] x   		– numer kolumny, w której znajduje się pole,
 *						  które chce przejąć gracz @p executor
 * @param[in] y 		– numer wiersza, w którym znajduje się pole,
 *						  które chce przejąć gracz @p executor
 * @return Wartość @p true, gdy nie zachodzi opisany warunek
 * uniemożliwiający wykonanie złotego ruchu, a wartość @p false,
 * gdy zachodzi i wówczas złoty ruch jest niemożliwy.
 */
static bool initial_golden_check(gamma_t * g, uint32_t executor,
									uint32_t x, uint32_t y)
{
	if(is_field_isolated(g, executor, x, y) && !can_have_more_areas(g, executor))
	{
		return false;
	}
	else
	{
		return true;
	}
}

/** @brief Odpowiada za wykonanie złotego ruchu, uprzednio
 * sprawdzając, czy jest legalny względem zarówno gracza
 * @p executor, jak i gracza, którego pole chce zająć
 * gracz @p executor.
 * @param[in] g 		– wskaźnik na strukturę gry
 * @param[in] executor 	– numer gracza wykonującego złoty ruch
 * @param[in] x 		– numer kolumny, w której znajduje się pole,
 * 						  na które gracz @p executor chce wykonać 
 *						  złoty ruch
 * @param[in] y 		– numer wiersza, w którym znajduje się pole,
 * 						  na które gracz @p executor chce wykonać
 *						  złoty ruch.
 * @return Wartość @p true, gdy ruch jest legalny; jednocześnie jest on
 * wykonany w ciele funkcji, a wartość @p false, gdy ruch nie jest legalny i
 * wówczas nic się nie dzieje
 */
static bool exec_golden(gamma_t * g, uint32_t executor,
								 uint32_t x, uint32_t y)
{
	if(g != NULL && !is_field_free(g, x, y)
				&& !is_on_field(g, x, y, executor))
	{
		uint64_t pos = convert_pos(g, x, y);
		uint32_t victim = (g->game_array)[pos];
		bool quick_check_executor = initial_golden_check(g, executor, x, y);
		if(!quick_check_executor)
		{
			return false;
		}
		(g->game_array)[pos] = 0;
		update_board_golden(g);
		if(area_check_golden(g, executor, victim, x, y))
		{
			update_player_golden(g, executor, x, y, true);
			update_player_golden(g, victim, x, y, false);
			(g->game_array)[pos] = executor;
			union_field(g, x, y);
			return true;
		}
		else
		{
			(g->game_array)[pos] = victim;
			union_field(g, x, y);
			return false;
		}
	}
	else
	{
		return false;
	}
}

bool gamma_golden_move(gamma_t * g, uint32_t player, uint32_t x, uint32_t y)
{
	if(g != NULL && is_player_in_range(g, player) && check_coordinates(g, x, y)
											&& gamma_golden_possible(g, player))
	{
		return exec_golden(g, player, x, y);
	}
	else
	{
		return false;
	}
}

/** @brief Sprawdza czy gracz @p player może wykonać złoty ruch
 * na pole o podanych współrzędnych.
 * @param[in] g - wskaźnik na strukturę gry
 * @param[in] x - numer kolumny, w której znajduje się
 * sprawdzane pole
 * @param[in] y - numer wiersza, w którym znajduje się
 * sprawdzane pole
 * @param[in] player - numer gracza, dla którego
 * sprawdzamy możliwość wykonania złotego ruchu na dane pole
 * @return Wartość @p true, gdy gracz @p player może wykonać
 * złoty ruch na dane pole, a @p false w przeciwnym wypadku.
 */
static inline bool check_field_golden_possible(gamma_t * g, 
					uint32_t x, uint32_t y, uint32_t player)
{
	uint64_t current_pos = convert_pos(g, x, y);
	uint32_t player_at_field = (g->game_array)[current_pos];
	(g->game_array)[current_pos] = player;
	update_board_golden(g);
	uint32_t areas_f = player_areas_around(g, player, x, y);
	uint32_t areas_s = player_areas_around(g, player_at_field, x, y);
	bool first = ((g->areas_array)[player-1] + 1 - areas_f
					<= g->maximum_area_count);
	bool second =((g->areas_array)[player_at_field-1] + areas_s - 1
					<= g->maximum_area_count);
	(g->game_array)[current_pos] = player_at_field;
	update_board_golden(g);
	return (first && second);
}

/** @brief Iteruje po polach zajętych przez graczy innych niż @p player,
 * dla każdego pola sprawdza, czy możliwe jest dla gracza wykonanie na nie
 * złotego ruchu.
 * @param[in] g - wskaźnik na strukturę gry
 * @param[in] player - numer gracza
 * @param[in] possible - wskaźnik na zmienną logiczną informującą,
 * czy gracz @p player może wykonać złoty ruch
 */
static inline void iterate_board(gamma_t * g, uint32_t player, bool * possible)
{
	for(uint32_t i = 0; i < g->width && !(*possible); i++)
	{
		for(uint32_t j = 0; j < g->height && !(*possible); j++)
		{
			if(!(is_field_free(g, i, j) || is_on_field(g, i, j, player)
								|| is_field_isolated(g, player, i, j)))
			{
				*possible = check_field_golden_possible(g, i, j, player);
			}
		}
	}
}

bool gamma_golden_possible(gamma_t * g, uint32_t player)
{
	if(g != NULL && is_player_in_range(g, player))
	{
        bool from_player = (g->golden_moves_array)[player-1];
        if(!from_player)
        {
        	return false;
        }
		bool from_board = (g->busy_fields_count > (g->occupied_fields_array)[player-1]);
		if(from_board && can_have_more_areas(g, player))
		{
			return true;
		}
		else if(from_board)
		{
			bool possible = false;
			iterate_board(g, player, &possible);
			return possible;
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}
}

/** @brief Tworzy tablicę zawierającą stan gry,
 * gdy liczba graczy w grze nie przekracza 9.
 * @param[in] g 		– wskaźnik na strukturę gry
 * @return Wskaźnik na utworzoną tablicę znaków
 */	
static char * board_less_than_ten(gamma_t * g)
{
	uint64_t h = (uint64_t) g->height;
	uint64_t w = (uint64_t) g->width;
	uint64_t array_size = h * (w + 1) + 1;
	char * board = malloc(array_size * sizeof(char));
	if(board == NULL)
	{
		return NULL;
	}
	board[array_size - 1] = '\0';
	for(uint64_t i = w; i < array_size-1; i += (w + 1))
	{
		board[i] = '\n';
	}
	uint64_t i = h * w;
	while(i > 0)
	{
		i--;
		uint32_t field_p = (g->game_array)[i];
		uint64_t y = i / w;
		uint64_t x = i - y * w;
		uint64_t new_index = (h-1-y)*(w+1) + x;
		if(field_p == 0)
		{
			board[new_index] = '.';
		}
		else
		{
			board[new_index] = field_p + '0';
		}
	}
	return board;
}

/** @brief Zlicza ile cyfr w zapisie dziesiętnym
 * posiada liczba.
 * @param[in] integer     – badana liczba
 */
static uint32_t count_digits_of(uint32_t integer)
{
	uint32_t log = 0;
	while(integer > 0)
	{
		log++;
		integer /= 10;
	}
	return log;
}

/** @brief Wypełnia pole w tablicy znaków numerem gracza
 * większego niż 9.
 * @param[in] board  		– wskaźnik na tablicę
 * @param[in] player 		– numer gracza
 * @param[in] start 		– indeks (początek pola)
 * @param[in] end 			– indeks (koniec pola)
 */
static inline void set_board_space(char * board, uint32_t player, 
								uint64_t start, uint64_t end)
{
	if(player == 0)
	{
		board[start] = '.';
		for(uint64_t i = start+1; i <= end; i++)
		{
			board[i] = ' ';
		}
	}
	else
	{
		uint32_t digits = count_digits_of(player);
		uint64_t i = start+digits;
		while(i > start)
		{
			i--;
			uint32_t cur_digit = player%10;
			board[i] = cur_digit + '0';
			player /= 10;
		}
		for(uint64_t i = start+digits; i <= end; i++)
		{
			board[i] = ' ';
		}
	}
}

/** @brief Tworzy tablicę zawierającą stan gry,
 * gdy liczba graczy w grze przekracza 9.
 * @param[in] g 		– wskaźnik na strukturę gry
 * @return Wskaźnik na utworzoną tablicę znaków
 */								
static char * board_more_than_nine(gamma_t * g)
{
	uint64_t b_h = (uint64_t) g->height;

	uint64_t w = (uint64_t) g->width;
	uint64_t log = (uint64_t) count_digits_of(g->players_count);

	uint64_t b_w = w*log + w;

	uint64_t a_size = b_h * b_w;

	char * board = malloc(a_size * sizeof(char));
	if(board == NULL)
	{
		return NULL;
	}
	board[a_size-1] = '\0';

	for(uint64_t i = w*log + (w-1); i < a_size-1; i+= b_w)
	{
		board[i] = '\n';
	}

	for(uint64_t i = 0; i < b_h; i++)
	{
		for(uint64_t j = 0; j < b_w - 1; j = j+log+1)
		{
			uint32_t x = (uint32_t) j/(log+1);
			uint64_t pos = i*b_w + j;
			uint32_t field_p = (g->game_array)[convert_pos(g, x, g->height-1-i)];
			set_board_space(board, field_p, pos, pos+log-1);
		}
		for(uint64_t j = 0; j < b_w-1; j++)
		{
			if((j+1)%(log+1) == 0)
			{
				board[i*b_w + j] = ' ';
			}	
		}
	}
	return board;
}

char * gamma_board(gamma_t * g)
{
	if(g != NULL)
	{

		if(g->players_count <= 9)
		{
			return board_less_than_ten(g);
		}
		else
		{
			return board_more_than_nine(g);
		}
	}
	else
	{
		return NULL;
	}
}

uint32_t get_board_width(gamma_t *g)
{
	return g->width;
}

uint32_t get_board_height(gamma_t *g)
{
	return g->height;
}

uint32_t get_players_count(gamma_t *g)
{
	return g->players_count;
}