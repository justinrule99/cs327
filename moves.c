#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fileparser.h"

// used to create ./advance, which takes in proper switches and processes moves 

switches_t* switches;

void make_output(FILE* outfile, state_t* my_file);

void make_output_x(FILE* outfile, state_t* my_file);


char** make_tableau_column(int column, int hide, state_t* my_file) {
	char** tableau;
	nodeptr tableau_stack[7];

	// array of strings, 13 2 char strings long
	tableau = malloc(13 * sizeof(char*));
	for (int i = 0; i < 13; i++){
		tableau[i] = malloc(2 * sizeof(char));
	}

	int t_idx = 0;
	int num_uncovered = 0;
	int num_covered = 0;

	while (!isEmpty(my_file->s_cols_covered[column])) {
		card_t cur_card = pop(&my_file->s_cols_covered[column]);
		if (hide) {
			tableau[t_idx++] = "##";
		} else {
			tableau[t_idx++] = to_string(cur_card);
		}
		num_covered++;
	}
	if (!hide) {
		card_t sep;
		sep.separator = 1;
		tableau[t_idx++] = to_string(sep);
	}

	while (!isEmpty(my_file->s_cols[column])) {
		card_t cur_card = pop(&my_file->s_cols[column]);
		push(&tableau_stack[column], cur_card);
		tableau[t_idx++] = to_string(cur_card);
		num_uncovered++;
	}

	// reverse the uncovered part of the array
	for (int i = 0; i < num_uncovered; i++){
		tableau[num_covered++] = to_string(pop(&tableau_stack[column]));
	}

	return tableau;
} 

// sends output to human readable format OR input format to stdout or file
void make_output(FILE* outfile, state_t* my_file) {

	// cdhs
	fprintf(outfile, "Foundations\n");
	fprintf(outfile, "%c%c ", my_file->clubs_f, 'c');
	fprintf(outfile, "%c%c ", my_file->diamonds_f, 'd');
	fprintf(outfile, "%c%c ", my_file->hearts_f, 'h');
	fprintf(outfile, "%c%c\n", my_file->spades_f, 's');
	fprintf(outfile, "Tableau\n");

	char** t[7];
	for (int i = 0; i < 7; i++){
		t[i] = make_tableau_column(i, 1, my_file);
	}
	int t1_idx;

	for (int i = 0; i < 13; i++){
		// ROWS
		for (int j = 0; j < 7; j++){
			// COLUMNS
			// top row: END tab1 -> tab7
			if (!is_rank(t[j][i][0]) && t[j][i][0] != '#') {
				t[j][i] = "..";
			}
			fprintf(outfile, "%s ", t[j][i]);
		}
		fprintf(outfile, "\n");
	}

	fprintf(outfile, "Waste top\n");
	if (!isEmpty(my_file->waste)) {
		card_t waste_top = pop(&my_file->waste);
		fprintf(outfile, "%c%c\n", waste_top.r, waste_top.s); 
	} else {
		fprintf(outfile, "(empty)\n");
	}
}

void make_output_x(FILE* outfile, state_t* my_file) {
	// if -x flag given, output in input format

	fprintf(outfile, "RULES:\n");
	fprintf(outfile, "turn %d\n", my_file->turn);
	if (my_file->limit) {
		fprintf(outfile, "limit %d\n", my_file->limit);
	} else {
		fprintf(outfile, "unlimited\n");
	}

	fprintf(outfile, "FOUNDATIONS:\n");
	fprintf(outfile, "\t%cc\n\t%cd\n\t%ch\n\t%cs\n\n", my_file->clubs_f, my_file->diamonds_f, my_file->hearts_f, my_file->spades_f);

	fprintf(outfile, "TABLEAU:\n");
	// col 7-1 with separator
	char** t[7];
	for (int i = 0; i < 7; i++){
		t[i] = make_tableau_column(i, 0, my_file);
	}

	for (int i = 6; i >= 0; i--){
		for (int j = 0; j < 13; j++){
			if (!is_rank(t[i][j][0]) && t[i][j][0] != '|') {
				continue;
			}
			fprintf(outfile, "%s ", t[i][j]);
		}
		fprintf(outfile, "\n");
	}

	fprintf(outfile, "STOCK:\n");

	nodeptr reverse_waste;
	int count = 0;
	int idx = 0;

	// to reverse the waste stack
	while (!isEmpty(my_file->waste)) {
		card_t cur_card = pop(&my_file->waste);
		push(&reverse_waste, cur_card);
		count++;
	}

	for (int i = 0; i < count; i++){
		card_t cur_card = pop(&reverse_waste);

		fprintf(outfile, "%s ", to_string(cur_card));
		idx++;
		if (idx%5 == 0) fprintf(outfile, "\n");
	}
	fprintf(outfile, "| ");

	while (!isEmpty(my_file->stock)) {
		card_t cur_card = pop(&my_file->stock);
		fprintf(outfile, "%s ", to_string(cur_card));
		idx++;
		if (idx%5 == 0) fprintf(outfile, "\n");
	}

	fprintf(outfile, "\nMOVES:\n");
}

// switches:
// -m N : at most N moves should be played
// -o file : game config out written to file (stdout if omitted)
// -x : output in exchange format (hum readable if omitted)
int main(int argc, char** argv) {

	FILE* file;
	FILE* out;
	state_t* my_file = malloc(sizeof(state_t));
	int file_as_arg = 0;

	switches = malloc(sizeof(switches_t)); 
	switches->moves = -1;

	for (int i = 1; i < argc; i++){
		if (strcmp(argv[i], "-m") == 0) {
			// next arg should be N
			switches->moves = atoi(argv[++i]);
		} else if (strcmp(argv[i], "-o") == 0) {
			// next arg should be file
			switches->outfile = fopen(argv[++i], "w");
		} else if (strcmp(argv[i], "-x") == 0) {
			switches->exchange = 1;
		} else {
			file = fopen(argv[i], "r");
			if (file) {
				file_as_arg = 1;
			}
		}
	}

	if (!switches->outfile) {
		switches->outfile = stdout;
	}

	if (!switches->exchange) {
		switches->exchange = 0;
	}

	if (!file_as_arg) {
		read_file(stdin, 1, 0, my_file);
	} else {
		read_file(file, 1, 0, my_file);
	}


	if (switches->moves < 0) {
		switches->moves = 100;
	}

	for (int i = 0; i < switches->moves; i++){
		if (i >= my_file->num_moves) {
			break;
		}

		// processes moves into ONE state_t struct (keep it this way)
		process_move(my_file->all_moves[i], my_file);
	}

	if (switches->exchange) {
		make_output_x(switches->outfile, my_file);
	} else {
		fprintf(switches->outfile, "Processed %d moves, all valid\n", my_file->move_idx);
		make_output(switches->outfile, my_file);
	}

	return 0;
}
