CC = gcc
CFLAGS = -std=c99 -g -Wall -Wpedantic -Wextra -fsanitize=address,undefined

nsfd: nfsd.c nfsd.h
	$(CC) $(CFLAGS) nfsd.c -o nfsd

clean:
	rm -f nfsd
