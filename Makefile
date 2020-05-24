
all: check advance winnable game

# on pyrite:
# game: game.o GameState.o fileparser.o
# 	g++ game.o GameState.o fileparser.o /share/cs327/lib/libtermbox.a -o game

check: check.o fileparser.o
	gcc check.o fileparser.o -o check

advance: moves.o fileparser.o
	gcc moves.o fileparser.o -o advance

winnable: winnable.o GameState.o fileparser.o
	g++ winnable.o GameState.o fileparser.o -o winnable

game: game.o GameState.o fileparser.o
	g++ game.o GameState.o fileparser.o -o game -ltermbox

game.o: game.cpp termbox.h
	g++ -c game.cpp

fileparser.o: fileparser.c fileparser.h
	gcc -c fileparser.c

GameState.o: GameState.cpp GameState.h 
	g++ -c GameState.cpp

check.o: check.c fileparser.h
	gcc -c check.c 

moves.o: moves.c fileparser.h
	gcc -c moves.c

winnable.o: winnable.cc fileparser.h
	g++ -c winnable.cc

clean: 
	rm *.o check advance winnable game
