#include <stdio.h>
#include <stdlib.h>
#include "fileparser.h"

/* game sections:
	RULES
	FOUNDATIONS: buildup for winning (right side)
	TABLEAU: playing area, 7 columns (7-1)
	STOCK: cards to be played, turned over 1/3 at a time
	MOVES: moves to be played
*/

// checks if card is in all_cards
// unfinished
int contains_card(card_t card, card_t all_cards[52]) {
	for (int i = 0; i < 52; i++){
		if (card_equals(card, all_cards[i])) return 1;
	}
	return 0;
}


int main(int argc, char* argv[]) {
	FILE* file;
	state_t* my_file = malloc(sizeof(state_t));


	if (argc == 2) {
		printf("%s\n", argv[1]);
		file = fopen(argv[1], "r");

		if (file) {
			read_file(file, 0, 1, my_file);
		}

	} else {
		read_file(stdin, 0, 1, my_file);
	}
	return 0;
}
