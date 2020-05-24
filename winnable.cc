// C++ source for part 3

#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <vector>
#include <bits/stdc++.h>
#include "GameState.h"
extern "C" {
#include "fileparser.h"
}

using namespace std;

typedef struct winnable_switches {
    int max_moves;
    bool use_cache;
    bool force_foundations;
    bool verbose_output;
} switches_winnable;

switches_winnable* win_switches;

// internal use only, assume moves are syntactically valid
// as stingy as possible to reduce recursive calls in stack
bool isMoveValid(GameState state, move_t move) {
    // check if waste to found is valid:
    card_t card_to_move;

    if (move.turn_over) {
        return state.stock.size() >= state.turnNum;
    }

    if (move.src == 'w') {
        if (state.waste.empty()) return false;
        card_to_move = state.waste.top();
        if (move.dest == 'f') { // waste->found
        } else { // waste->tableau
            int idx = move.dest-'0'-1;
            if (state.cols[idx].empty()) return false;
            return is_next_rank(state.cols[idx].top().r, card_to_move.r) && is_alt_suit(card_to_move, state.cols[idx].top());
        }
    } else if (move.dest == 'f'){ // for now: tableau->found
        int idx = (move.src-'0'-1);
        card_to_move = state.cols[move.src-'0'-1].top();
    }

    // moves anything to foundations
    for (int i = 0; i < 4; i++) {
        if (card_to_move.s == suits[i] && is_next_rank(card_to_move.r, state.foundations[i])) return true;
    }

    return false;
}

vector<move_t> getValidMoves(GameState state) {
    vector<move_t> valid_moves;
    vector<move_t> all_possible_moves;

    if (!state.waste.empty()) {
        // not given that w->f is valid
        move_t waste_f;
        waste_f.src = 'w';
        waste_f.dest = 'f';
        waste_f.turn_over = 0;
        waste_f.waste_reset = 0;

        move_t waste_tab;
        waste_tab.src = 'w';
        all_possible_moves.push_back(waste_f);
    }

    move_t tab_found;
    tab_found.dest = 'f';
    tab_found.turn_over = 0;
    tab_found.waste_reset = 0;

    move_t waste_tab;
    waste_tab.src = 'w';
    waste_tab.turn_over = 0;
    waste_tab.waste_reset = 0;

    move_t turn;
    turn.turn_over = 1;
    if (isMoveValid(state, turn)) {
        valid_moves.push_back(turn);
    }

    // handles tableau->foundations
    for (int i = 0; i < 7; i++) {
        int j = i+1;
        tab_found.src = '0' + j;
        waste_tab.dest = '0' + j;
        if (!state.cols[i].empty()) {
            all_possible_moves.push_back(tab_found);
        }
        all_possible_moves.push_back(waste_tab);
    }

    for (int i = 0; i < all_possible_moves.size(); i++) {
        if (isMoveValid(state, all_possible_moves.at(i))) {
            valid_moves.push_back(all_possible_moves.at(i));
        }
    }

    return valid_moves;
}


int sequenceLength = 0;
FILE* fileout;
// recursively: do valid moves and check if the game is won

stack<GameState> prevGameStates;
queue<move_t> winningMoves;
int totalPositions = 0;

void printWinInfo() {
    // print entire winning sequence to stdout
    cout << "# Game is winnable within " << win_switches->max_moves-1 << " moves.\n";
    cout << "Analyzed " << totalPositions << " positions.\n";
    int startSize = winningMoves.size();
    for (int i = 0; i < startSize; i++) {
        move_t move = winningMoves.front();
        winningMoves.pop();
        if (move.turn_over) {
            fprintf(stdout, ".\n");
        } else if(move.waste_reset) {
            fprintf(stdout, "r\n");
        } else {
            fprintf(stdout, "%c->%c\n", move.src, move.dest);
        }
    }
    exit(0);
}


void searchWin(GameState state, vector<move_t> moves) {
    if (state.foundations[0] == 'K' && state.foundations[1] == 'K' && state.foundations[2] == 'K' && state.foundations[3] == 'K') {
        printWinInfo();
    }
    // game won: at most 1 waste, no covered, no stock
    bool allEmpty = true;
    if (state.waste.size() <= 1 && state.stock.empty()) {
        for (int i = 0; i < 7; i++) {
            if (!state.cols_covered[i].empty()) {
                allEmpty = false;
            }
        }
    } else allEmpty = false;
    if (allEmpty) {
        printWinInfo();
    }

    if (moves.size() == 0) {
        winningMoves.pop();
        return;
    };

    // how do we make this faster?
    for (int i = 0; i < moves.size(); i++) {
        // do each move recursively, do everything from state
        move_t one_move = moves.at(i);

//        cout << state.waste.empty() << "\n";
//        cout << i+1 << " of " << moves.size() << "\n";
//        cout << one_move.src << "->" << one_move.dest << "\n";
//        for (int j = 0; j < moves.size(); ++j) {
//            cout << "\t: " << moves.at(j).src << "->" << moves.at(j).dest << "\n";
//        }
//        cout << "\n";

        GameState newState = state.processMove(one_move, state);
        totalPositions++;
        if (totalPositions % 100000 == 0) {
            cout << "Evaluated " << totalPositions << " positions...\n";
        }
        winningMoves.push(one_move);
        prevGameStates.push(newState);
        vector<move_t> newMoves = getValidMoves(newState);

        // this represents recursion depth
        sequenceLength++;

        if (sequenceLength < win_switches->max_moves) {
            // newState should reflect AFTER processing ONE move
            searchWin(newState, newMoves);
            prevGameStates.pop();
        } else {
            prevGameStates.pop();
            winningMoves.pop();
            sequenceLength--;
        }
    }
    winningMoves.pop();
    sequenceLength--;
}


int main(int argc, char** argv) {
    // assume all moves and input is valid, want to get game state as reflected by current structs (port)
    win_switches = new winnable_switches;
    state_t* my_file = new state_t;
    bool file_as_arg = 0;
    FILE* file;
    fileout = fopen("out.txt", "w");

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-m") == 0) {
            win_switches->max_moves = atoi(argv[++i])+1;
        } else if (strcmp(argv[i], "-c") == 0) {
            win_switches->use_cache = true;
        } else if (strcmp(argv[i], "-f") == 0) {
            win_switches->force_foundations = true;
        } else if (strcmp(argv[i], "-v") == 0) {
            win_switches->verbose_output = true;
        } else {
            // assume arg is file
            file = fopen(argv[i], "r");
        }
    }

    if (!win_switches->max_moves) {
        win_switches->max_moves = 1000;
    }
    if (!file) {
        file = stdin;
    }

    switches_t* moves_switches = new switches_t;
    moves_switches->moves = 10;
    moves_switches->outfile = stdout;
    moves_switches->exchange = 0;


    GameState gameState(file);

    prevGameStates.push(gameState);

    vector<move_t> moves = getValidMoves(gameState);
    searchWin(gameState, moves);
    cout << "# Game is not winnable within " << win_switches->max_moves-1 << " moves.\n";
    cout << "Analyzed " << totalPositions << " positions\n";
	return 0;
}