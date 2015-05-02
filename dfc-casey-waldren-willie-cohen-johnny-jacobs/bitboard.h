#ifndef __BITBOARD_H__
#define __BITBOARD_H__

typedef struct state {
	unsigned long long board;
	unsigned long long children;
	float val;
} state_t;


unsigned int iter_count(unsigned long long board);
unsigned int bit_count(unsigned long long board);
unsigned char compute_white_moves(unsigned char w, unsigned char b);
unsigned char compute_white_moves(unsigned char w, unsigned char b);
void compute_all_moves(unsigned char moves[256][256][2]);
unsigned long long r90(unsigned long long board);
unsigned long long l90(unsigned long long board);
unsigned long long r45(unsigned long long board);
unsigned long long l45(unsigned long long board);
void compute_rotations(unsigned long long board[2][4]);
unsigned long long generate_moves(unsigned long long board[2][4], int color);
float heuristics(unsigned long long board);
unsigned long long flip(unsigned long long w, unsigned long long b, unsigned long long move);
unsigned long long generate_child(unsigned long long board[2][4], unsigned long long move, int color);

#endif