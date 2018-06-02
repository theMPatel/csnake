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

void set_text_bkgrd(int color)
{
	if (!color) {
		printf("\e[%dm", color);
	} else {
		printf("\e[%d;%dm", color & 0x10 ? 1 : 0, (color & 0xF) + 40);
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

void write_border(void)
{
	int x,y;
	set_text_color(RED);
	for (x = 0; x < MAX_ROW; ++x) {
		for (y=0; y < MAX_COL; ++y) {
			
			if (x == 0 || x == MAX_ROW-1) {
				goto_coor(x, y);
				printf("*");

			} else if (y == 0 || y == MAX_COL-1) {
				goto_coor(x, y);
				printf("*");
			}

		}
	}
	set_text_color(RESETATTR);
}

// This function will setup the screen to make sure
// that terminal inputs will get blocked from view
void setup_scr(coor_t *apple, snake_t *snake)
{
	system ("stty cbreak -echo stop u");
	clear_scr();
	write_border();

	set_text_color(YELLOW);
	goto_coor(apple->row, apple->col);
	printf("%c", APPLE);
	set_text_color(RESETATTR);

	set_text_bkgrd(WHITE);

	for (coor_t *ptr=snake->body; ptr<=snake->tail; ptr++) {
		goto_coor(ptr->row, ptr->col);
		printf(" ");
	}

	set_text_bkgrd(RESETATTR);
}

void get_rand_coor(coor_t *rand_coor)
{
	// A border is defined as:
	// 
	// ******************************
	// *							*
	// 
	// The actual screen space is going to be
	// between the two asterisks hence -2
	// and +1 to get the coordinate in front
	// of the first asterisk
	rand_coor->row = (rand() % (MAX_ROW-2))+1;
	rand_coor->col = (rand() % (MAX_COL-2))+1;
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
	
	game_state->high_score = 0;
	game_state->current_score = 0;
	game_state->best_time = 0;

	struct coor_t start_gold;
	get_rand_coor(&start_gold);
	game_state->current_gold = start_gold;

	// Set up the snake body so that we get a starting position
	snake->tail = snake->body;
	coor_t *arr = snake->tail;

	arr->row = MAX_ROW/2-1;
	arr->col = MAX_COL/2-1;

	for (int i=1; i<START_LEN; ++i) {
		arr++;
		arr->row = (arr-1)->row;
		arr->col = (arr-1)->col+1;
	}

	// Move the head pointer forward to the current head forward
	snake->tail = arr;
	snake->direction = LEFT;

	return 0;
}

int check_coordinates(coor_t *a, coor_t *b){
	return a->row == b->row && a->col == b->col;
}

int check_wall_collision(coor_t *head)
{

	if (head->row == 0 || head->row == MAX_ROW) {
		return 1;

	} else if (head->col == 0 || head->col == MAX_COL){
		return 1;

	}

	return 0;
}

int check_self_collision(snake_t *snake, coor_t *start, coor_t *pos)
{
	// This function is useful to check the collision of a specified
	// point and the snake body. 
	// Normally we want to check the head position
	// and all the other body parts
	// 
	// When creating a new tail object
	// it is useful to be able to check the collision of the potential
	// new point and the rest of the body
	// 
	for(; start<=snake->tail; ++start){
		if(check_coordinates(start, pos)){
			return 1;
		}
	}
	return 0;
}

int check_collision(snake_t *snake)
{
	return check_wall_collision(snake->body) || 
		check_self_collision(snake, snake->body+1, snake->body);

}

int check_yum_apple(snake_t *snake, coor_t *apple)
{
	if (snake->body->row == apple->row 
		&& snake->body->col == apple->col) {
		return 1;
	}

	return 0;
}

void extend_snake(snake_t *snake)
{
	coor_t *prev_tail = snake->tail-1;
	coor_t *curr_tail = snake->tail;

	// This is the next tail
	snake->tail++;
	coor_t directions[4];

	// up, down, right, left
	// We need to know how to pick the next tail position
	// We can add to the current tail in 4 different directions
	// At least one will be the same as prev_tail
	// some may collide with the snake self
	// some may collide with the walls
	// At least one point *should* always work
	directions[0].row = curr_tail->row-1;
	directions[0].col = curr_tail->col;

	directions[1].row = curr_tail->row+1;
	directions[1].col = curr_tail->col;

	directions[2].row = curr_tail->row;
	directions[2].col = curr_tail->col-1;

	directions[3].row = curr_tail->row;
	directions[3].col = curr_tail->col+1;

	for (coor_t *ptr=&directions[0]; ptr<=&directions[0]+4; ++ptr) {

		if (!check_coordinates(ptr, prev_tail) && 
			!check_wall_collision(ptr) && 
			!check_self_collision(snake, snake->body, ptr)){

			snake->tail->row = ptr->row;
			snake->tail->col = ptr->col;
			break;
		}
	}


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
	char key_chars[NUM_KEYS] = MOVE_KEYS;

	setup_game(&current_scr, &game_state, &snake);
	setup_scr(&game_state.current_gold, &snake);
	char keypress;

	do{
		coor_t next_apple;
		get_rand_coor(&next_apple);

		do {

		} while (!check_collision(&snake));



	} while( keypress == 'y' );
	


	set_text_color(RESETATTR);

	clear_scr();
	printf("%c\n",(char)getchar() );
	return system("stty sane");
}