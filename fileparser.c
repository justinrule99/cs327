#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fileparser.h"

// A COLLECTION OF FILE PARSING FUNCTIONS, MOSTLY USED FOR PART 1	
// plan: keep parse_section functions in main.c , move all helpers here
// this file will also do part2 functionality

#ifndef PARSEC
#define PARSEC

char* ranks = "A23456789TJQK";
char* suits = "cdhs";

int line_number = 1;

/* NOTE: This implementation of a stack is based off lecture notes from Week 6
    coms327notesspring2020/Week6/stack.c
      Credit goes to Dr. Miner
*/
int isEmpty(nodeptr stack)
{
  return !stack;
}

void push(nodeptr *stack, card_t card){
  /*
    (1) Make a new node
    (2) Put item inside
    (3) Point it to the current top
    (4) Change the top
  */
  nodeptr thing = malloc(sizeof(*thing));
  thing->card = card;
  thing->next = *stack;
  *stack = thing;
}

card_t pop(nodeptr *stack){
  /*
    (0) Sanity check - stack not empty 
    (1) store top value in temporary long var
    (2) Change top to stack->next
    (3) Free the old top
  */
  if (0==*stack) {
    fprintf(stderr, "Error: pop on an empty stack\n");
    exit(1);
  }
  card_t temp = (*stack)->card;
  nodeptr oldtop = *stack;
  *stack = (*stack)->next;
  free(oldtop);
  return temp;
}

card_t get(nodeptr *stack) {
    if (0==*stack) {
        fprintf(stderr, "Error: get on an empty stack\n");
        exit(1);
    }
	return (*stack)->card;
}




// cdhs
int is_suit(char c, int suit_num) {
	if (!suit_num) {
		return c == 'c' || c == 'd' || c == 'h' || c == 's';
	}
	switch (suit_num) {
		case 1: return c == 'c';
		case 2: return c == 'd';
		case 3: return c == 'h';
		case 4: return c == 's';
		default: return 0;
	}
}

int card_equals(card_t card1, card_t card2) {
	return (card1.r == card2.r) && (card1.s == card2.s) && (!card1.separator); 
}


int contains(char arr[], int arr_size, char c) {
	for (int i = 0; i < arr_size; i++){
		if(arr[i] == c) {
			return 1;
		}
	}
	return 0;
}


int is_rank(char c) {
	return contains(ranks, 13, c);
}

int is_src(char c) {
	int num = 0;
	if (c >= '1' && c < '8') num = 1;
	return (c == 'w') || num;
}

int is_dest(char c) {
	int num = 0;
	if (c >= '1' && c < '8') num = 1;
	return (c == 'f') || num;
}

// returns int: number of newlines read
int read_whitespace(FILE* file) {
	int num_newlines = 0;


	while (1) {
		char cur = fgetc(file);

		if (cur == '#'){
			while (fgetc(file) != '\n');
			line_number++;
			num_newlines++;
			continue;
		}
		if (cur == ' ') continue;
		if (cur == '\t') continue;
		if (cur == '\n'){
			line_number++; // will get thrown off by fgets later?
			num_newlines++;
			continue;	
		} 
		if (cur == '\r') continue;
		
		ungetc(cur, file);
		break;
	}

	return num_newlines;
}

int read_int(FILE* file) {
	int num = fgetc(file) - '0';
	if (num < 0 || num > 9) {
		fprintf(stderr, "Error on line %d: Not a valid number 0-9\n", line_number);
		exit(0);
	}
	return num;
}

card_t read_card(FILE* file) {
	card_t card;

	char r = fgetc(file);
	card.separator = 0;


	if (r == '|') {
		// no card to return 
		card.separator = 1;
		card.r = 0;
		card.s = 0;
		return card;
	}

	if (is_rank(r)) {
		card.r = r;
	} else {
		fprintf(stderr, "Error on line %d: %c not a valid rank\n", line_number, r);
		exit(0);
	}

	char s = fgetc(file);
	if (is_suit(s, 0)) {
		card.s = s;
	} else {
		fprintf(stderr, "Error on line %d: %c not a valid suit\n", line_number, s);
		exit(0);
	}

	return card;
}

/* --------------------------

	THIS STARTS THE SECTION PARSING FUNCTIONS
	FILE WITH MAIN NEED TO CALL read_file() WHILE HANDLING CMD LINE ARGS
	RESULT IS READ INTO my_file STRUCT

	----------------------------*/

void parse_rules(FILE* file, int section, state_t* state) {	
	// parse rules section, assumes right after colon
	char turn[4];
	char limit[9];
	while (1) {
		read_whitespace(file);
		fgets(turn, 5, file);
		if (!(strcmp(turn, "turn"))) {
			// get next one, then number
			fgetc(file);
			int file_turn = read_int(file);
			state->turn = file_turn;

			if (file_turn == 3 || file_turn == 1) {
				// turn correct, look for limit
				read_whitespace(file);
				char u = fgetc(file);
				if (u == 'u') {
					ungetc(u, file);
					// look for unlimited
					fgets(limit, 10, file);
					if (!(strcmp(limit, "unlimited"))) {
						for (int i = 0; i < 9; i++){
							state->game_version[i] = limit[i];
						}
					} else {
						fprintf(stderr, "Error on line %d: Invalid game type!\n", line_number);
						exit(0);
					}
				} else {
					ungetc(u, file);
					// assuming limit N
					fgets(limit, 6, file);
					if (!(strcmp(limit, "limit"))) {
						// correct
						for (int i = 0; i < 5; i++){
							state->game_version[i] = limit[i];
						}
						fgetc(file);
						int lim = read_int(file);
						state->limit = lim;
					} else {
						fprintf(stderr, "Error on line %d: Invalid game type!\n", line_number);
						exit(0);
					}
				}
			} else {
				fprintf(stderr, "Error on line %d: Invalid number of cards per turn!\n", line_number);
				exit(0);				
			}

			return;
		}
		if (feof(file)) {
			break;
		}
	}
	printf("error: doesn't contain \"turn\"\n");
}

void parse_foundations(FILE* file, state_t* state) {
	int suit_num = 1;

	while (1) {
		read_whitespace(file);
		// here at first non-whitesapace char
		char r = fgetc(file);
		if (is_rank(r) || r == '_') {
			// now check if is a valid suit
			char s = fgetc(file);
			if (is_suit(s, suit_num)) {
				// suits must be in order: cdhs
				switch (suit_num) {
					case 1: state->clubs_f = r;
							break;
					case 2: state->diamonds_f = r;
							break;
					case 3: state->hearts_f = r;
							break;
					case 4: state->spades_f = r;
							break;
					default: break;
				}
				suit_num++;

				if (suit_num > 4) break;
				continue;
			} else {
				fprintf(stderr, "Error on line %d: Invalid suit\n", line_number);
				exit(0);
			}

		} else {
			fprintf(stderr, "Error on line %d: Invalid rank\n", line_number);
			exit(0);
		}
		break;
	}
}

void parse_tableau(FILE* file, state_t* state) {
	// reverse order: 7-1
	int column = 7;
	int position = 0;
	int covered = 1;
	card_t cur_card; 

	while (1) {
		if (read_whitespace(file)){
			column--;
			position = 0;
			covered = 1;
			if (column < 0) break;
		}
		cur_card = read_card(file);

		if (cur_card.separator) {
			// separator, not valid card
			covered = 0;
			continue;
		}
		if (covered) {
			push(&state->s_cols_covered[column], cur_card);
			state->covered_cards++;
		} else {
			push(&state->s_cols[column], cur_card);
		}
	}
}

void parse_stock(FILE* file, state_t* state) {
	card_t cur_card;
	int i = 0;
	int in_waste = 1;

	nodeptr local_stock;

	while (1) {
		read_whitespace(file);

		char c = fgetc(file);
		if (c == 'M') {
			ungetc(c, file);
			break;
		} 
		ungetc(c, file);
		cur_card = read_card(file);
		if (cur_card.separator) {
			state->separator_idx = i;
			// separator card, don't push to anything
			in_waste = 0;
		}
		// push to waste

		if (in_waste) {
			push(&state->waste, cur_card);
			state->num_waste++;
		} else {
			// need to reverse this stack 
			push(&local_stock, cur_card);
			// also push to stock
			if (!cur_card.separator) state->num_stock++;
		}
	}

	// this reverses the stock section from order read in, for ease of use later
	card_t* arr_stock = malloc(state->num_stock * sizeof(card_t));
	for (int i = 0; i < state->num_stock; i++){
		arr_stock[i] = pop(&local_stock);
	}

	for (int i = 0; i < state->num_stock; i++){
		push(&state->stock, arr_stock[i]);
	}


}

void parse_moves(FILE* file, state_t* state) {
	// moves section: . OR 'r' OR 'w->#' OR 'f->#'
	int move_idx = 0;
	state->all_moves = malloc(100 * sizeof(move_t));

	while (1) {
		read_whitespace(file);


		char c = fgetc(file);
		move_t mv;
		mv.src = 0;
		mv.dest = 0;
		mv.turn_over = 0;
		mv.waste_reset = 0;

		if (c == EOF) return;

		if (c == '.') {
			mv.turn_over = 1;
			state->all_moves[move_idx] = mv;
			state->num_moves++;
			move_idx++;
		} else if (c == 'r') {
			mv.waste_reset = 1;
			state->all_moves[move_idx] = mv;
			state->num_moves++;
			move_idx++;
		} else {
			// validate src char: 'w' or 1-7
			if (is_src(c)){
				mv.src = c;
				if (fgetc(file) == '-' && fgetc(file) == '>') {
					// valid arrow
					char n = fgetc(file);
					if (is_dest(n)) {
						mv.dest = n;
						state->all_moves[move_idx] = mv;
						state->num_moves++;
						move_idx++;
					} else {
						fprintf(stderr, "Error: invalid destination for move on line %d, got char %c\n", line_number, n);
						exit(0);
					}
				}
			} else {
				fprintf(stderr, "Error: invalid source for move on line %d, got char %c\n", line_number, c);
				exit(0);
			}
		}
	}

}

void read_file(FILE* file, int include_moves, int print_output, state_t* my_file) {

	int section = 0;
	int isValid = 1;
	my_file->covered_cards = 0;
	my_file->turn = 0;
	my_file->num_waste = 0;
	my_file->num_stock = 0;
	my_file->move_idx = 0;
	my_file->num_moves = 0;
	my_file->waste_stock = malloc(53 * sizeof(card_t));
	char section_heading[12];

	// fill all_cards with all cards
	int card_num = 0;
	for (int i = 0; i < 13; i++){
		for (int j = 0; j < 4; j++){
			card_t card;
			card.r = ranks[i];
			card.s = suits[j];
			card.separator = 0;
			my_file->all_cards[card_num] = card;
			card_num++;
		}
	}

	while (!feof(file)) {
		// skips whitespace
		read_whitespace(file);

		char c = fgetc(file);
		if (c == ':') {
			section++;
			if (section == 1) {
				parse_rules(file, section, my_file);
			} else if (section == 2) {
				parse_foundations(file, my_file);
			} else if (section == 3) {
				parse_tableau(file, my_file);
			} else if (section == 4) {
				parse_stock(file, my_file);
			} else if (section == 5 && include_moves) {
				parse_moves(file, my_file);
			}
		}
	}

	// will disable when calling from moves.c
	if (print_output) {
		printf("Input file is valid\n");
		printf("%d covered cards\n", my_file->covered_cards);
		printf("%d stock cards\n", my_file->num_stock);
		printf("%d waste cards\n", my_file->num_waste);
	}
	fclose(file);
}


/*********
 * THIS BEGINS THE PART 2 FUNCTIONS TO ADVANCE BASED ON GIVEN MOVES
 * does not create proper output or deal with switches, still must be handled in moves.c
 * requires switches object to operate (default: no switches)
************************/




// returns 1 if rank1 is one less than rank2
int is_next_rank(char rank1, char rank2) {
    int idx1 = strchr(ranks, rank1)-ranks;
    int idx2 = strchr(ranks, rank2)-ranks;

    return idx1-idx2 == 1;
}

char* to_string(card_t card) {
    // card as rs
    char* str = calloc(2, 2 * sizeof(char));
    char* bar = "| ";
    if (card.separator){
        strcat(str, bar);
        return str;
    }

    strcat(str, &card.r);
    return str;
}

// returns 1 if suit is a different color (for putting on tableau)
int is_alt_suit(card_t c1, card_t c2) {
    if (c1.s == 'c' || c1.s == 's') return c2.s == 'd' || c2.s == 'h';
    return c2.s == 'c' || c2.s == 's';
}

// must print in same format as infile (1->6)
void error(move_t move, state_t* my_file) {
    fprintf(stdout, "Move %d is illegal (%c->%c)\n",my_file->move_idx+1, move.src, move.dest);
    exit(0);
}

void error_val(move_t move, state_t my_file) {
    fprintf(stdout, "Move %d is illegal (%c->%c)\n",my_file.move_idx+1, move.src, move.dest);
    exit(0);
}



// helper: get_foundation, get_waste, etc
void add_to_foundation(card_t to_add, move_t move, nodeptr *where_to_pop, state_t* my_file) {
    printf("card adding to h found: %c%c\n", to_add.r, to_add.s);
    printf("but, h found is: %c\n", my_file->hearts_f);
    if (to_add.s == 'c') {
        if (to_add.r == 'A' && my_file->clubs_f == '_') {
            my_file->clubs_f = 'A';
            pop(where_to_pop);
            my_file->move_idx++;
            return;
            // delete to_add from waste
        } else if (is_next_rank(to_add.r, my_file->clubs_f)) {
            // increment rank on foundation
            my_file->clubs_f = ranks[(strchr(ranks,my_file->clubs_f)-ranks)+1];
            pop(where_to_pop);
            my_file->move_idx++;
            return;
        }

    }

    if (to_add.s == 'd') {
        if (to_add.r == 'A' && my_file->diamonds_f == '_') {
            my_file->diamonds_f = 'A';
            pop(where_to_pop);
            my_file->move_idx++;

            return;
            // delete to_add from waste
        } else if (is_next_rank(to_add.r, my_file->diamonds_f)) {
            // increment rank on foundation
            my_file->diamonds_f = ranks[(strchr(ranks,my_file->diamonds_f)-ranks)+1];
            pop(where_to_pop);
            my_file->move_idx++;
            return;
        }
    }

    if (to_add.s == 'h') {
        // trying to add to hearts foundation:

        if (to_add.r == 'A' && my_file->hearts_f == '_') {
            my_file->hearts_f = 'A';
            pop(where_to_pop);
            my_file->move_idx++;
            return;
            // delete to_add from waste
        } else if (is_next_rank(to_add.r, my_file->hearts_f)) {
            // increment rank on foundation
            my_file->hearts_f = ranks[(strchr(ranks,my_file->hearts_f)-ranks)+1];
            pop(where_to_pop);
            my_file->move_idx++;
            return;
        }
    }

    if (to_add.s == 's') {
        if (to_add.r == 'A' && my_file->spades_f == '_') {
            my_file->spades_f = 'A';
            pop(where_to_pop);
            my_file->move_idx++;
            return;
        } else if (is_next_rank(to_add.r, my_file->spades_f)) {
            // increment rank on foundation
            my_file->spades_f = ranks[(strchr(ranks,my_file->spades_f)-ranks)+1];
            pop(where_to_pop);
            my_file->move_idx++;
            return;
        }
    }

    error(move, my_file);
}

// this should be bare bones, no unnecessary output generation
// ie: leave make_output version in moves.c
void process_move(move_t move, state_t* my_file) {
    // do copy here to new object

    // turn over limit R
    if (move.turn_over) {
        if (my_file->turn == 1) {
            if (!isEmpty(my_file->stock)) {
                card_t turn = pop(&my_file->stock);
                push(&my_file->waste, turn);
                my_file->move_idx++;
                return;
            } else {
                fprintf(stderr, "Move %d is illegal (.)\n",my_file->move_idx+1);
//                make_output(switches->outfile, my_file);
                exit(0);
            }

        } else if (my_file->turn == 3) {
            for (int i = 0; i < 3; i++){
                if (!isEmpty(my_file->stock)) {
                    card_t turn = pop(&my_file->stock);
                    push(&my_file->waste, turn);
                } else {
                    fprintf(stderr, "Move %d is illegal (.)\n",my_file->move_idx+1);
//                    make_output(switches->outfile, my_file);
                    exit(0);
                }
            }
            my_file->move_idx++;
            return;

        }

    }

    if (move.waste_reset) {
        if (!(strcmp(my_file->game_version, "unlimited") == 0 || my_file->limit > 0)) {
            fprintf(stderr, "Move %d is illegal (r)\n",my_file->move_idx+1);
//            make_output(switches->outfile, my_file);
            exit(0);
        }

        if (!isEmpty(my_file->stock)) {
            fprintf(stderr, "Move %d is illegal (r)\n",my_file->move_idx+1);
//            make_output(switches->outfile, my_file);
            exit(0);
        }

        while (!isEmpty(my_file->waste)) {
            card_t card = pop(&my_file->waste);
            push(&my_file->stock, card);
        }

        if (my_file->limit) {
            my_file->limit--;
        }
        my_file->move_idx++;
        return;
    }

    // waste -> foundation
    if (move.src == 'w' && move.dest == 'f') {
        card_t waste_card = get(&my_file->waste);
        add_to_foundation(waste_card, move, &my_file->waste, my_file);
        return;
    }

    // tableau->foundation
    if (move.dest == 'f') {
        int src = move.src-'0'-1;

        // if popping makes s_cols empty, push (pop s_cols_covered), then push the original popped
        card_t to_add = pop(&my_file->s_cols[src]);
        if (isEmpty(my_file->s_cols[src]) && !(isEmpty(my_file->s_cols_covered[src]))) {
            push(&my_file->s_cols[src], pop(&my_file->s_cols_covered[src]));
        }
        push(&my_file->s_cols[src], to_add);

        add_to_foundation(my_file->s_cols[move.src-'0'-1]->card, move, &my_file->s_cols[move.src-'0'-1] ,my_file);
        return;
    }

    // waste->tableau
    if (move.src == 'w') {
        // guarantees waste -> tableau
        int dest = move.dest-'0'-1;
        card_t waste_card = pop(&my_file->waste);

        if (isEmpty(my_file->s_cols[dest])) {
            if (waste_card.r == 'K') {
                push(&my_file->s_cols[dest], waste_card);
                my_file->move_idx++;
                return;
            } else {
                error(move, my_file);
            }
        }

        card_t tab_card = get(&my_file->s_cols[dest]);

        if (is_next_rank(tab_card.r, waste_card.r) && is_alt_suit(tab_card, waste_card)) {
            // push onto tableau
            push(&my_file->s_cols[dest], waste_card);
            my_file->move_idx++;
            return;

        } else {
            error(move, my_file);
        }
    }

    // tableau->tablueau
    int src = move.src-'0'-1;
    int dest = move.dest-'0'-1;
    nodeptr temp_stack;

    if (isEmpty(my_file->s_cols[dest])) {
        int i;
        for (i = 1; i < 13; i++){
            if (isEmpty(my_file->s_cols[src])){
                error(move, my_file);
            }
            card_t card_in_src = pop(&my_file->s_cols[src]);
            push(&temp_stack, card_in_src);
            if (card_in_src.r == 'K') {
                break;
            }
        }

        for (int j = 0; j < i; j++){
            card_t popped = pop(&temp_stack);
            push(&my_file->s_cols[dest], popped);
        }
        // uncovers the next one
        if (isEmpty(my_file->s_cols[src])) {
            if (!isEmpty(my_file->s_cols_covered[src])) {
                push(&my_file->s_cols[src], pop(&my_file->s_cols_covered[src]));
            }
        }

        my_file->move_idx++;
        return;
    }

    card_t card_in_dest = get(&my_file->s_cols[dest]);

    int i;
    for (i = 1; i < 13; i++){
        if (isEmpty(my_file->s_cols[src])){
            error(move, my_file);
        }
        card_t card_in_src = pop(&my_file->s_cols[src]);
        push(&temp_stack, card_in_src);
        if (is_next_rank(card_in_dest.r, card_in_src.r) && is_alt_suit(card_in_src, card_in_dest)) {
            break;
        }
    }

    for (int j = 0; j < i; j++){
        card_t popped = pop(&temp_stack);
        push(&my_file->s_cols[dest], popped);
    }

    // uncovers a card if necessary
    // also check if theres nothing to pop from covered
    if (isEmpty(my_file->s_cols[src])) {
        if (!isEmpty(my_file->s_cols_covered[src])) {
            push(&my_file->s_cols[src], pop(&my_file->s_cols_covered[src]));
        }
    }
    my_file->move_idx++;

    // return my_file
}




// PASS BY VALUE EXPERIMENT
// NUKE IF IT DOESN'T WORK

// this one can still be ref (never called outside this file)
void add_to_foundation_val(card_t to_add, move_t move, nodeptr *where_to_pop, state_t my_file) {
    if (to_add.s == 'c') {
        if (to_add.r == 'A' && my_file.clubs_f == '_') {
            my_file.clubs_f = 'A';
            pop(where_to_pop);
            my_file.move_idx++;
            return;
            // delete to_add from waste
        } else if (is_next_rank(to_add.r, my_file.clubs_f)) {
            // increment rank on foundation
            my_file.clubs_f = ranks[(strchr(ranks,my_file.clubs_f)-ranks)+1];
            pop(where_to_pop);
            my_file.move_idx++;
            return;
        }

    }

    if (to_add.s == 'd') {
        if (to_add.r == 'A' && my_file.diamonds_f == '_') {
            my_file.diamonds_f = 'A';
            pop(where_to_pop);
            my_file.move_idx++;

            return;
            // delete to_add from waste
        } else if (is_next_rank(to_add.r, my_file.diamonds_f)) {
            // increment rank on foundation
            my_file.diamonds_f = ranks[(strchr(ranks,my_file.diamonds_f)-ranks)+1];
            pop(where_to_pop);
            my_file.move_idx++;
            return;
        }
    }

    if (to_add.s == 'h') {
        if (to_add.r == 'A' && my_file.hearts_f == '_') {
            my_file.hearts_f = 'A';
            pop(where_to_pop);
            my_file.move_idx++;
            return;
            // delete to_add from waste
        } else if (is_next_rank(to_add.r, my_file.hearts_f)) {
            // increment rank on foundation
            my_file.hearts_f = ranks[(strchr(ranks,my_file.hearts_f)-ranks)+1];
            pop(where_to_pop);
            my_file.move_idx++;
            return;
        }
    }

    if (to_add.s == 's') {
        if (to_add.r == 'A' && my_file.spades_f == '_') {
            my_file.spades_f = 'A';
            pop(where_to_pop);
            my_file.move_idx++;
            return;
        } else if (is_next_rank(to_add.r, my_file.spades_f)) {
            // increment rank on foundation
            my_file.spades_f = ranks[(strchr(ranks,my_file.spades_f)-ranks)+1];
            pop(where_to_pop);
            my_file.move_idx++;
            return;
        }
    }

    error_val(move, my_file);
}

// this should be bare bones, no unnecessary output generation
// ie: leave make_output version in moves.c
// issue: any stack operations persist throughout program and thus change my_file
state_t process_move_val(move_t move, state_t my_file) {
    // do copy here to new object (for stacks only)
    // changes in retval == changes in my_file
    state_t retval = my_file;

    // turn over limit R
    if (move.turn_over) {
        if (my_file.turn == 1) {
            if (!isEmpty(my_file.stock)) {
                card_t turn = pop(&my_file.stock);
                push(&my_file.waste, turn);
                my_file.move_idx++;
                return my_file;
            } else {
                fprintf(stderr, "Move %d is illegal (.)\n",my_file.move_idx+1);
//                make_output(switches->outfile, my_file);
                exit(0);
            }

        } else if (my_file.turn == 3) {
            for (int i = 0; i < 3; i++){
                if (!isEmpty(my_file.stock)) {
                    card_t turn = pop(&my_file.stock);
                    push(&my_file.waste, turn);
                } else {
                    fprintf(stderr, "Move %d is illegal (.)\n",my_file.move_idx+1);
//                    make_output(switches->outfile, my_file);
                    exit(0);
                }
            }
            my_file.move_idx++;
            return my_file;

        }

    }

    if (move.waste_reset) {
        if (!(strcmp(my_file.game_version, "unlimited") == 0 || my_file.limit > 0)) {
            fprintf(stderr, "Move %d is illegal (r)\n",my_file.move_idx+1);
//            make_output(switches->outfile, my_file);
            exit(0);
        }

        if (!isEmpty(my_file.stock)) {
            fprintf(stderr, "Move %d is illegal (r)\n",my_file.move_idx+1);
//            make_output(switches->outfile, my_file);
            exit(0);
        }

        while (!isEmpty(my_file.waste)) {
            card_t card = pop(&my_file.waste);
            push(&my_file.stock, card);
        }

        if (my_file.limit) {
            my_file.limit--;
        }
        my_file.move_idx++;
        return my_file;
    }

    // waste -> foundation
    if (move.src == 'w' && move.dest == 'f') {
        card_t waste_card = get(&my_file.waste);
        add_to_foundation(waste_card, move, &my_file.waste, &my_file);
        return my_file;
    }

    // tableau->foundation
    // returning my_file should not change it
    // here, s_cols[i] has been changed from other reference (why???)
    if (move.dest == 'f') {
        int src = move.src-'0'-1;

        // if popping makes s_cols empty, push (pop s_cols_covered), then push the original popped
        card_t to_add = pop(&my_file.s_cols[src]);
        if (isEmpty(my_file.s_cols[src]) && !(isEmpty(my_file.s_cols_covered[src]))) {
            push(&my_file.s_cols[src], pop(&my_file.s_cols_covered[src]));
        }
        push(&my_file.s_cols[src], to_add);

        add_to_foundation(my_file.s_cols[src]->card, move, &my_file.s_cols[src] ,&my_file);
        return my_file;
    }

    // waste->tableau
    if (move.src == 'w') {
        // guarantees waste -> tableau
        int dest = move.dest-'0'-1;
        card_t waste_card = pop(&my_file.waste);

        if (isEmpty(my_file.s_cols[dest])) {
            if (waste_card.r == 'K') {
                push(&my_file.s_cols[dest], waste_card);
                my_file.move_idx++;
                return my_file;
            } else {
                error_val(move, my_file);
            }
        }

        card_t tab_card = get(&my_file.s_cols[dest]);

        if (is_next_rank(tab_card.r, waste_card.r) && is_alt_suit(tab_card, waste_card)) {
            // push onto tableau
            push(&my_file.s_cols[dest], waste_card);
            my_file.move_idx++;
            return my_file;

        } else {
            error_val(move, my_file);
        }
    }

    // tableau->tablueau
    int src = move.src-'0'-1;
    int dest = move.dest-'0'-1;
    nodeptr temp_stack;

    if (isEmpty(my_file.s_cols[dest])) {
        int i;
        for (i = 1; i < 13; i++){
            if (isEmpty(my_file.s_cols[src])){
                error_val(move, my_file);
            }
            card_t card_in_src = pop(&my_file.s_cols[src]);
            push(&temp_stack, card_in_src);
            if (card_in_src.r == 'K') {
                break;
            }
        }

        for (int j = 0; j < i; j++){
            card_t popped = pop(&temp_stack);
            push(&my_file.s_cols[dest], popped);
        }
        // uncovers the next one
        if (isEmpty(my_file.s_cols[src])) {
            if (!isEmpty(my_file.s_cols_covered[src])) {
                push(&my_file.s_cols[src], pop(&my_file.s_cols_covered[src]));
            }
        }

        my_file.move_idx++;
        return my_file;
    }

    card_t card_in_dest = get(&my_file.s_cols[dest]);

    int i;
    for (i = 1; i < 13; i++){
        if (isEmpty(my_file.s_cols[src])){
            error_val(move, my_file);
        }
        card_t card_in_src = pop(&my_file.s_cols[src]);
        push(&temp_stack, card_in_src);
        if (is_next_rank(card_in_dest.r, card_in_src.r) && is_alt_suit(card_in_src, card_in_dest)) {
            break;
        }
    }

    for (int j = 0; j < i; j++){
        card_t popped = pop(&temp_stack);
        push(&my_file.s_cols[dest], popped);
    }

    // uncovers a card if necessary
    // also check if theres nothing to pop from covered
    if (isEmpty(my_file.s_cols[src])) {
        if (!isEmpty(my_file.s_cols_covered[src])) {
            push(&my_file.s_cols[src], pop(&my_file.s_cols_covered[src]));
        }
    }
    my_file.move_idx++;

    return my_file;
}





void process_all_moves(state_t* my_file, switches_t* switches) {
    for (int i = 0; i < switches->moves; i++) {
        if (i >= my_file->num_moves) break;
        process_move(my_file->all_moves[i], my_file);
    }
}

#endif