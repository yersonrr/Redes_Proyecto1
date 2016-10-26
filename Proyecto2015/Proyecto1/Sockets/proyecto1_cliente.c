#include <stdio.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>

#define MAXHOSTNAME 1000

void parse_arguments(int argc, char *argv[], int *r, int *c) {
	char * bad_syntax_msg = "Sintaxis errada de argumentos del programa. Sintaxis correcta: reserva_bol_cli <ip-servidor> -p <puerto> -f <fila> -c <columna>\n";

	if (argc != 8) {
		printf(bad_syntax_msg);
		exit(1);
	}

    if (strcmp(argv[2], "-p")) {
    	printf(bad_syntax_msg);
    	exit(1);
    }
    
    if (!strcmp(argv[4], "-f"))
        *r = atoi(argv[5]);
    else {
    	printf(bad_syntax_msg);
    	exit(1);
    }

    if (!strcmp(argv[6], "-c"))
        *c = atoi(argv[7]);
    else {
    	printf(bad_syntax_msg);
    	exit(1);
    }
}


int main (int argc, char *argv[]) {
	char buffer[50];
    int sock, try, r, c, i, j, a, len, R, C;
    struct addrinfo hints, *servinfo;

	parse_arguments(argc, argv, &r, &c);
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(argv[1], argv[3], &hints, &servinfo) != 0) {
        printf("ERROR: no se pudo resolver el nombre del servidor.\n");
        return -1;
    }

 	while (1) {
 		sock = socket(AF_INET, SOCK_STREAM, 0);
 		if (connect(sock, servinfo->ai_addr, servinfo->ai_addrlen) != 0) {
 			printf("ERROR: no se pudo conectar con el servidor.\n");
        	return -1;
 		}

	 	sprintf(buffer, "%d %d", r, c);
	 	try = 3;

	 	while (try--)
	 		if (send(sock, buffer, strlen(buffer), 0) >= 0) break;

	 	if (try == -1) {
	 		printf("ERROR: tiempo de respuesta del servidor agotado.\n");
	        return -1;
	 	}

	    len = recv(sock, buffer, 50, 0);
	    switch (buffer[0]) {
	    	case '0':
	    		printf("Vagon lleno.\n");
	    		return -1;
	    	case '1':
	    		printf("Puesto ocupado.\nPuestos disponibles:\n ");
	    		buffer[len] = '\0';
	    		//printf("%d :: %s\n", strlen(buffer), buffer);
	    		R = buffer[1]-48;
	    		C = buffer[2]-48;
	    		if (R == 0) R = 10;
	    		//printf("%d %d\n", R,C);
	            for (i=0; i<R; i++) {
	                for (j=0; j<C; j++) {
	                    if ((buffer+3)[i*C+j] != '1') printf("(%d,%d) | ", i+1,j+1);
	                }
	            }

	    		printf("\nIngrese numero de fila y columna (<fila> <columna>): ");
	    		scanf("%d %d", &r, &c);
	    		close(sock);
	    		break;
	    	case '2':
	    		printf("Puesto reservado con exito.\n");
	    		return 0;
	    	case '3':
	    		printf("Rango errado.\n");
	    		return -1;
	    	default:
	    		printf("El servidor esta presentando una falla. Recibido: %c.\n", buffer[0]);
	    		return -1;
	    }
	}
    close(sock);
}