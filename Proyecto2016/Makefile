CC = gcc
CFLAGS = -g

all: servidor cliente

servidor: servidor.c
	$(CC) $(CFLAGS) -lpthread -o servidor servidor.c

cliente: cliente.c
	$(CC) $(CFLAGS) -o cliente cliente.c
	
clean:
	rm -fr *.o servidor cliente