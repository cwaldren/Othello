CC = gcc
CFLAGS = -O3 -std=c99 
DEPS = bitboard.h
OBJ = player.o bitboard.o

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<

player: $(OBJ)
	gcc $(CFLAGS) -o $@ $^

#othello: player.c bitboard.c
#	gcc -O3 -std=c99 player.c -o player
clean:
	rm -f player bitboard.o player.o

