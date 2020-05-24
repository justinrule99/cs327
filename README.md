# jrule-coms327

jrule@iastate.edu

This repository was created by Justin Rule, for COM S 327 Spring 2020

For this project, we are implementing a Solitaire game in C and C++


# Usage

Part 1: Type `make` into a terminal, then `./check < inputfile.txt` or `./check inputfile.txt` 

Part 2: Type `make` into a terminal, then `./advance < inputfile.txt (flags)` or `./advance inputfile.txt (flags)`

Flags: 
	* `-m N` plays only N moves from the input file
	* `-o outfile` writes the processed game configuration to `outfile` rather than `stdout`
	* `-x` will output the game in 'exchange' format (otherwise, output in human readable form)

Part 3: Type `make` into a terimal, then `./winnable inputfile.txt (flags)` or `./winnable (flags) < inputfile.txt`

Flags: 
    * `-m N` checks if a game is winnable in N moves
    * `-c` use a cache (not implemented)
    * `-f` force moves to foundations (not implemented)
    
Part 4: Type `make` into a terminal, then `./game (flags)`
    The following switches are implemented: `-f filename.txt` runs the game from a previous configuration in file, 
    `-s (seed)` generates a random game based a an int `(seed)`. `-l L` sets a limit `L` on stock resets for a randomly generated game.
    Note: to compile on pyrite with termbox, verify the Makefile runs 
    
    game: game.o GameState.o fileparser.o 
        g++ game.o GameState.o fileparser.o /share/cs327/lib/libtermbox.a -o game
        
   

# Part 1

(Part 1 due 2/11/2020)

This portion parses an input file that describes a game of Solitaire, given either as a command line argument or from stdin.

Below is a sample input file: 

```
RULES: # This section must be first
# The following must appear in this order
	turn 1 # flip over one card at a time
	unlimited # Easiest possible version of Klondike
FOUNDATIONS: # Alpha order: c,d,h,s
	_c # nothing on clubs foundation
	_d # nor on the diamonds foundation
	2h # hearts foundation has 2h and Ah
	As # spades foundation has As
TABLEAU:
	8d 5c 7h Jd | Qs Jh Tc # Column 7
	Ad 3h 4d 5s | 7d 6s 5d 4s # Column 6
	7s Kd | 3s # Column 5
	6h Qc 4h | 7c # Column 4
	8s 2s | 4c 3d 2c # Column 3
	| # Column 2 (empty)
	| Ks Qh Jc Td # Column 1 (nothing covered)
STOCK:
	3c 8c | Th Kh 8h # These don’t have to appear all on one line
	Qd 9s 6c Kc Ac Ts
	Js 2d # Lots of spaces, just because
	9h 6d 9c 5h 9d
MOVES:
```

The parser reads all these sections (except MOVES) into appropriate structures and set up a game of Solitaire

# Part 2

(Part 2 due 3/10/20)

This part parses an input file similar to that of part 1, now including the MOVES section. Then, the game is advanced based on the rules of Solitaire.

Sample `MOVES` section: 

```
MOVES:
  4->1  5->2  5->3  2->5  2->1  5->2  5->f  5->3  .     w->7
  6->7  .     .     w->f  .     .     .     .     .     .
  .     w->4  1->4  w->1  7->1  7->f  3->1  .     w->6  .
  w->f  .     .     .     .     w->f  .     .     .     w->f
  .     w->3  .     .     .     .     r     .     w->f  .  
  w->5  .     .     w->f  4->f  4->f  .     w->5  7->5  7->5
  .     .     w->3  .     w->f  .     .     w->5  7->5  .
  w->3  .     w->3  7->3  7->f  .     w->f  6->f  .     .
  w->f  r     .     w->f  6->f  6->5  6->3  6->1  4->6
```

# Part 3

(Part 3 due 4/11/20)

This part determines whether a game is winnable or not given a current game state. The program will search through game configurations, then print a winning move sequence if found.

Sample usage: `./winnable -m 5 testwin1.txt`  
Output: 
```
    # Game is winnable within 5 moves.
    Analyzed 382 positions.
```

Not winnable: `./winnable -m 20 testwin3.txt`  
Output: 
```
    # Game is not winnable within 21 moves.
    Analyzed 8 positions
```

#Part 4

(Part 4 due 4/28/20)

This part allows a user to play Solitaire in a GUI through a unix terminal. Running the executable requires
`termbox`, which can be found [here](https://github.com/nsf/termbox). Running with `-s S` generates a random
game, while `-f filename` starts a game based on the given text file (formatted the same as input files for part 1). 
