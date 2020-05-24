//
// Created by justin on 4/3/20.
extern "C" {
#include "fileparser.h"
#include <stdlib.h>
#include <stdio.h>
}
#include <bits/stdc++.h>
#include <iostream>

using namespace std;

#ifndef JRULE_COMS327_GAMESTATE_H
#define JRULE_COMS327_GAMESTATE_H

// redesign entire file_state struct as a class
// includes all of fileparser.h via winnable.cc

class GameState {
public:
    FILE* file;
    int turnNum;
    char game_version[50];
    int limitNum;
    char foundations[4];

    // array of std::Stack
    stack<card_t> cols[7];
    stack<card_t> cols_covered[7];

    int covered_cards;
    stack<card_t> waste;
    stack<card_t> stock;

    int move_idx;
    int num_waste;
    int num_stock;

    GameState();
    GameState(FILE* file);
    // SHALLOW copies stacks (nothing i can do -_-)
    GameState(state_t* my_file) {
        // limit, foundations, s_cols_covered, s_cols, waste, stock
    }
    GameState(const GameState& oldState);
        // no args, just read from stdin
    card_t getTopWaste();

    void parseRules();
    void parseFoundations();
    void parseTableau();
    void parseStock();
    void parseMoves();
    void readFile();

    void addToFoudnation(card_t to_add, move_t move);
    GameState processMove(move_t move, GameState currentState);
    void processThisMove(move_t move);


};


#endif //JRULE_COMS327_GAMESTATE_H
