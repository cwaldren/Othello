#include <stdio.h>
#include <stdlib.h>
#include "math.h"
#define WHITE 0
#define BLACK 1
#define R0 0 //0 rotation
#define R90 1 //90 degrees right rotation
#define R45 2 //45 degrees right rotation
#define L45 3 //45 degrees left rotation
unsigned char mask[8] = {0xffu,0xfeu,0xfcu,0xf8u,0xf0u,0xe0u,0xc0u,0x80u}; //mask out, respectively, no bit, far right bit, far right 2 bits, etc.
unsigned char moveTable[256][256][2]; //stores all moves (by row) based on [white row config][black row config][color to move]

typedef unsigned long long ull;
/*
 * Counts bits iteratively
 */
//credit to Gunnar Andersson's wzebra for the idea of this function
unsigned int iter_count(unsigned long long board) {
	unsigned int n;
	n = 0;
	for ( ; board != 0; n++, board &= (board - 1) );

	return n;
}

/*
 * Counts bits with bit fiddling
 */
//credit to Gunnar Andersson's wzebra for the idea of this function
unsigned int bit_count(unsigned long long board) {
	board = board - ((board >> 1) & 0x5555555555555555u);
	board = (board & 0x3333333333333333u) + ((board >> 2) & 0x3333333333333333u);
	board = (board + (board >> 4)) & 0x0F0F0F0F0F0F0F0Fu;
	return ((board) * 0x0101010101010101u) >> 56;
}

/*
 * Compute white moves (for moveTable) based on row config w(hite) and b(lack)
 */
unsigned char compute_white_moves(unsigned char w, unsigned char b){
	if(!(w^b)) //no pieces in this area or pieces occupy the same space
		return 0;

	unsigned char i = 0;
	unsigned char moves = ~(w|b); //can't play in occupied slots

	//must play across black from white
	unsigned char lmoves = 0;
	unsigned char t = moves & b<<1;
	unsigned char t1 = 0;
	for(i = 1; i<8; i++){ //look right
		t >>= 1;
		t1 = t&b;
		if(t ^ t1)
			lmoves |= ((t ^ t1) & w)<<i;
		t = t1;
	}
	t = moves & b>>1;
	for(i = 1; i<8; i++){ //look left
		t <<= 1;
		t1 = t&b;
		if(t ^ t1)
			lmoves |= ((t ^ t1) & w)>>i;
		t = t1;
	}
	return (moves & lmoves);
}


/*
 * Compute black moves (for moveTable) based on row config i(white) and j(black)
 * (done by pretending colors are reversed and finding white moves)
 */
unsigned char compute_black_moves(unsigned char i, unsigned char j){
	return compute_white_moves(j, i);
}

/*
 * Fill moveTable (pre-computation)
 */
void compute_all_moves(unsigned char moves[256][256][2]){
	unsigned int i;
	unsigned int j;
	for (i = 0; i < 256; i++) {
		for (j = 0; j < 256; j++) {
			//Compute all legal white moves given i and j
			moves[i][j][WHITE] = compute_white_moves(i,j);

			//Compute all legal black moves given i and j
			moves[i][j][BLACK] = compute_black_moves(i,j);
		}
	}
}

/*
 * Rotate board by 90 degrees to the right
 */
unsigned long long r90(unsigned long long board){
	unsigned long long rBoard = 0;
	unsigned long long w = 0x1u; //needed to make b be 64 bits
	unsigned char i;
	unsigned char j;
	for (i = 0; i < 8; i++) {
		for (j = 0; j < 8; j++) {
			int x = 63-(j*8+i); //convert i,j to 1D
			unsigned long long b = (board & (w<<x)) != 0; //get bit at i,j
			int y = 63-(7-j+i*8); //convert 7-j, i to 1D
			rBoard |= (b<<y); //set 7-j, i to bit b
		}
	}
	return rBoard;
}

/*
 * Rotate board by 90 degrees to the left
 */
unsigned long long l90(unsigned long long board){
	unsigned long long rBoard = 0;
	unsigned long long w = 0x1u; //needed to make b be 64 bits

	unsigned char i;
	unsigned char j;
	for (i = 0; i < 8; i++) {
		for (j = 0; j < 8; j++) {
			int x = 63-(j*8+i); //convert i,j to 1D
			unsigned long long b = (board & (w<<x)) != 0; //get bit at i,j
			int y = 63-((7-i)*8+j); //convert j, 7-i to 1D
			rBoard |= (b<<y); //set j, 7-i to bit b
		}
	}
	return rBoard;
}

/*
 * Rotate board by 45 degrees to the right
 */
unsigned long long r45(unsigned long long board){
	unsigned long long rBoard = 0;
	unsigned long long w = 0x1u; //needed to make b be 64 bits

	unsigned char i;
	unsigned char j;
	for (i = 0; i < 8; i++) {
		for (j = 0; j < 8; j++) {
			int x = 63-(j*8+i); //convert i,j to 1D
			unsigned long long b = (board & (w<<x)) != 0; //get bit at i,j
			int z = j-i;
			if(z < 0)
				z += 8;
			int y = 63-(z*8+i);
			rBoard |= (b<<y);
		}
	}
	return rBoard;
}

/*
 * Rotate board by 45 degrees to the left
 */
unsigned long long l45(unsigned long long board){
	unsigned long long rBoard = 0;
	unsigned long long w = 0x1u; //needed to make b be 64 bits

	unsigned char i;
	unsigned char j;
	for (i = 0; i < 8; i++) {
		for (j = 0; j < 8; j++) {
			int x = 63-(j*8+i); //convert i,j to 1D
			unsigned long long b = (board & (w<<x)) != 0; //get bit at i,j
			int z = j+i;
			if(z > 7)
				z -= 8;
			int y = 63-(z*8+i);
			rBoard |= (b<<y);
		}
	}
	return rBoard;
}

/*
 * Computes all other rotations based on boards in board[WHITE][R0] and board[BLACK][R0]
 * and then stores them
 */
void compute_rotations(unsigned long long board[2][4]){
	board[WHITE][R90] = r90(board[WHITE][R0]);
	board[WHITE][R45] = r45(board[WHITE][R0]);
	board[WHITE][L45] = l45(board[WHITE][R0]);

	board[BLACK][R90] = r90(board[BLACK][R0]);
	board[BLACK][R45] = r45(board[BLACK][R0]);
	board[BLACK][L45] = l45(board[BLACK][R0]);
}

/*
 * Generate moves using moveTable, current board, and color to move
 */
unsigned long long generate_moves(unsigned long long board[2][4], int color){
	unsigned long long boardR0 = 0;
	unsigned long long boardR90 = 0;
	unsigned long long boardR45 = 0;
	unsigned long long boardL45 = 0;

	unsigned char i = 0;
	for(i = 0; i<8; i++){ //must cast to long long because fuck C (all untyped numbers are int by default
		boardR0 |= (long long)moveTable[(board[WHITE][R0]>>(8*(7-i)))&mask[0]][(board[BLACK][R0]>>(8*(7-i)))&mask[0]][color] << (8*(7-i));
		boardR90 |= (long long)moveTable[(board[WHITE][R90]>>(8*(7-i)))&mask[0]][(board[BLACK][R90]>>(8*(7-i)))&mask[0]][color] << (8*(7-i));
		boardR45 |= ((long long)moveTable[(board[WHITE][R45]>>(8*(7-i)))&mask[i]][(board[BLACK][R45]>>(8*(7-i)))&mask[i]][color]&mask[i]) << (8*(7-i));
		boardL45 |= ((long long)moveTable[(board[WHITE][L45]>>(8*(7-i)))&mask[7-i]][(board[BLACK][L45]>>(8*(7-i)))&mask[7-i]][color]&mask[7-i]) << (8*(7-i));
		boardR45 |= ((long long)moveTable[(board[WHITE][R45]>>(8*(7-i)))&(~mask[i]&mask[0])][(board[BLACK][R45]>>(8*(7-i)))&(~mask[i]&mask[0])][color]&(~mask[i]&mask[0])) << (8*(7-i));
		boardL45 |= ((long long)moveTable[(board[WHITE][L45]>>(8*(7-i)))&(~mask[7-i]&mask[0])][(board[BLACK][L45]>>(8*(7-i)))&(~mask[7-i]&mask[0])][color]&(~mask[7-i]&mask[0])) << (8*(7-i));
	}
	return boardR0|l90(boardR90)|l45(boardR45)|r45(boardL45);

}

/*
 * Calculate value of a board position
 */
float heuristics(unsigned long long board){
	float value = 0;
	float corner = 100;
	float closeCorner = -50;
	float piece = .01;
	unsigned long long temp;


	value = iter_count(board&0x8100000000000081u)*corner;

	//TODO: close corner

	value += iter_count(board)*piece;
}

/*
 * Flip all pieces in row given by w(hite) and b(lack) given move
 */
unsigned long long flip(unsigned long long w, unsigned long long b, unsigned long long move){

	unsigned long long flipped = 0;

	unsigned long long t = move;
	unsigned char flip = 0;
	unsigned char i = 0;

//	printf("w:%016I64x\n", w);
	//printf("b:%016I64x\n", b);
	////printf("move:%016I64x\n", move);
	if((move>>1&b)||(move<<1&b)){
		printf("-----------\n");
		//RIGHT
		for(i = 0; i<8; i++){ //look right
			t>>=1;
			t&=b;
			//printf("tr:%016I64x\n", t);
			if(!t){
				flip = t&(w>>i);
				break;
			}
		}
		t = move;
		if(flip)
			for(i = 0; i<8; i++){ //flip right
				t>>=1;
				t&=b;
				flipped |= t;
				if(!t)
					break;
			}
		//LEFT
		t = move;
		flip = 0;
		for(i = 0; i<8; i++){ //look left
				t<<=1;
				t&=b;
				//printf("tl:%016I64x\n", t);
				if(!t){
					flip = move&(w>>i);
					printf("fl:%d\n", flip);
					break;
				}
			}
		t = move;
		if(flip)
			for(i = 0; i<8; i++){ //flip left
				t<<=1;
				t&=b;
				flipped |= t;
				if(!t)
					break;
			}
	}
	return flipped;
}

/*
 * Generate a child given current board, next move, and color to move
 */
unsigned long long generate_child(unsigned long long board[2][4], unsigned long long move, int color){
	static unsigned long long newBoard[2];

	unsigned long long flipped;
	flipped =
			flip(board[color][R0], board[abs(color-1)][R0],move)
			|l90(flip(board[color][R90], board[abs(color-1)][R90],r90(move)))
			|l45(flip(board[color][R45], board[abs(color-1)][R45],r45(move)))
			|r45(flip(board[color][L45], board[abs(color-1)][L45],l45(move)))
			|l45(flip(board[color][R45], board[abs(color-1)][R45],r45(move)))
			|r45(flip(board[color][L45], board[abs(color-1)][L45],l45(move)));

	newBoard[color] = flipped|move|board[color][0]; //add flipped pieces and this move to players old board
	newBoard[abs(color-1)] = board[abs(color-1)][0]&~flipped; //remove flipped pieces from opponents old board
	return flipped;
}

/*
 * Generate all children given current board, board of possible moves, and color to move
 *//*
unsigned long long generate_children(unsigned long long **board, unsigned long long moves, int color){
	if(color){
		unsigned long long *temp = 0;
		temp = board[WHITE];
		board[WHITE] = board[BLACK];
		board[BLACK] = temp;
	}

	unsigned char i = 0;
	for(i = 0; i<8; i++){
		unsigned long long row = moves&(0xff<<(8*(7-i)));
		generate_child(board, row&(~row+1), i);
		row&=row-1;
		if(row)
			generate_child(board, row, i);
		//TODO: attach children to each other somehow
	}
}*/

/*
 * Me fucking around/testing
 */
 /*
int main()
{
	//unsigned char taco = compute_white_moves((unsigned char)129, (unsigned char) 78);
	//printf("r:%x\n", taco);

	//generate_children(board, generate_moves(board, color), color);
	//heuristics(board);


	compute_all_moves(moveTable);
	unsigned long long board[2][4];
	board[WHITE][R0] = 0x8040203424000000;
	board[BLACK][R0] = 0x00081808183c0000;
	compute_rotations(board);


	printf("moves:%016I64X\n",generate_moves(board, BLACK)); //this works
	printf("child:%016I64X\n", generate_child(board, 0x0020000000000000 ,BLACK)); //this don't

	//unsigned long long board = 0x8100000000000081;

	//printf("0:%I64X\n", board[0][0]);
	/*printf("r45:%I64X\n", r90(board[1][0]));
	printf("0:%I64X\n", l90(r90(board[1][0])));
	*/

//	return 0;
//}
