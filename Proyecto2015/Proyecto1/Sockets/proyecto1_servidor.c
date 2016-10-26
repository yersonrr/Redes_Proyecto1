#include <stdio.h>
#include <sys/socket.h>
#include <netdb.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_THREADS 100

pthread_mutex_t lock;
FILE *fp;

char m[10][4];
int used_seats,
	MAX_SEATS,
	PORTNUM = 5000,
	R,
	C;

void log_(struct sockaddr_in destiny, char * action) {
    char str[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(destiny.sin_addr), str, INET_ADDRSTRLEN);
    printf("%s :: %s\n", str, action);
}

void parse_arguments(int argc, char *argv[]) {
	char * bad_syntax_msg = "Sintaxis errada de argumentos del programa. Sintaxis correcta: reserva_bol_ser -f <filas> -c <columnas> [-p puerto]\n";
	char * bad_range = "ERROR: Rango invalido. Correcto: 1 <= filas <= 10, 1 <= columnas <= 4\n";
	if (argc != 5 && argc != 7) {
		printf(bad_syntax_msg);
		exit(1);
	}

    if (!strcmp(argv[1], "-f"))
        R = atoi(argv[2]);
    else {
    	printf(bad_syntax_msg);
    	exit(1);
    }
    
    if (!strcmp(argv[3], "-c"))
        C = atoi(argv[4]);
    else {
    	printf(bad_syntax_msg);
    	exit(1);
    }

    if (R < 1 || R > 10 || C < 1 || C > 4) {
    	printf(bad_range);
    	exit(1);
    }

    MAX_SEATS = R*C;

    if (argc == 5) return;  // use default port 5000

    if (!strcmp(argv[5], "-p"))
        PORTNUM = atoi(argv[6]);
    else {
    	printf(bad_syntax_msg);
    	exit(1);
    }
}


void *manage_request (void *arg) {
	printf("Manejando solicitud en nuevo hilo.\n");
	char buffer[60];
    char numbers[] = {'0','1','2','3','4','5','6','7','8','9','0'};
	int sock = *((int *) arg), len, r, c, i, j, a;
  	len = recv(sock, buffer, 10, 0);
   	buffer[len] = '\0';
   	sscanf(buffer, "%d %d", &r, &c);

	if (r < 1 || r > R || c < 1 || c > C) {
		send(sock, "3", 1, 0);	// 3:= Rango errado
		close(sock);
		return 0;
	}
	if (used_seats == MAX_SEATS) {
		send(sock, "0", 1, 0);	  // 0 := Vagon completo
    }
	else {
        pthread_mutex_lock(&lock);
		if (m[r-1][c-1] != '0') {
            sprintf(buffer, "1%c%c0000000000000000000000000000000000000000", numbers[R],numbers[C]);
            for (i=0; i<R; i++) {
                for (j=0; j<C; j++) {
                    (buffer+3)[i*C+j] = m[i][j];
                }
            }
            len = send(sock, buffer, 43, 0);
            //printf("%d :: %s\n", len, buffer);
        }
		else {
			m[r-1][c-1] = '1'; 	  // 1-index => 0-index
			used_seats++;
			send(sock, "2", 1, 0);// 2 := Reserva exitosa
		}
        pthread_mutex_unlock(&lock);
	}
	close(sock);
}


int main (int argc, char *argv[]) {
	int s, t, mysocket, sock, next = 0,i,j;
	pthread_t tid[MAX_THREADS];
	pthread_attr_t attr;
	struct sockaddr_in destiny, server; 
	socklen_t socksize = sizeof(struct sockaddr_in);

    fp = fopen("log.txt", "a+");
	parse_arguments(argc, argv);	
	used_seats = 0;

    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(PORTNUM);   
    mysocket = socket(AF_INET, SOCK_STREAM, 0);
    bind(mysocket, (struct sockaddr *)&server, sizeof(struct sockaddr));
    listen(mysocket, 10);
 
    if (pthread_attr_init(&attr)) {
        	printf("ERROR: No se pudo inicializar atributos de hilos.\n");
        	exit(1);
    }

    for (i=0; i<R; i++) {
        for (j=0; j<C; j++) m[i][j] = '0';
    }

    do {
    	printf("Escuchando en el puerto %d\n", PORTNUM);
    	sock = accept(mysocket, (struct sockaddr *)&destiny, &socksize);
        log_(destiny, "Conectado.");

    	if (pthread_create (&tid[next++], NULL, &manage_request, (void *) &sock)) {
        	printf("ERROR: No se pudo crear un hilo.\n");
        	exit(1);
    	}
    } while(sock);
 
    close(mysocket);
    close(sock);
    pthread_mutex_destroy(&lock);
}