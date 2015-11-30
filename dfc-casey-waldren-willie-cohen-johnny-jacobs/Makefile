CC = gcc
CFLAGS = -O3 -std=c99 
DEPS = structs.h
OBJ = bitboard.o

#%#.o: %.c $(DEPS)
	#$(CC) $(CFLAGS) -c -o $@ $<

#player: $(OBJ)
	#gcc $(CFLAGS) -o $@ $^

othello: bitboard.c
	gcc -O3 -std=c99 bitboard.c -o player
clean:
	rm -f player

