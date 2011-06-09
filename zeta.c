/*
    Zeta CL, OpenCL Chess Engine
    Author: Srdja Matovic <srdja@matovic.de>
    Created at: 20-Jan-2011
    Updated at:
    Description: A Chess Engine written in OpenCL, a language suited for GPUs.

    Copyright (C) 2011 Srdja Matovic
    This program is distributed under the GNU General Public License.
    See file COPYING or http://www.gnu.org/licenses/
*/

#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "types.h"


const char filename[]  = "zeta.cl";
char *source;
size_t sourceSize;

long MOVECOUNT = 0;
long NODECOUNT = 0;
int PLY = 0;

// config
int max_depth  = 4;
int max_mem_mb = 64;
int max_cores  = 1;


clock_t start, end;
double elapsed;


const Score  INF = 32000;

Piece BOARD[129];

Move bestmove = 0;

Move Lastmove = 0;


// functions
Move move_parser(char *usermove, Piece *board, int som);
void setboard(char *fen);
void print_movealg(Move move);
void print_board(Piece *board);
void print_stats();


// cl functions
extern int load_file_to_string(const char *filename, char **result);
extern int initializeCL(Piece *board);
extern int  runCLKernels(int som, Move lastmove);
extern int   cleanupCL(void);


/* ############################# */
/* ###        inits          ### */
/* ############################# */


/* ############################# */
/* ###     move tools        ### */
/* ############################# */

inline Move makemove(Square from, Square to, Piece pcpt, Piece promo) {
    return (from | (Move)to<<8 |  (Move)pcpt <<16 |  (Move)promo <<20);  
}
inline Square getfrom(Move move) {
    return (move & 0xFF);
}
inline Square getto(Move move) {
    return ((move>>8) & 0xFF);
}
inline Piece getpcpt(Move move) {
    return ((move>>16) & 0xF);
}
inline Piece getpromo(Move move) {
    return ((move>>20) & 0xF);
}

static inline Square make_square(int f, int r) {
  return ( f |  (r << 3));
}

static inline int square_file(Square s) {
  return (s & 7);
}

static inline int square_rank(Square s) {
  return (s >> 3);
}


/* ############################# */
/* ###     domove undomove   ### */
/* ############################# */

void domove(Piece *board, Move move, int som) {

    // set to, unset capture
    board[getto(move)] = board[getfrom(move)];        
    // unset from
    board[getfrom(move)] = PEMPTY;        

    // Todo: handle en passant and castle


}

void undomove(Piece *board, Move move, int som) {


    // restore from
    board[getfrom(move)] = board[getto(move)];
    // restore capture
    board[getto(move)] = board[getpcpt(move)];        

    // Todo: handle en passant and castle

}

/* ############################# */
/* ###      root search      ### */
/* ############################# */

Move rootsearch(Piece *board, int som, int depth, Move lastmove) {

    int status =0;

    start = clock();

    status = initializeCL(board);

    status = runCLKernels(som, lastmove);

    end = clock();
    elapsed = ((double) (end - start)) / CLOCKS_PER_SEC;

    
    fflush(stdout);

    return bestmove;
}


/* ############################# */
/* ### main loop, and Xboard ### */
/* ############################# */
int main(void) {

    char line[256];
    char command[256];
    char c_usermove[256];
    char fen[256];
    int my_side = WHITE;
    int go = 0;
    Move move;
    Move usermove;

	signal(SIGINT, SIG_IGN);


    load_file_to_string(filename, &source);
    sourceSize    = strlen(source);

    for(;;) {
        fflush(stdout);
		if (!fgets(line, 256, stdin)) {
        }
		if (line[0] == '\n') {
			continue;
        }
		sscanf(line, "%s", command);

		if (!strcmp(command, "xboard")) {
			continue;
        }
        if (strstr(command, "protover")) {
			printf("feature myname=\"Zeta 0.0.0.1 \" reuse=0 colors=1 setboard=1 memory=1 smp=1 usermove=1 san=0 time=0 debug=1 \n");
			continue;
        }
		if (!strcmp(command, "new")) {
            setboard("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
			continue;
		}
		if (!strcmp(command, "setboard")) {
			sscanf(line, "setboard %s", fen);
            setboard(fen);
			continue;
		}
		if (!strcmp(command, "quit")) {
			return 0;
        }

		if (!strcmp(command, "force")) {
			continue;
		}
		if (!strcmp(command, "white")) {
			my_side = WHITE;
			continue;
		}
		if (!strcmp(command, "black")) {
			my_side = BLACK;
			continue;
		}
		if (!strcmp(command, "sd")) {
			sscanf(line, "sd %i", &max_depth);
			continue;
		}
		if (!strcmp(command, "memory")) {
			sscanf(line, "memory %i", &max_mem_mb);
			continue;
		}
		if (!strcmp(command, "cores")) {
			sscanf(line, "cores %i", &max_cores);
			continue;
		}
		if (!strcmp(command, "go")) {
            go =1;
            move = rootsearch(BOARD, my_side, max_depth, Lastmove);
            Lastmove = move;
            PLY++;
            domove(BOARD, move, my_side);
            print_movealg(move);
			continue;
		}
        if (strstr(command, "usermove")){
            if (!go) my_side = BLACK;
			sscanf(line, "usermove %s", c_usermove);
            usermove = move_parser(c_usermove, BOARD, !my_side);
            Lastmove = usermove;
            domove(BOARD, usermove, !my_side);
            PLY++;
            print_board(BOARD);
            if (go) {
                move = rootsearch(BOARD, my_side, max_depth, Lastmove);
                domove(BOARD, move, my_side);
                PLY++;
                Lastmove = move;
                print_board(BOARD);
                print_movealg(move);
            }
			continue;
        }
    }
   return 0;
}


/* ############################# */
/* ###        parser         ### */
/* ############################# */

Move move_parser(char *usermove, Piece *board, int som) {

    int file;
    int rank;
    Square from,to;
    Piece promo = PEMPTY;
    Piece pcpt = PEMPTY;
    Move move;
    char promopiece;

    file = (int)usermove[0] -97;
    rank = (int)usermove[1] -49;
    from = make_square(file,rank);
    file = (int)usermove[2] -97;
    rank = (int)usermove[3] -49;
    to = make_square(file,rank);

    from = (from/8)*16 + from;
    to   = (to/8)*16 + to;

    pcpt = board[to];

    // pawn promo piece
    promopiece = usermove[4];
    if (promopiece == 'q' || promopiece == 'Q' )
        promo = QUEEN;
    else if (promopiece == 'n' || promopiece == 'N' )
        promo = KNIGHT;
    else if (promopiece == 'b' || promopiece == 'B' )
        promo = BISHOP;
    else if (promopiece == 'r' || promopiece == 'R' )
        promo = ROOK;

    move = makemove(from, to, pcpt, promo);

    return move;
}

void setboard(char *fen) {

/*
(pos/8)*16 + pos

(pos88/16)*8 + pos88%16
*/

    int i, j, side;
    int index;
    int file = 0;
    int rank = 7;
    int pos  = 0;
    int pos88 = 0;
    char temp;
    char position[255];
    char som[1];
    char castle[4];
    char ep[2];
    char string[26] = {" P NKBRQ  pnkbrq/12345678"};

	sscanf(fen, "%s %s %s %s ", position, som, castle, ep);

    for(i=0;i<64;i++) {
        BOARD[i] = PEMPTY;
    }
    i =0;
    while (!(rank==0 && file==8)) {
        temp = fen[i];
        i++;        
        for (j=0;j<25;j++) {
    		if (temp == string[j]) {
                if (j == 16) {
                    rank--;
                    file=0;
                }
                else if (j >=17) {
                    file+=j-16;
                }
                else {
                    pos = (rank*8) + file;
                    pos88 = (pos/8)*16 + pos;
                    side = (j>7) ? 1 :0;
                    index = side? j-8 : j;
                    BOARD[pos88] = (side)? (index|CkB) : index ;
                    file++;
                }
                break;                
            }
        }
    }

    print_board(BOARD);
}


/* ############################# */
/* ###       print tools     ### */
/* ############################# */

void print_movealg(Move move) {

    char rankc[9] = "12345678";
    char filec[9] = "abcdefgh";
    char movec[5] = "";
    Square from = getfrom(move);
    Square to   = getto(move);
    Piece promo   = getpromo(move);

    from = (from/16)*8 + from%16;
    to   = (to/16)*8 + to%16;

    movec[0] = filec[square_file(from)];
    movec[1] = rankc[square_rank(from)];
    movec[2] = filec[square_file(to)];
    movec[3] = rankc[square_rank(to)];

    // pawn promo
    if ( promo) {
        if (promo == QUEEN )
            movec[4] = 'q';
        if (promo == ROOK )
            movec[4] = 'r';
        if (promo == BISHOP )
            movec[4] = 'b';
        if (promo == KNIGHT )
            movec[4] = 'n';
    }

    printf("move %s\n", movec);
    fflush(stdout);

}

void print_board(Piece *board) {

    int i,j,pos;
    int pos88;
    char wpchars[10] = "-P NKBRQ";
    char bpchars[10] = "- pnkbrq";

    printf("###ABCDEFGH###\n");

    for(i=8;i>0;i--) {
        printf("#%i ",i);
        for(j=0;j<8;j++) {
            pos = ((i-1)*8) + j;
            pos88 = (pos/8)*16 + pos;

            if (board[pos88] && (board[pos88]&CkB))
                printf("%c", bpchars[(board[pos88]^CkB)]);
            else if ((board[pos88]))
                printf("%c", wpchars[board[pos88]]);
            else 
                printf("-");
       }
       printf("\n");
    }
    fflush(stdout);
}


void print_stats() {
    FILE 	*Stats;
    Stats = fopen("zeta_amd.debug", "ab+");
    fprintf(Stats, "nodes: %lu ,moves: %lu, ,sec: %f \n", NODECOUNT, MOVECOUNT, elapsed);
    fclose(Stats);
}

