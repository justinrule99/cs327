// HEADER FOR FILEPARSER.C
// FUNCTION DECLARATIONS ONLY

#ifndef PARSER
#define PARSER

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// THESE SHOULD NEVER CHANGE
extern char* ranks;
extern char* suits;

extern int line_number;


// HEARTS AND DIAMONDS ARE RED
// SPADES AND CLUBS ARE BLACK

// src must be 'w' or 1-7
// dest must be 'f' 1-7
typedef struct move {
	char src;
	char dest;
	char turn_over;
	char waste_reset;
} move_t;


typedef struct card {
	char separator;
	char r;
	char s;
} card_t;


struct node {
  card_t card;
  struct node* next;
};

typedef struct node* nodeptr;

// should change all arrays to pointers so they use the heap and can be accessed after function calls
typedef struct file_state {
	int turn; // how many cards to flip over at once
	char game_version[50]; // unlimited, etc
	int limit; // limit if not using unlimited gamemode
	char clubs_f; // foundations for clubs, hearts, etc
	char diamonds_f;
	char hearts_f;
	char spades_f;

	// array of stacks for tableau cards
	nodeptr s_cols_covered[7];
	nodeptr s_cols[7];


	// all cards in the game
	card_t all_cards[52];

	int covered_cards;

	// represents waste | stock
	card_t* waste_stock;

	nodeptr waste;
	nodeptr stock;
	// should make a stack for waste, queue for stock	

	int separator_idx;

	int num_waste;
	int num_stock;

	// max: 100 moves (stores all based on format, not gametime validity)
	move_t* all_moves;
	int num_moves;
	int move_idx;

} state_t;



int isEmpty(nodeptr stack);

void push(nodeptr *stack, card_t card);

card_t pop(nodeptr *stack);

card_t get(nodeptr *stack);

int is_suit(char c, int suit_num);

int card_equals(card_t card1, card_t card2);

int contains(char arr[], int arr_size, char c);

int is_rank(char c);

int is_src(char c);

int is_dest(char c);

int read_whitespace(FILE* file);

int read_int(FILE* file);

card_t read_card(FILE* file);

void parse_rules(FILE* file, int section, state_t* state);

void parse_foundations(FILE* file, state_t* state);

void parse_tableau(FILE* file, state_t* state);

void parse_stock(FILE* file, state_t* state);

void parse_moves(FILE* file, state_t* state);

void read_file(FILE* file, int include_moves, int print_output, state_t* my_file);

typedef struct advance_switches {
    int moves; // -1 means inf
    FILE* outfile; // null means stdout
    int exchange; // 0 means human readable
} switches_t;

int is_next_rank(char rank1, char rank2);

char* to_string(card_t card);

int is_alt_suit(card_t c1, card_t c2);

void error(move_t move, state_t* my_file);

void add_to_foundation(card_t to_add, move_t move, nodeptr *where_to_pop, state_t* my_file);

void add_to_foundation_val(card_t to_add, move_t move, nodeptr *where_to_pop, state_t my_file);

void process_move(move_t move, state_t* my_file);

state_t process_move_val(move_t move, state_t my_file);

void process_all_moves(state_t* my_file, switches_t* switches);

#endif