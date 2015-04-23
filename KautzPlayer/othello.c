#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#define FALSE 0
#define TRUE 1

typedef signed char board[8][8];

int me;
int depthlimit, timelimit1, timelimit2;
board gamestate;

int debug = FALSE;
int turn;

void error(char * msg)
{
    fprintf(stderr,"%s\n", msg);
    exit(-1);
}


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


void NewGame(void)
{
    int X,Y;
    for (X=0; X<8; X++)
	for (Y=0; Y<8; Y++)
	    gamestate[X][Y] = 0;
    gamestate[3][3] = -1;
    gamestate[4][4] = -1;
    gamestate[3][4] = 1;
    gamestate[4][3] = 1;
}





int CanFlip(board state, int player, int X, int Y, int dirX, int dirY) 
{
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
	    if ((i!=0 || j!=0) && CanFlip(state, player, X, Y, i, j))
		return TRUE;
    return FALSE;
}


void DoFlip(board state, int player, int X, int Y, int dirX, int dirY)
{
    while (X+dirX < 8 && X+dirX >= 0 && Y+dirY < 8 && Y+dirY >= 0 && state[X+dirX][Y+dirY]==-player) {
        X = X+dirX; Y = Y+dirY;
        state[X][Y] = player;
    }
}

void Update(board state, int player, int X, int Y)
{
    int i,j;

    if (X<0) return; /* pass move */
    if (state[X][Y] != 0) {
	printboard(state, player, turn, X, Y);
	error("Illegal move");
    }
    state[X][Y] = player;
    for (i=-1; i<=1; i++)
	for (j=-1; j<=1; j++)
	    if ((i!=0 || j!=0) && CanFlip(state, player, X, Y, i, j))
		DoFlip(state, player, X, Y, i, j);
}

    

void Result(board oldstate, board newstate, int player, int X, int Y)
{
    int i, j;
    for (i=0; i<8; i++)
	for (j=0; j<8; j++)
	    newstate[i][j] = oldstate[i][j];
    Update(newstate, player, X, Y);
}

int GameOver(board state)
{
    int X,Y;

    for (X=0; X<8; X++)
	for (Y=0; Y<8; Y++) {
	    if (Legal(state, 1, X, Y)) return FALSE;
	    if (Legal(state, -1, X, Y)) return FALSE;
	}
    return TRUE;
}



void MakeMove(void)
{
    int X,Y;
	int a =0;
	int b = 0;

	X = -1;
	Y = -1;
	for(a = 0; a<8; a++)
	{
		for(b = 0; b<8; b++)
		{
			if(Legal(gamestate,me, a,b))
			{
				X = a;
				Y = b;
				break;
			}
		}
	}

    if (X>=0) {
	Update(gamestate, me, X, Y);
	printf("%d %d\n", X, Y);
	fflush(stdout);
    } else {
	printf("pass\n");
	fflush(stdout);
    }
    if (debug) printboard(gamestate, me, ++turn, X, Y);
}


int main(int argc, char** argv)
{
    char inbuf[256];
    char playerstring[1];
    int X,Y;

    if (argc >= 2 && strncmp(argv[1],"-d",2)==0) debug = TRUE;
    turn = 0;
    fgets(inbuf, 256, stdin);
    if (sscanf(inbuf, "game %1s %d %d %d", playerstring, &depthlimit, &timelimit1, &timelimit2) != 4) error("Bad initial input");
    depthlimit = 4;
    if (playerstring[0] == 'B') me = 1; else me = -1;
    NewGame();
    if (me == 1) MakeMove();
    while (fgets(inbuf, 256, stdin)!=NULL){
	if (strncmp(inbuf,"pass",4)!=0) {
	    if (sscanf(inbuf, "%d %d", &X, &Y) != 2) return 0;
	    Update(gamestate, -me, X, Y);
	    if (debug) printboard(gamestate, -me, ++turn, X, Y);
	}
	MakeMove();
    }
    return 0;
}

    



