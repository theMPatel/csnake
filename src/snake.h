/********************************************************
 * 														*
 * This is the basic header file for the snake program	*
 * It defines some important variables for the game 	*
 *														*
 ********************************************************/

// This is the special variable name that says this file
// has been defined already. It is a good way to make sure
// that space is not allocated for this file twice
#ifndef __SNAKE_H_

// The apple that we want to get
#define APPLE			'@'

// The default starting length of the snake
#define START_LEN		4

// I am assuming this is the delay at the
// start of the game
#define DEFAULT_DELAY	200000

// You won the game after you get here
#define MAX_SCORE	100

// Define the colors for the pieces
#define RESETATTR    0

#define RED		0x1
#define YELLOW	0x13
#define WHITE	0x17

// Move the cursor location to a specific place
extern int _ERROR, MAX_ROW, MAX_COL;

// Keys to define the general movment direction
#define MOVE_KEYS {'w', 'a', 's', 'd'}

// Enumeration for magic numbers aka mapping semantics
typedef enum {UP, LEFT, DOWN, RIGHT, NUM_KEYS} direction_t;

// Struct that represents a peice of the
// snake and it's position
typedef struct coor_t
{
	int row, col;
} coor_t;

typedef struct game_t
{
	int high_score;
	int current_score;
	double best_time;
	time_t start_time;
	coor_t apple;

} game_t;

typedef struct snake_t
{
	int len;
	int direction;
	int speed;
	coor_t body[MAX_SCORE+START_LEN];
	coor_t *tail;

} snake_t;

typedef struct two_d_char_arr_t
{
	char *grid_ptr;
	int row, col;

} two_d_char_arr_t;

// ------ Function prototypes -----------
// Considered good practice
void signal_setup(int signal, void (*handler)(int));
void signal_handler(int signal __attribute__((unused)));

// Game specifics
void setup_game(struct winsize *current_scr, struct game_t *game_state, snake_t *snake);
void goto_coor(int x, int y);
void clear_scr(void);
void setup_scr(coor_t *apple, snake_t *snake);

#endif