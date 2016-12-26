#include <stdio.h>
#include <string.h>
#include <limits.h>

// http://stackoverflow.com/a/37539
#define NELEMS(x)  (sizeof(x) / sizeof((x)[0]))

#define NUM_OF_POSITIONS 9

// | 1 1 1 | 1 1 1 | 1 1 1 |

/*

   1  2  3

   1  _  _  _  

   2  _  _  _

   3  _  _  _


   Possible wins:

   Horizonal wins: 
   | 1 1 1 | 0 0 0 | 0 0 0 | = 0700 (octal)
   | 0 0 0 | 1 1 1 | 0 0 0 | =  070
   | 0 0 0 | 0 0 0 | 1 1 1 | =   07

   Vertical wins:
   | 1 0 0 | 1 0 0 | 1 0 0 | = 0444
   | 0 1 0 | 0 1 0 | 0 1 0 | = 0222
   | 0 0 1 | 0 0 1 | 0 0 1 | = 0111

   Diagonal wins:
   | 1 0 0 | 0 1 0 | 0 0 1 | = 0421
   | 0 0 1 | 0 1 0 | 0 0 1 | = 0124

 */

int wins[] = {0700, 070, 07, 0444, 0222, 0111, 0421, 0124};

enum Sides {X_side, O_side, No_side};

typedef struct _board {
    short x;
    short o;
} Board;

typedef struct _player {
    void (*f) (Board *board, enum Sides side);
} Player;

typedef struct _game {

    Board board;
    enum Sides turn;

    Player x_player;
    Player o_player;

} Game;



void init_board(Board *board) {
    memset(board, 0, sizeof(Board));
}

enum Sides check_win(Board *board) {
    int i;

    for (i = 0; i < NELEMS(wins); i++)
        if ((wins[i] & board->x) == wins[i])
            return X_side;

    for (i = 0; i < NELEMS(wins); i++)
        if ((wins[i] & board->o) == wins[i])
            return O_side;

    return No_side;
}

void print_board(Board *board) {

    int mask;
    int count = 1;

    for (mask = 0400; mask > 0; mask >>= 1, count++) {

        if ((board->x & mask) == mask)
            printf("x ");
        else if ((board->o & mask) == mask)
            printf("o ");
        else
            printf("- ");

        if (count % 3 == 0) {
            count = 0;
            printf("\n");
        }
    }
}

int is_vacant(Board *board, short position) {

    return ((board->x | board->o) & position) != position;
}

void print_winner(Board *board) {

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

short get_possible_moves(Board *board) {

    short moves = 0;
    short position;

    for (position = 0400; position > 0; position >>= 1)
        if (is_vacant(board, position))
            moves += position;

    return moves;
}

void print_possible_moves(short moves, Board *board, enum Sides side) {

    Board new_board;
    short position;

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

int is_board_full(Board *board) {
    return (board->x | board->o) == 0777;
}


int can_win_in(Board *board, enum Sides side, int count) {

    enum Sides winner;
    Board new_board;
    int minimum_moves = INT_MAX;
    int num_moves;

    short moves;
    short position;

    winner = check_win(board);

    if (is_board_full(board))
        return INT_MAX;

    if (winner == side)
        return count;

    if (winner == No_side) {

        moves = get_possible_moves(board);
        for (position = 0400; position > 0; position >>= 1) {
            if ((position & moves) == position) {
                memcpy(&new_board, board, sizeof(Board));

                if (side == X_side)
                    new_board.x |= position;
                else 
                    new_board.o |= position;

                num_moves = can_win_in(&new_board, side, count + 1);

                if (num_moves < minimum_moves)
                    minimum_moves = num_moves;
            }
        }

        return minimum_moves;
    }

    return INT_MAX; // The other side won.
}

void make_best_move(Board *board, enum Sides side) {

    int min_position;
    int min_num_moves = INT_MAX; 
    int num_moves;
    int position;
    Board new_board;
    int moves;

    if (is_board_full(board))
        return;


    moves = get_possible_moves(board);

    for(position = 0400; position > 0; position >>=1) {

        if ((moves & position) == position) {

            memcpy(&new_board, board, sizeof(Board));
            if (side == X_side)
                new_board.x |= position;
            else
                new_board.o |= position;
        
            num_moves = can_win_in(&new_board, side, 0);
            if (num_moves <= min_num_moves) {
                min_position = position;
                min_num_moves = num_moves;
            }
        }
    }

    if (side == X_side)
        board->x |= min_position;
    else
        board->o |= min_position;
}

int is_game_over(Board *board) {

    if (is_board_full(board))
        return 1;

    if (check_win(board) == No_side)
        return 0;

    return 1;
}

enum Sides other_side(enum Sides side) {

    if (side == X_side)
        return O_side;

    return X_side;
}

int main() {

    Board board;
    short moves;
    int num_moves;
    enum Sides winner;
    enum Sides turn;

    init_board(&board);


    num_moves = can_win_in(&board, X_side, 0);

    print_board(&board);

    turn = X_side;

    while (!is_game_over(&board)) {

        printf ("%s turn:\n", turn == X_side ? "X" : "O");

        make_best_move(&board, turn);
        print_board(&board);
        printf("\n");

        turn = other_side(turn);
    }

    
    winner = check_win(&board);
    if (winner == X_side)
        printf("X won!\n");
    if (winner == O_side)
        printf("O won!\n");

    return 0;
}
