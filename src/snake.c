/*
	This is the first implementation of snake in C.
	the project exists so that I have method to learn
	C and because I like the game snake!
*/

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <time.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include "snake.h"

int _ERROR = 0;
int MAX_ROW = 0;
int MAX_COL = 0;
int MOVE_DELAY = DEFAULT_DELAY;

// Define the exit semantics
void signal_setup(int signal, void (*handler)(int))
{

	// Set up the sigaction struct
	struct sigaction action;

	// This is needed to modify the set of signals that
	// are blocked. Not necessary from my point of view
	// as of right now. 

	// *** This is import for porting this to another OSes ***
	sigemptyset(&action.sa_mask);

	// Set the function that is going to do the processing
	// when this signal is received
	action.sa_handler = handler;

	// Sigaction provides a cool way to retain the previous
	// action (default or not) via a struct that is passed
	// as the third param in the below func call
	sigaction(signal, &action, NULL);
}

void signal_handler(int signal __attribute__((unused)))
{
	clear_scr();
	printf("Received signal %d exiting...\n", signal);

	// Get return the terminal back to it's original state
	int success = system("stty sane");
	exit(success);
}

void set_text_color(int color )
{
	if (!color) {
		printf("\e[%dm", color);
	} else {
		printf("\e[%d;%dm", color & 0x10 ? 1 : 0, (color & 0xF) + 30);
	}
}

void goto_coor(int x, int y)
{
	printf("\e[%d;%dH", x, y);
}

void clear_scr(void)
{
	printf("\e[H\e[J");
}	

void get_border(void)
{
	int x, y;

	for (x = 0; x < MAX_ROW; ++x) {
		for (y=0; y < MAX_COL; ++y){

			if (x==0 || x == MAX_ROW-1){
				printf("*");
				goto_coor(x,y);
			}else if (y == 0 || y == MAX_COL-1) {
				printf("*");
				goto_coor(x,y);
			}

		}
	}
}

// This function will setup the screen to make sure
// that terminal inputs will get blocked from view
void setup_scr(void)
{
	system ("stty cbreak -echo stop u");
	clear_scr();
}

void get_rand_coor(coor_t *rand_coor)
{
	rand_coor->row = rand() % MAX_ROW;
	rand_coor->col = rand() % MAX_COL;
}

void two_d_arr_setter(int x, int y, two_d_char_arr_t *arr, char val)
{
	char *grid_ptr = arr->grid_ptr;
	int row = arr->row;
	int col = arr->col;

	if (x > row || y > col) {
		return;
	}

	*(grid_ptr + (x * col) + y) = val;
}

char two_d_arr_getter(int x, int y, two_d_char_arr_t *arr)
{
	char *grid_ptr = arr->grid_ptr;
	int row = arr->row;
	int col = arr->col;

	if (x > row || y > col){
		return '~';
	}

	return *(grid_ptr + (x * col) + y);
}

int setup_game(struct winsize *current_scr, struct game_t *game_state,
	snake_t *snake)
{

	if (current_scr == NULL || game_state == NULL || snake == NULL){
		_ERROR = 1;
		return _ERROR;
	}

	MAX_ROW = current_scr->ws_row;
	MAX_COL = current_scr->ws_col;

	// Initialize the struct for the 2d array
	struct two_d_char_arr_t *grid = malloc(sizeof(struct two_d_char_arr_t));
	char *grid_ptr = (char*) malloc(MAX_ROW*MAX_COL*sizeof(*grid_ptr));
	
	grid->grid_ptr = grid_ptr;

	game_state->game_grid = grid;
	game_state->high_score = 0;
	game_state->current_score = 0;
	game_state->best_time = 0;

	// Fill the grid with blanks
	int i, j;
	for (i=0; i<MAX_ROW; ++i) {
		for (j=0; j<MAX_COL; ++j) {
			two_d_arr_setter(i, j, grid, ' ');
		}
	}

	struct coor_t start_gold;
	get_rand_coor(&start_gold);

	// Set the gold value in the grid so we know where it is
	two_d_arr_setter(start_gold.row, start_gold.col, game_state->game_grid, APPLE);

	snake->head = snake->body;
	coor_t *arr = snake->head;

	arr->row = MAX_ROW/2-1;
	arr->col = MAX_COL/2-1;
	arr++;

	for (i=1; i<START_LEN; i++){
		arr->row = (arr-1)->row;
		arr->col = (arr-1)->col+1;
		arr++;
	}

	snake->head = arr;
	// Print the apple
	goto_coor(start_gold.row, start_gold.col);
	set_text_color(YELLOW);
	printf("%c", APPLE);
	set_text_color(RESETATTR);




	return 0;
}

int main (void)
{
	// Get the current window size
	// This way we allow the player to have a larger landscape to play
	// if they start out with a larger terminal
	struct winsize current_scr;
	ioctl(0, TIOCGWINSZ, &current_scr);
	
	struct game_t game_state;

	signal_setup(SIGINT, signal_handler);
	signal_setup(SIGHUP, signal_handler);
	signal_setup(SIGTERM, signal_handler);

	char selection;
	snake_t snake;

	setup_game(&current_scr, &game_state, &snake);

	return system("stty sane");
}