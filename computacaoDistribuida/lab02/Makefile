CC = gcc
CFLAGS = -Wall -Wextra -pedantic -std=c11
LDFLAGS = -lpthread

all: servidor cliente

servidor: servidor.c jogo.c
	$(CC) $(CFLAGS) -o servidor servidor.c jogo.c $(LDFLAGS)

cliente: cliente.c jogo.c
	$(CC) $(CFLAGS) -o cliente cliente.c jogo.c

clean:
	rm -f servidor cliente
