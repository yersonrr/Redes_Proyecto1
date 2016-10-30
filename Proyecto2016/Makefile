CC = gcc
CFLAGS = -g

all: servidor cliente

servidor: servidor.c
	$(CC) $(CFLAGS) -lpthread -o bsb_svr servidor.c

cliente: cliente.c
	$(CC) $(CFLAGS) -o bsb_cli cliente.c
	
clean:
	rm -fr *.o bsb_svr bsb_cli