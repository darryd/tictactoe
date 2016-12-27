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

enum Sides other_side(enum Sides side) {

    if (side == X_side)
        return O_side;

    return X_side;
}

typedef struct _board {
    short x;
    short o;
} Board;

typedef void (*Player) (Board *board, enum Sides side);

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


int can_win_in(Board *board, enum Sides side, int count, int *ways) {

    enum Sides winner;
    Board new_board;
    int minimum_moves = INT_MAX;
    int num_moves;
    int num_moves_other_side;

    short moves;
    short position;

    winner = check_win(board);

    if (is_board_full(board))
        return INT_MAX;

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
    int num_moves_other_side;
    int position;
    Board new_board;
    int moves;
    int ways;
    int max_ways;

    if (is_board_full(board))
        return;


    moves = get_possible_moves(board);
    max_ways = 0;

    for(position = 0400; position > 0; position >>=1) {

        if ((moves & position) == position) {

            memcpy(&new_board, board, sizeof(Board));
            if (side == X_side)
                new_board.x |= position;
            else
                new_board.o |= position;
        

            if (check_win(&new_board) == side) {
                // We won!

                min_position = position;
                break;
            }

            // Would the other side win if they made this move?
            if (other_side(side) == X_side)
                new_board.x |= position;
            else
                new_board.o |= position;

            if (check_win(&new_board) == other_side(side)) {
                // We better move here so that the other side can't
                min_position = position;
                break;
            }

            num_moves = can_win_in(&new_board, side, 0, &ways);

            if (num_moves == min_num_moves && ways > max_ways) {
                min_position = position;
                max_ways = ways;
            }
            else if (num_moves < min_num_moves) {
                min_position = position;
                min_num_moves = num_moves;
                max_ways = ways;
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


int main() {

    Board board;
    short moves;
    int num_moves;
    enum Sides winner;
    enum Sides turn;

    init_board(&board);

    num_moves = can_win_in(&board, X_side, 0, NULL);

    print_board(&board);

    turn = X_side;

    while (!is_game_over(&board)) {


        printf("\n");

        printf ("---------------------------------------------------\n");

        printf("\n");

        printf ("%s turn:\n", turn == X_side ? "X" : "O");

        make_best_move(&board, turn);
        print_board(&board);
        printf("\n");

        turn = other_side(turn);
    }

    
    winner = check_win(&board);
    if (winner == X_side)
        printf("X won!\n");
    else if (winner == O_side)
        printf("O won!\n");
        
    else 
        printf("No winner\n");

        
    return;
}
