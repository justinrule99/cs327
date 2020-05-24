//
// Created by justin on 4/22/20.
//
#include <iostream>
#include <string.h>
#include "termbox.h"
#include "fileparser.h"
#include "GameState.h"

// main executable for part 4 (game)

using namespace std;

struct switches {
    FILE* infile; // exchange format
    char* filename;
    int seed; // no game init if 0
    int turn;
    int limit;
};

switches* sw;
FILE* debug;

unsigned long RNG_seed;

// Lehmer random number generator as shown in assignment PDF
double Random() {
    const unsigned long MODULUS = 2147483647;
    const unsigned long MULTIPLIER = 48271;
    const unsigned long Q = MODULUS / MULTIPLIER;
    const unsigned long R = MODULUS % MULTIPLIER;
    unsigned long t1 = MULTIPLIER * (RNG_seed % Q);
    unsigned long t2 = R * (RNG_seed / Q);
    if (t1 > t2) {
        RNG_seed = t1 - t2;
    } else {
        RNG_seed = t1 + (MODULUS - t2);
    }
    return ((double) RNG_seed / MODULUS);
}


stack<card_t> reverseStack(stack<card_t> st) {
    stack<card_t> newStack;
    while (!st.empty()) {
        newStack.push(st.top());
        st.pop();
    }

    return newStack;
}

long chooseBetween(long a, long b) {
    return a + (long) ((b-a+1)*Random());
}

card_t* generateDeck() {
    // standard 52 card deck, indexed 0-51 by indexof(suit)*indexof(rank)
    card_t* cards = new card_t[52];
    for (int i = 0; i < 52; i++) {
        cards[i].separator = 0;
        cards[i].s = suits[i/13];
        cards[i].r = ranks[i%13];
    }

    // randomize deck here
    for (int i = 0; i < 51; i++) {
        int j = chooseBetween(i,51);
        if (i != j) {
            swap(cards[i], cards[j]);
        }
    }

    return cards;
}



void write_text(int x, int y, char const *text, int fcolor=TB_WHITE, int bcolor=TB_BLACK) {
    tb_cell onechar;
    onechar.fg = fcolor;
    onechar.bg = bcolor;

    for (int i = 0; i < strlen(text); i++) {
        onechar.ch = text[i];
        tb_put_cell(x+i, y, &onechar);
    }
}

// cards 5 cells wide, 4 cells long
void draw_card(int x, int y, card_t card) {
    if (card.separator) return;

    // 5x4 with white background, text is rs on top row
    tb_cell cells[5][4];

    unsigned short suit;
    switch(card.s) {
        case 'c': suit = 0x02660;
            break;
        case 'h': suit = 0x02665;
            break;
        case 'd': suit = 0x02666;
            break;
        default: suit = 0x02663;
    }

    int color = card.s == 'd' || card.s == 'h' ? TB_RED : TB_BLACK;

    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 4; j++) {
            cells[i][j].bg = TB_WHITE;
            if (!j) {
                cells[i][j].fg = i >= 2 ? TB_WHITE : color;
            } else {
                cells[i][j].fg = TB_WHITE;
            }

            if (!i && !j) {
                cells[i][j].ch = card.r;
            } else if (i == 1 && !j) {
                cells[i][j].ch = suit;
            } else {
                cells[i][j].ch = 'r';
            }
            tb_put_cell(x+i, y+j, &cells[i][j]);
        }
    }
}

// draw everything, will get called every time the game changes
void draw_base_interface(GameState* state) {
    tb_cell border;
    border.bg = TB_CYAN;
    border.fg = TB_CYAN;
    border.ch = 'd';

    // draw border for 100x30
    for (int i = 0; i <= 100; i++) {
        tb_put_cell(i, 30, &border);
        if (i < 30) {
            tb_put_cell(100, i, &border);
        }
    }

    // horizontal
    for (int i = 0; i < 35; i++) {
        tb_put_cell(i, 18, &border);
    }

    // vertical
    for (int i = 18; i < 30; i++) {
        tb_put_cell(35, i, &border);
    }


    write_text(1, 19, "Solitaire: ");
    if (sw->infile) {
        write_text(13, 19, sw->filename);
    } else {
        write_text(13,19, to_string(sw->seed).c_str());
    }
    write_text(2, 21, "q", TB_BLACK, TB_YELLOW);
    write_text(3, 21, ": quit");

    write_text(2, 22, "r", TB_BLACK, TB_YELLOW);
    write_text(3, 22, ": restart");

    write_text(2, 23, "u", TB_BLACK, TB_YELLOW);
    write_text(3, 23, ": undo last move");

    write_text(2, 25, ".", TB_BLACK, TB_YELLOW);
    write_text(3, 25, ": next card");

    write_text(2, 26, "r", TB_BLACK, TB_YELLOW);
    write_text(3, 26, ": reset stock");

    write_text(2, 28, "To move: press key for source");
    write_text(2, 29, "(w or 1-7), then dest (f or 1-7)");


    // move: press source then destination

    write_text(65, 20, "Foundations");
    // foundations: x: 60 +=5, y=32
    for (int i = 0; i < 4; i++) {
        if (state->foundations[i] == '_') {
            // don't draw
            write_text(57+7*i, 22, "empty");
        } else {
            card_t card;
            card.r = state->foundations[i];
            card.separator = 0;
            switch(i) {
                case 0: card.s = 'c';
                    break;
                case 1: card.s = 'd';
                    break;
                case 2: card.s = 'h';
                    break;
                default: card.s = 's';
            }
            draw_card(57+7*i, 22, card);
        }
    }

    for (int i = 0; i < 7; i++) {
        int num = 0;
        write_text(50+7*i, 1, to_string(i+1).c_str(), TB_WHITE, TB_MAGENTA);
        stack<card_t> coveredCopy = state->cols_covered[i];

        while (!coveredCopy.empty()) {
            write_text(48+7*i, 3+num, "#####", TB_WHITE, TB_BLUE);
            coveredCopy.pop();
            num++;
        }
        stack<card_t> reverse = reverseStack(state->cols[i]);
        while (!reverse.empty()) {
            card_t c = reverse.top();
            reverse.pop();
            draw_card(48+7*i, 3+num, c);
            num++;
        }
    }

    // write waste and stock piles
    write_text(7, 4, "STOCK");
    write_text(20, 4, "WASTE");

    // facedown card under stock
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 4; j++) {
            write_text(7+i, 6+j, "#", TB_WHITE, TB_BLUE);
        }
    }
    write_text(9, 11, to_string(state->stock.size()).c_str());

    // assume turn 1
    if (!state->waste.empty()) {
        draw_card(20, 6, state->waste.top());
    }
    // write: x resets remaining
    char* limText = new char;
    if (sw->limit) {
        sprintf(limText, "%d resets", sw->limit);
        write_text(19, 11, limText);
    } else if (state->limitNum) {
        if (state->limitNum != 1000) {
            sprintf(limText, "%d resets", state->limitNum);
        }
        write_text(19, 11, limText);
    }
}



void show_termbox(GameState *startState) {
    // designed for terminal 100x30

    tb_init();

    draw_base_interface(startState);

    while (true) {
        tb_present();

        tb_event event;
        tb_poll_event(&event);

        if (event.ch == 'q') break;
        if (event.ch == 'u') {

        } else if (event.ch == '.') {
            // make move_t and process it on startState, then redraw
            move_t move;
            move.turn_over = 1;
            move.src = 0;
            move.dest = 0;

            (*startState).processThisMove(move);

            tb_clear();
            draw_base_interface(startState);
        } else if (event.ch == 'r') {
            // move: reset
            move_t move;
            move.waste_reset = 1;
            move.turn_over = 0;
            (*startState).processThisMove(move);
            tb_clear();
            draw_base_interface(startState);

        } else if (event.ch == 'w' || event.ch-'0' <= 7 ){

            tb_event destEvent;
            tb_poll_event(&destEvent);

            // don't process an invalid move
            if (destEvent.ch != 'f' && destEvent.ch-'0' > 7) {
                tb_clear();
                draw_base_interface(startState);
                continue;
            }

            move_t longMove;
            longMove.src = event.ch;
            longMove.dest = destEvent.ch;
            longMove.turn_over = 0;
            longMove.waste_reset = 0;

            (*startState).processThisMove(longMove);
            tb_clear();
            draw_base_interface(startState);
        } else {
            tb_clear();
            draw_base_interface(startState);
        }
    }

    tb_shutdown();
    cout << "Game Ended.\n";
}

GameState* generateRandomGame(int seed) {
    card_t* deck = generateDeck();
    GameState* game = new GameState;
    game->turnNum = sw->turn;
    if (!sw->limit) {
        game->limitNum = 1000;
    } else {
        game->limitNum = sw->limit;
    }

    for (int i = 0; i < 4; i++) {
        game->foundations[i] = '_';
    }

    int card_idx = 0;
    for (int i = 0; i < 7; i++) {
        for (int j = 0; j < i; j++) {
            game->cols_covered[i].push(deck[card_idx++]);
        }
        game->cols[i].push(deck[card_idx++]);
    }

    while (card_idx < 52) {
        game->stock.push(deck[card_idx++]);
    }

    return game;
}

int main(int argc, char** argv) {
    debug = fopen("debug.txt", "w");

    // get command line args
    sw = new switches;
    for (int i = 0; i < argc; i++) {
        if (strcmp(argv[i], "-f") == 0) {
            sw->infile = fopen(argv[++i], "r");
            sw->filename = argv[i];
        } else if (strcmp(argv[i], "-s") == 0) {
            sw->seed = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-l") == 0) {
            sw->limit = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-1") == 0) {
            sw->turn = 1;
        } else if (strcmp(argv[i], "-3") == 0) {
            sw->turn = 3;
        }
    }

    if (!sw->turn) {
        sw->turn = 1;
    }

    GameState* start;

    if (sw->infile) {
        // make game based on input file
        start = new GameState(sw->infile);
    } else {
        // make game based on seed, then pass into show_termbox
        RNG_seed = sw->seed;
        start = generateRandomGame(sw->seed);
    }
    show_termbox(start);

    return 0;
}

