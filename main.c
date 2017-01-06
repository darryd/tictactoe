
/*

Command line tic tac toe

The board has nine positions:

     - - - 
     - - - 
     - - - 

These positions have three states: vacant, occupied by an x, or occupied by an o.

We will represent the board using two 9 bit numbers (one for x's and the other for o's)
to represent when a position on the board is occupied by either an x or an o. A 1 means 
that a position is occupied,while a 0 means that a position is vacant.

The first row will be represented by the first 3 bits. The second row by the next
three bits. And the third row, by the last three bits.

For example,

    X 0 -
    - X 0
    - - X

Would have two numbers:

x: 100 010 001
o: 010 001 000

To see if a position is occupied by either an x or an o, we can OR x and o. Then
we can AND it with the position. 

For example, 
Suppose we label the board like this:

1 2 3
4 5 6 
7 8 9

We can now say that the 5th position is the position in the very middle of the board.
So positon = 000 010 000

So (x OR o) AND position == position if position is occupied.

Let's try it given that x = 100 010 001 and o = 010 001 000.

    100 010 001 
 OR 010 001 000
    -----------
    110 011 001
AND 000 010 000
    -----------
    000 010 000 

Since we end up with 000 010 000 we know that the 5th position is occupied. Otherwise,
we would have ended up with 000 000 000.

Because each row is represented by three bits, it is convenient to represent each row 
with an octal digit.

So continuing our example:
x: 100 010 001 = 0421
o: 010 001 000 = 0220
 
To win at tic tac toe, a player (x or o) can win in one of eight ways:

   Horizonal wins: 
    111 000 000  = 0700
    000 111 000  =  070
    000 000 111  =   07

   Vertical wins:
    100 100 100 = 0444
    010 010 010 = 0222
    001 001 001 = 0111

   Diagonal wins:
   100 010 001 = 0421
   001 010 001 = 0124

 */

#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>

// http://stackoverflow.com/a/37539
#define NELEMS(x)  (sizeof(x) / sizeof((x)[0]))

#define NUM_OF_POSITIONS 9
#define MAX_LENGTH 1024

int wins[] = {0700, 070, 07, 0444, 0222, 0111, 0421, 0124};

enum Sides {X_side, O_side, No_side};

enum Sides other_side(enum Sides side) {

    if (side == X_side)
        return O_side;

    if (side == O_side)
        return  X_side;

    return No_side;
}

typedef short Position;

typedef struct _board {
    Position x;
    Position o;
} Board;


typedef Position (*Player) (const Board *board, enum Sides side);

typedef struct _game {

    Board board;
    enum Sides turn;

    Player x_player;
    Player o_player;

} Game;


void print_binary(Position pos) {

    int mask;

    for (mask = 0400; mask > 0; mask >>= 1) {
        if ((pos & mask) == mask)
            printf("1");
        else
            printf("0");
    }
    printf("\n");
}

void init_board(Board *board) {
    memset(board, 0, sizeof(Board));
}

enum Sides check_win(const Board *board) {
    int i;

    for (i = 0; i < NELEMS(wins); i++)
        if ((wins[i] & board->x) == wins[i])
            return X_side;

    for (i = 0; i < NELEMS(wins); i++)
        if ((wins[i] & board->o) == wins[i])
            return O_side;

    return No_side;
}

void print_board(const Board *board) {

    int mask;
    int count = 1;

    printf("   ");
    for (mask = 0400; mask > 0; mask >>= 1, count++) {

        if ((board->x & mask) == mask)
            printf("x ");
        else if ((board->o & mask) == mask)
            printf("o ");
        else
            printf("- ");

        if (count % 3 == 0) {
            count = 0;
            printf("\n   ");
        }
    }
}


int number_of_positions(Position pos) {

    Position mask;
    int count = 0;

    for (mask = 0400; mask > 0; mask >>= 1) {
        if ((pos & mask) == mask)
            count++;
    }

    return count;
}

int is_vacant(const Board *board, Position position) {

    return ((board->x | board->o) & position) != position;
}

void play_position(Board *board, Position position, enum Sides side) {

    if (!is_vacant(board, position)) {

        fprintf(stderr, "Board is not vacant at that position!\n");
        print_board(board);
        print_binary(position);

        exit(1);
    }

    if (side == X_side)
        board->x |= position;

    if (side == O_side)
        board->o |= position;
}

void print_winner(const Board *board) {

    enum Sides win;

    win = check_win(board);

    switch(win) {
        case X_side:
            printf ("X wins\n");
            break;

        case O_side:
            printf ("O wins\n");
            break;

        case No_side:
            printf("No winner\n");
    }
}

Position get_possible_moves(const Board *board) {

    Position moves = 0;
    Position position;

    for (position = 0400; position > 0; position >>= 1)
        if (is_vacant(board, position))
            moves |= position;

    return moves;
}

void print_possible_moves(Position moves, const Board *board, enum Sides side) {

    Board new_board;
    Position position;

    for (position = 0400; position > 0; position >>= 1) {

        if ((position & moves) == position) {

            memcpy(&new_board, board, sizeof(Board));
            if (side == X_side)
                new_board.x |= position;
            else
                new_board.o |= position;

            printf("Board:\n");
            print_board(&new_board);
            printf("\n");
        }
    }
}

int is_board_full(const Board *board) {
    return (board->x | board->o) == 0777;
}


int can_win_in(const Board *board, enum Sides side, int count, int *ways) {

    enum Sides winner;
    Board new_board;
    int minimum_moves = INT_MAX;
    int num_moves;

    Position moves;
    Position position;

    winner = check_win(board);

    if (winner == side)
        return count;

    if (winner == No_side) {

        moves = get_possible_moves(board);

        if (ways != NULL)
            *ways = 1;

        for (position = 0400; position > 0; position >>= 1) {
            if ((position & moves) == position) {
                memcpy(&new_board, board, sizeof(Board));

                if (side == X_side)
                    new_board.x |= position;
                else 
                    new_board.o |= position;

                num_moves = can_win_in(&new_board, side, count + 1, NULL);

                if (num_moves == minimum_moves && ways != NULL)
                    (*ways)++;

                if (num_moves < minimum_moves) {
                    minimum_moves = num_moves;
                    if (ways != NULL)
                        *ways = 1;
                }
            }
        }
        return minimum_moves;
    }

    return INT_MAX; // The other side won.
}


// From: http://stackoverflow.com/a/15827913
#define BUFFER 32
char *readString()
{
    char *str = malloc(sizeof(char) * BUFFER), *err;
    int pos;
    for(pos = 0; str != NULL && (str[pos] = getchar()) != '\n'; pos++)
    {
        if(pos % BUFFER == BUFFER - 1)
        {
            if((err = realloc(str, sizeof(char) * (BUFFER + pos + 1))) == NULL)
                free(str);
            str = err;
        }
    }
    if(str != NULL)
        str[pos] = '\0';
    return str;
}


Position get_move_from_user(const Board *board, enum Sides side) {

    char *input;
    char number;
    Position move;

    while (1) {
        do {
            printf("\n");
            printf("Type a number 1 to 9:\n\n");
            printf("\n   1 2 3\n   4 5 6\n   7 8 9\n\n");
            input = readString();
            printf("%s\n", input);

            number = atol(input);
            free(input);

        } while (number < 1 || number > 9);

        move = 01000 >> number;

        if (is_vacant(board, move))
            break;
        else
            printf("Invalid move.\n");
    }

    return move;
}

Position make_random_move(const Board *board, enum Sides side) {

    Position pos;
    Position moves;
    Position list_moves[9];
    int count = 0;

    moves = get_possible_moves(board);

    for (pos = 0400; pos > 0; pos >>= 1) {
        if ((moves & pos) == pos) {
            list_moves[count++] = pos;
        }
    }

    if (count == 0)
        return 0; // There were no possible moves.

    return list_moves[rand() % count];
}


Position make_best_move(const Board *board, enum Sides side) {

    int min_position;
    int min_num_moves = INT_MAX; 
    int num_moves;
    int position;
    Board new_board;
    int moves;
    int ways;
    int max_ways;

    if (is_board_full(board))
        return 0;

    moves = get_possible_moves(board);
    max_ways = 0;

    // Look for a win
    for (position = 0400; position > 0; position >>= 1) {
        if ((moves & position) != position)
            continue;

        memcpy(&new_board, board, sizeof(Board));
        play_position(&new_board, position, side);

        if (check_win(&new_board) == side)
            return position; // We won!
    }

    // Look for a lose, and prevent it
    // Would the other side win if they made this move?
    for (position = 0400; position > 0; position >>= 1) {
        if ((moves & position) != position)
            continue;

        memcpy(&new_board, board, sizeof(Board));
        play_position(&new_board, position, other_side(side));

        if (check_win(&new_board) == other_side(side))
            return position; // Let's move here so that the other side can't.
    }

    // Look for the best move

    for(position = 0400; position > 0; position >>=1) {

        if ((moves & position) != position) {
            continue;
        }

        memcpy(&new_board, board, sizeof(Board));
        play_position(&new_board, position, side);

        num_moves = can_win_in(&new_board, side, 0, &ways);

        if (num_moves == min_num_moves && ways > max_ways) {
            min_position = position;
            max_ways = ways;
        }

        else if (num_moves == min_num_moves && ways == max_ways) {
            // Add randomness to the decision making process here.
            if (rand() % 2 == 0) {
                min_position = position;
            }
        }
        else if (num_moves < min_num_moves) {
            min_position = position;
            min_num_moves = num_moves;
            max_ways = ways;
        }
    }

    return min_position;
}

int is_game_over(const Board *board) {

    if (is_board_full(board))
        return 1;

    if (check_win(board) == No_side)
        return 0;

    return 1;
}

void play_game(Game *game, enum Sides who_goes_first) {

    Position move;
    enum Sides turn = who_goes_first;

    while (!is_game_over(&game->board)) {

        printf("\33[2J\33[;H");
        printf("%s turn:\n", turn == X_side ? "X" : "O");
        printf("\n");

        print_board(&game->board);

        move = turn == X_side ? game->x_player(&game->board, turn) : game->o_player(&game->board, turn);
        // TODO validate move
        play_position(&game->board, move, turn);


        turn = other_side(turn);
    }


    printf("\33[2J\33[;H");
    printf("%s turn:\n", other_side(turn) == X_side ? "X" : "O");
    printf("\n");
    print_board(&game->board);
    printf("\n");
    print_winner(&game->board);
}

int main() {

    Game game;
    srand(time(NULL));

    game.x_player = get_move_from_user;
    game.o_player = make_best_move;

    init_board(&game.board);
    play_game(&game, X_side);

    return 0;
}
