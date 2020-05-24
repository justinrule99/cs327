//
// Created by justin on 4/3/20.
//

// next step: convert process_move to cpp, return a new GameState object

#include "GameState.h"

GameState::GameState(FILE* filename){
    // empty gameState, run parse_file
    file = filename;
    readFile();
}

GameState::GameState() {
    // need to do readFile later
}

GameState::GameState(const GameState& oldState) {
    // basic deep copy constructor
//    fprintf(fopen("debug.txt", "w"), "HERE IN AIDAS\n");
    cout << "HERe\n";
}


void GameState::readFile() {
    // can use file, read_whitespace, read_int, read_card from fileparser.c
    // assign variables to this, similar to part 1
    // what to do for filename? given from winnable.cc
    // should zero out everything in object?
    num_stock = 0;
    num_waste = 0;
    covered_cards = 0;


    int section = 0;
    int isValid = 1;

    while (!feof(file)) {
        // skips whitespace
        read_whitespace(file);

        char c = fgetc(file);
        if (c == ':') {
            section++;
            if (section == 1) {
                parseRules();
            } else if (section == 2) {
                parseFoundations();
            } else if (section == 3) {
                parseTableau();
            } else if (section == 4) {
                parseStock();
            } else if (section == 5) {
                parseMoves();
            }
        }
    }
    fclose(file);
}



void GameState::parseRules() {
    char turn[4];
    char limit[9];
    while (1) {
        read_whitespace(file);
        fgets(turn, 5, file);
        if (!(strcmp(turn, "turn"))) {
            // get next one, then number
            fgetc(file);
            int file_turn = read_int(file);
            turnNum = file_turn;

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
                            game_version[i] = limit[i];
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
                            game_version[i] = limit[i];
                        }
                        fgetc(file);
                        int lim = read_int(file);
                        limitNum = lim;
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

void GameState::parseFoundations() {
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
                foundations[suit_num-1] = r;
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

void GameState::parseTableau() {
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
            cols_covered[column].push(cur_card);
//            push(&state->s_cols_covered[column], cur_card);
            covered_cards++;
        } else {
            cols[column].push(cur_card);
//            push(&state->s_cols[column], cur_card);
        }
    }
}

void GameState::parseStock() {
    card_t cur_card;
    int i = 0;
    int in_waste = 1;

//    nodeptr local_stock;
    stack<card_t> localStock;

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
            // separator card, don't push to anything
            in_waste = 0;
        }
        // push to waste

        if (in_waste) {
            waste.push(cur_card);
            num_waste++;
        } else {
            // need to reverse this stack
            localStock.push(cur_card);
            // also push to stock
            if (!cur_card.separator) num_stock++;
        }
    }

    // this reverses the stock section from order read in, for ease of use later
//    card_t* arr_stock = malloc(num_stock * sizeof(card_t));
    card_t* arr_stock = new card_t[num_stock];

    for (int i = 0; i < num_stock; i++){
        arr_stock[i] = localStock.top();
        localStock.pop();
    }

    for (int i = 0; i < num_stock; i++){
        stock.push(arr_stock[i]);
    }
}

void GameState::parseMoves() {
    return;
}

void GameState::addToFoudnation(card_t to_add, move_t move) {

    for (int i = 0; i < 4; i++) {
        if (to_add.s == suits[i]) {
            if (to_add.r == 'A' && foundations[i] == '_') {
                foundations[i] = 'A';
                // pop from stack of card source (from move)
                // move.src is either a number (1-7) or w
                if (move.src == 'w') {
                    waste.pop();
                } else {
                    cols[move.src-'0'-1].pop();
                }
                move_idx++;
                return;
            } else if (is_next_rank(to_add.r, foundations[i])) {
                // increment rank on foundation
                foundations[i] = ranks[(strchr(ranks,foundations[i])-ranks)+1];
                if (move.src == 'w') {
                    waste.pop();
                } else {
                    cols[move.src-'0'-1].pop();
                }
                move_idx++;
                return;
            }
        }
    }

    cout << "error adding to foundations!\n";
    exit(0);
}

card_t GameState::getTopWaste() {
    return waste.top();
}

// ----------------------------------
// ------- END MEMBER FUNCTIONS -----
// ----------------------------------


 GameState GameState::processMove(move_t move, GameState currentState) {
    // turn over limit R
    if (move.turn_over) {
        if (currentState.turnNum == 1) {
            if (!currentState.stock.empty()) {
                card_t turn = currentState.stock.top();
                currentState.stock.pop();
                currentState.waste.push(turn);
                currentState.move_idx++;
                return currentState;
            } else {
                fprintf(stderr, "Move %d is illegal (.)\n",currentState.move_idx+1);
                exit(0);
            }

        } else if (currentState.turnNum == 3) {
            for (int i = 0; i < 3; i++){
                if (!currentState.stock.empty()) {
                    card_t turn = currentState.stock.top();
                    currentState.stock.pop();
                    currentState.waste.push(turn);
                } else {
                    fprintf(stderr, "Move %d is illegal (.)\n",currentState.move_idx+1);
                    exit(0);
                }
            }
            currentState.move_idx++;
            return currentState;
        }
    }

    if (move.waste_reset) {
        if (!(strcmp(currentState.game_version, "unlimited") == 0 || currentState.limitNum > 0)) {
            fprintf(stderr, "Move %d is illegal (r)\n",currentState.move_idx+1);
            exit(0);
        }

        if (!currentState.stock.empty()) {
            fprintf(stderr, "Move %d is illegal (r)\n",currentState.move_idx+1);
            exit(0);
        }

        while (!currentState.waste.empty()) {
            card_t card = currentState.waste.top();
            currentState.waste.pop();
            currentState.stock.push(card);
        }

        if (currentState.limitNum) {
            currentState.limitNum--;
        }
        currentState.move_idx++;
        return currentState;
    }

    // waste -> foundation
    if (move.src == 'w' && move.dest == 'f') {
        card_t waste_card = currentState.waste.top();
        currentState.addToFoudnation(waste_card, move);
        return currentState;
    }

    // tableau->foundation
    // returning my_file should not change it
    // here, s_cols[i] has been changed from other reference (why???)
    if (move.dest == 'f') {
        int src = move.src-'0'-1;
        card_t to_add = currentState.cols[src].top();
        currentState.cols[src].pop();

        if (currentState.cols[src].empty() && !currentState.cols_covered[src].empty()) {
            currentState.cols[src].push(currentState.cols_covered[src].top());
            currentState.cols_covered[src].pop();
        }

        currentState.cols[src].push(to_add);

        currentState.addToFoudnation(currentState.cols[src].top(), move);
        return currentState;
    }

    // waste->tableau
    if (move.src == 'w') {
        // guarantees waste -> tableau
        int dest = move.dest-'0'-1;
        card_t waste_card = currentState.waste.top();
        currentState.waste.pop();

        if (currentState.cols[dest].empty()) {
            if (waste_card.r == 'K') {
                currentState.cols[dest].push(waste_card);
                currentState.move_idx++;
                return currentState;
            } else {
                fprintf(stderr, "error moving waste->tableau\n");
                exit(0);
            }
        }

        card_t tab_card = currentState.cols[dest].top();

        if (is_next_rank(tab_card.r, waste_card.r) && is_alt_suit(tab_card, waste_card)) {
            // push onto tableau
            currentState.cols[dest].push(waste_card);
            currentState.move_idx++;
            return currentState;

        } else {
            fprintf(stderr, "erro moving waste->tab %c\n", move.dest);
            exit(0);
        }
    }

    // tableau->tablueau
    int src = move.src-'0'-1;
    int dest = move.dest-'0'-1;
    stack<card_t> temp_stack;

    if (currentState.cols[dest].empty()) {
        int i;
        for (i = 1; i < 13; i++){
            if (currentState.cols[src].empty()){
                fprintf(stderr, "error\n");
                exit(0);
            }
            card_t card_in_src = currentState.cols[src].top();
            currentState.cols[src].pop();

            temp_stack.push(card_in_src);
            if (card_in_src.r == 'K') {
                break;
            }
        }

        for (int j = 0; j < i; j++){
            card_t popped = temp_stack.top();
            temp_stack.pop();
            currentState.cols[dest].push(popped);
        }
        // uncovers the next one
        if (currentState.cols[src].empty()) {
            if (!currentState.cols_covered[src].empty()) {
                currentState.cols[src].push(currentState.cols_covered[src].top());
                currentState.cols_covered[src].pop();
            }
        }

        currentState.move_idx++;
        return currentState;
    }

    card_t card_in_dest = currentState.cols[dest].top();

    int i;
    for (i = 1; i < 13; i++){
        if (currentState.cols[src].empty()){
            fprintf(stderr, "error\n");
            exit(0);
        }
        card_t card_in_src = currentState.cols[src].top();
        currentState.cols[src].pop();

        temp_stack.push(card_in_src);
        if (is_next_rank(card_in_dest.r, card_in_src.r) && is_alt_suit(card_in_src, card_in_dest)) {
            break;
        }
    }

    for (int j = 0; j < i; j++){
        card_t popped = temp_stack.top();
        temp_stack.pop();
        currentState.cols[dest].push(popped);
    }

    // uncovers a card if necessary
    // also check if theres nothing to pop from covered
    if (currentState.cols[src].empty()) {
        if (!currentState.cols_covered[src].empty()) {
            currentState.cols[src].push(currentState.cols_covered[src].top());
            currentState.cols_covered[src].pop();
        }
    }
    currentState.move_idx++;

    return currentState;

}

// modifies current GameState object instead of creating a new one
void GameState::processThisMove(move_t move) {
    if (move.turn_over) {
        if (turnNum == 1) {
            if (!stock.empty()) {
                card_t turn = stock.top();
                stock.pop();
                waste.push(turn);
                move_idx++;
                return;
            } else {
                fprintf(stderr, "Move %d is illegal (.)\n",move_idx+1);
                exit(0);
            }

        } else if (turnNum == 3) {
            for (int i = 0; i < 3; i++){
                if (!stock.empty()) {
                    card_t turn = stock.top();
                    stock.pop();
                    waste.push(turn);
                } else {
                    fprintf(stderr, "Move %d is illegal (.)\n",move_idx+1);
                    exit(0);
                }
            }
            move_idx++;
            return;
        }
    }

    if (move.waste_reset) {
        if (!(strcmp(game_version, "unlimited") == 0 || limitNum > 0)) {
            fprintf(stderr, "Move %d is illegal (r)\n",move_idx+1);
            exit(0);
        }

        if (!stock.empty()) {
            fprintf(stderr, "Move %d is illegal (r)\n",move_idx+1);
            exit(0);
        }

        while (!waste.empty()) {
            card_t card = waste.top();
            waste.pop();
            stock.push(card);
        }

        if (limitNum) {
            limitNum--;
        }
        move_idx++;
        return;
    }

    // waste -> foundation
    if (move.src == 'w' && move.dest == 'f') {
        card_t waste_card = waste.top();
        addToFoudnation(waste_card, move);
        return;
    }

    // tableau->foundation
    // returning my_file should not change it
    // here, s_cols[i] has been changed from other reference (why???)
    if (move.dest == 'f') {
        int src = move.src-'0'-1;
        card_t to_add = cols[src].top();
        cols[src].pop();

        if (cols[src].empty() && !cols_covered[src].empty()) {
            cols[src].push(cols_covered[src].top());
            cols_covered[src].pop();
        }

        cols[src].push(to_add);

        addToFoudnation(cols[src].top(), move);
        return;
    }

    // waste->tableau
    if (move.src == 'w') {
        // guarantees waste -> tableau
        int dest = move.dest-'0'-1;
        card_t waste_card = waste.top();
        waste.pop();

        if (cols[dest].empty()) {
            if (waste_card.r == 'K') {
                cols[dest].push(waste_card);
                move_idx++;
                return;
            } else {
                fprintf(stderr, "error moving waste->tableau\n");
                exit(0);
            }
        }

        card_t tab_card = cols[dest].top();

        if (is_next_rank(tab_card.r, waste_card.r) && is_alt_suit(tab_card, waste_card)) {
            // push onto tableau
            cols[dest].push(waste_card);
            move_idx++;
            return;

        } else {
            fprintf(stderr, "erro moving waste->tab %c\n", move.dest);
            exit(0);
        }
    }

    // tableau->tablueau
    int src = move.src-'0'-1;
    int dest = move.dest-'0'-1;
    stack<card_t> temp_stack;

    if (cols[dest].empty()) {
        int i;
        for (i = 1; i < 13; i++){
            if (cols[src].empty()){
                fprintf(stderr, "error\n");
                exit(0);
            }
            card_t card_in_src = cols[src].top();
            cols[src].pop();

            temp_stack.push(card_in_src);
            if (card_in_src.r == 'K') {
                break;
            }
        }

        for (int j = 0; j < i; j++){
            card_t popped = temp_stack.top();
            temp_stack.pop();
            cols[dest].push(popped);
        }
        // uncovers the next one
        if (cols[src].empty()) {
            if (!cols_covered[src].empty()) {
                cols[src].push(cols_covered[src].top());
                cols_covered[src].pop();
            }
        }

        move_idx++;
        return;
    }

    card_t card_in_dest = cols[dest].top();

    int i;
    for (i = 1; i < 13; i++){
        if (cols[src].empty()){
            fprintf(stderr, "error\n");
            exit(0);
        }
        card_t card_in_src = cols[src].top();
        cols[src].pop();

        temp_stack.push(card_in_src);
        if (is_next_rank(card_in_dest.r, card_in_src.r) && is_alt_suit(card_in_src, card_in_dest)) {
            break;
        }
    }

    for (int j = 0; j < i; j++){
        card_t popped = temp_stack.top();
        temp_stack.pop();
        cols[dest].push(popped);
    }

    // uncovers a card if necessary
    // also check if theres nothing to pop from covered
    if (cols[src].empty()) {
        if (!cols_covered[src].empty()) {
            cols[src].push(cols_covered[src].top());
            cols_covered[src].pop();
        }
    }
    move_idx++;

    return;
}