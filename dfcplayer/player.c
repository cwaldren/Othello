#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <float.h>
#define FALSE 0
#define TRUE 1

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

int me;
int depthlimit, timelimit1, timelimit2;
int turn;
typedef signed char board[8][8];

// Holds our different heuristics values. Min is not minimum -- just a reference to the 'min' player. Same for max.
typedef struct heuristics {
    int maxScore;
    int minScore;

    int maxMoves;
    int minMoves;

    int maxCorners;
    int minCorners;

} heuristics_t;

// Holds the state of the game. The game board, where the last piece was placed, and the value of the game. 
typedef struct state {
    signed char board[8][8];
    int x;
    int y;
    float val;
    struct state* next;
} state_t;


// The current board that we are doing operations on.
board gamestate;

void error(char * msg)
{
    fprintf(stderr,"%s\n", msg);
    exit(-1);
}

// Helper function to new up a heuristics struct
heuristics_t* newHeuristics(int maxScore, int maxMoves, int maxCorners,
                            int minScore, int minMoves, int minCorners) 
{
    heuristics_t *s = NULL;
    s = malloc(sizeof(heuristics_t));
    
    s->maxScore = maxScore;
    s->maxMoves = maxMoves;
    s->maxCorners = maxCorners;
    
    s->minScore = minScore;
    s->minMoves = minMoves;
    s->minCorners = minCorners;
    
    return s;
}

// Helper function to new up a state struct.
state_t* newState() {
    state_t *s = NULL;
    s = malloc(sizeof(state_t));
    s->next = NULL;

    // Zero out the board
    for (int x = 0; x < 8; x++) {
        for (int y = 0; y < 8; y++) {
            s->board[x][y] = 0;
        }
    }

    s->x = -1;
    s->y = -1;
    s->val = 0;

    return s;
}

/* The following are provided functions */
void printboard(board state, int player, int turn, int X, int Y)
{
    int i, j, num;
    num = 64;

    fprintf(stderr, "%2d. %c plays (%2d, %2d)\n", turn, (player==1 ? 'B' : 'W'), X, Y);
    for(i = 0; i < 8; i ++) {
        for(j = 0; j < 8; j++){
            fprintf(stderr, "%4d", state[i][j]);
            if(state[i][j]) num --;
        }
    printf("\n");
    }
    printf("number of blanks = %d\n", num);
}

void DoFlip(board state, int player, int X, int Y, int dirX, int dirY)
{
    while (X+dirX < 8 && X+dirX >= 0 && Y+dirY < 8 && Y+dirY >= 0 && state[X+dirX][Y+dirY]==-player) {
        X = X+dirX; Y = Y+dirY;
        state[X][Y] = player;
    }
}
int CanFlip(board state, int player, int X, int Y, int dirX, int dirY) 
{
  //  printf("Player %d: Checking position %d %d\n", player, X, Y);
    int capture = FALSE;
    while (X+dirX < 8 && X+dirX >= 0 && Y+dirY < 8 && Y+dirY >= 0 && state[X+dirX][Y+dirY]==-player) {
        X = X+dirX; Y = Y+dirY;
        capture = TRUE;
    }
    if (capture == FALSE) return FALSE;
    if (X+dirX < 8 && X+dirX >= 0 && Y+dirY < 8 && Y+dirY >= 0 && state[X+dirX][Y+dirY]==player)
        return TRUE;
    else return FALSE;
}
int Legal(board state, int player, int X, int Y)
{

    int i,j, captures;
    captures = 0;
    if (state[X][Y]!=0) return FALSE;
    for (i=-1; i<=1; i++)
    for (j=-1; j<=1; j++)
        if ((i!=0 || j!=0) && CanFlip(state, player, X, Y, i, j)) {
           // printf("It's a legal move.\n");
            return TRUE;
        }
   /// printf("It's an ILLEGAL move\n");
    return FALSE;
}

int GameOver(board state)
{
     //printboard(state, 1, 0, 0,0 );

    int X,Y;

    for (X=0; X<8; X++) {
        for (Y=0; Y<8; Y++) {
            if (Legal(state, 1, X, Y)) return FALSE;
            if (Legal(state, -1, X, Y)) return FALSE;
         //   printf("Position %d, %d had no legal moves.\n", X, Y);
        }
    }
   // printf("no!");
    return TRUE;
}

void Update(board state, int player, int X, int Y)
{
    int i,j;

    if (X<0) return; /* pass move */
    if (state[X][Y] != 0) {
   // printboard(state, player, turn, X, Y);
    error("Illegal move");
    }
    state[X][Y] = player;
    for (i=-1; i<=1; i++)
    for (j=-1; j<=1; j++)
        if ((i!=0 || j!=0) && CanFlip(state, player, X, Y, i, j))
        DoFlip(state, player, X, Y, i, j);
}

/* Our functions begin here */

void copyFirstBoardToSecond(state_t *a, state_t *b) {
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {               
            b->board[i][j] = a->board[i][j];
        }
    }
}
// Generates the different states that could be reachable from the current one.
void generateChildren(state_t *s, int player, state_t *head) {
    state_t *current = head;
  
    for (int x = 0; x < 8; x++) {
        for (int y = 0; y < 8; y++) {
            if (Legal(s->board, player, x, y)) {
                // Copy in the current state of the game to our child.
                copyFirstBoardToSecond(s, current);

                // Basically stick a piece down here and flip over pieces, and update the board to reflect those changes
                Update(current->board, player, x, y);

                // Also store what we did
                current->x = x;
                current->y = y;
                current->next = newState();

                // printf("A child is:\n");
                // printboard(current->board, player, 0, x, y); 
                current = current->next;
            }
        }
    }
}


// Helper function that just checks if a piece is in a corner. Returns +1 (yes, add 1 to the total) or +0 (add nothing to total) instead of true or false
// because of premature optimization. 
int isCorner(int x, int y) {
    return ((x == 0 || x == 7) && (y == 0 || y == 7)) ? 1 : 0;
}

// Calculates the hopefully best moves
heuristics_t* calcHeuristics(state_t *b) {
    int minScore = 0;
    int maxScore = 0;

    int minMoves = 0;
    int maxMoves = 0;

    int minCorners = 0;
    int maxCorners = 0;

    for (int x = 0; x < 8; x++) {
        for (int y = 0; y < 8; y++) {
            // Cache some variables (goddamnit 252)
            int player = b->board[x][y];
            int corner = isCorner(x, y);

            if (Legal(b->board, 1, x, y)) {
                maxMoves++;
            }

            if (Legal(b->board, -1, x, y)) {
                minMoves++;
            }

            if (player == 1) {
                maxScore++;
                maxCorners += corner;
            } else 
            if (player == -1) {
                minScore++;
                minCorners += corner;
            }
        }
    }
    return newHeuristics(maxScore, maxMoves, maxCorners, minScore, minMoves, minCorners);
}

// Evaluates an end game (or max depth) state. Weights the values appropriately. 
state_t * evaluate(state_t *b) {
    heuristics_t* heuristics = calcHeuristics(b);

    // Cache variables so we have only 1 pointer access per var at the expense of 6 ints. No idea if worth it.
    int maxScore = heuristics->maxScore;
    int maxMoves = heuristics->maxMoves;

    int minScore = heuristics->minScore;
    int minMoves = heuristics->minMoves;

    int minCorners = heuristics->minCorners;
    int maxCorners = heuristics->maxCorners;

    float coinParity = 100.0 * (maxScore - minScore) / (maxScore + minScore);
    float mobility = 0;
    float corners = 0;

    if (maxMoves + minMoves != 0) {
        mobility = 100.0 * (maxMoves - minMoves) / (maxMoves + minMoves);
    }

    if (maxCorners + minCorners != 0) {
        corners = 100.0 * (maxCorners - minCorners) / (maxCorners + minCorners);
    }

    float score = (coinParity * 10)  + (800 * corners) + (80 * mobility);
    b->val = score;
    return b;
}



state_t * minimax(state_t *state, int depth, int player) {
     //  printf("Entering minimax at depth %d\n", depth);
    if (depth == 0 || GameOver(state->board)) {
        /*if (depth == 0) {
            printf("Since depth=0, must eval\n");
        } else {
           printf("Since the game is over, must eval\n");

        }*/
        return evaluate(state);
    }

    if (player == 1) {
       // printf("Player is 1. Attempting to generate children.\n");
        float bestVal = FLT_MIN;
        state_t *firstChild = newState();
        state_t *bestState = firstChild;
        generateChildren(state, player, firstChild);
        state_t *current = firstChild;

        while (current != NULL) {
            printboard(current->board, player, 0, 0,0);
            float val = minimax(current, depth - 1, -1)->val;
            bestVal = MAX(bestVal, val);
            current->val = bestVal;
            if (bestVal == val) {
                bestState = current;
            }
            current = current->next;
        }
        return bestState;

    } else {
        float bestVal = FLT_MAX;
        state_t *firstChild = newState();
        state_t *bestState = firstChild;
        generateChildren(state, player, firstChild);
        state_t *current = firstChild;

        while (current != NULL) {
                        printboard(current->board, player, 0, 0,0);

            float val = minimax(current, depth - 1, 1)->val;
            bestVal = MIN(bestVal, val);
            current->val = bestVal;
            if (bestVal == val) {
                bestState = current;
            }
            current = current->next;
        }
        return bestState;
    }
}


void newGame(void)
{
    int X, Y;
    for (X=0; X<8; X++) {
        for (Y=0; Y<8; Y++) {
            gamestate[X][Y] = 0;
        }
    }
    gamestate[3][3] = -1;
    gamestate[4][4] = -1;
    gamestate[3][4] = 1;
    gamestate[4][3] = 1;
}



void makeMove() {
    state_t *initialState = newState();
    for (int x = 0; x < 8; x++)
        for (int y = 0; y < 8; y++)
            initialState->board[x][y] = gamestate[x][y];
   
    // Depth 3
    state_t *bestState = minimax(initialState, 7, me);
 
    Update(gamestate, me, bestState->x, bestState->y);
  
    //need pass logic tho
    printboard(gamestate, me, turn, bestState->x, bestState->y);
    printf("%d %d\n", bestState->x, bestState->y);
    fflush(stdout);
}
int main(int argc, char** argv) {
    char inbuf[256];
    char playerstring[1];
    int X,Y;

    turn = 0;
    //fgets(inbuf, 256, stdin);
    //if (sscanf(inbuf, "game %1s %d %d %d", playerstring, &depthlimit, &timelimit1, &timelimit2) != 4) {
      //  error("Bad initial input");
   // }
    me =1;// (playerstring[0] == 'B') ? 1 : -1;
    newGame();
    if (me == 1) {
        makeMove();
    }
    while (fgets(inbuf, 256, stdin) != NULL) {
        if (strncmp(inbuf, "pass", 4) != 0) {
            if (sscanf(inbuf, "%d %d", &X, &Y) != 2) {
                return 0;
            }
            Update(gamestate, -me, X, Y);
            printboard(gamestate,-me,turn, X, Y);

        }
        makeMove();
    }
    return 0;
}