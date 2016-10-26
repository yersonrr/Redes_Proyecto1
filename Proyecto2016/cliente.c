#include <stdio.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAXHOSTNAME 1000

void parse_arguments(int argc, char *argv[], char **port, int *i, char **server, char **o) {
	char *bad_syntax_msg = "Sintaxis errada de argumentos del programa. Sintaxis correcta: bsd_cli -d <ip_servidor> -p <puerto_servidor> -c <op> -i <codigo_usuario>\n";


	if (argc != 9) {
		printf("%s", bad_syntax_msg);
		exit(1);
	}

	int opt = 0;
	char *id = NULL;
	char *s = NULL;
	char *option = NULL;
	char *p = NULL;

	while ((opt = getopt(argc, argv, "d:c:i:p:")) != -1) {

	    switch(opt) {
		    case 'i':
		    	id = optarg;
		    	//printf("Id value=%s\n", id);
		    	break;
		    case 'd':
		    	s = optarg;
		    	//printf("Direcction Server=%s\n", server);
		    	break;
	    	case 'c':
		    	option = optarg;
		    	//printf("Option=%s\n", option);
		    	break;
		    case 'p':
		    	p = optarg;
		    	//printf("Port=%s\n", port);
		    	break;
		    case '?':			    
			    if (optopt == 'i') {
			    	printf("Falta el valor -i para el codigo de usuario\n");
			    	printf("%s", bad_syntax_msg);
			  	} else if (optopt == 'd') {
			    	printf("Falta el valor -d para el IP del servidor\n");
			    	printf("%s", bad_syntax_msg);
			  	} else if (optopt == 'c') {
			    	printf("Falta el valor -c para el IP del servidor\n");
			    	printf("%s", bad_syntax_msg); 
			    } else if (optopt == 'p') {
			    	printf("Falta el valor -p para el puerto del servidor\n");
			    	printf("%s", bad_syntax_msg);
			    } else {
			    	printf("\nInvalid option received");
			  	}
		  	break;
		}
 	}

	if((strcmp(option,"d") != 0) && (strcmp(option,"r") != 0)){
		printf("Opcion de Deposito o Retiro invalido, use 'd' o 'r' para ello.\n");
		exit(0);
	} else {
		*o = option;
	}

	if(atoi(p) == 0){
		printf("El puerto solo debe contener numeros.\n");
		exit(0);
	} else {
		*port = p;
	}

	if(atoi(id) == 0){
		printf("El Id solo debe contener numeros.\n");
		exit(0);
	} else if(strlen(id) > 4){
		printf("Id no valido, el Id debe contener 4 numeros.\n");
		exit(0);
	} else {
		*i = atoi(id);
	}

	*server = s;

}


int main (int argc, char *argv[]) {
	char buffer[50];
    int sock, try, c, id, j, a, len, R, C;
    struct addrinfo hints, *servinfo;
    char *server = NULL;
    char *option = NULL;
    char *port = NULL;

	parse_arguments(argc, argv, &port, &id, &server, &option);
	printf("Server: %s\n", server);
	printf("Puerto: %s\n", port);
	printf("Id: %d\n", id);
	printf("Opcion: %s\n", option);

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(server, port, &hints, &servinfo) != 0) {
        printf("ERROR: no se pudo resolver el nombre del servidor.\n");
        return -1;
    }

    
 	while (1) {
 		sock = socket(AF_INET, SOCK_STREAM, 0);
 		if (connect(sock, servinfo->ai_addr, servinfo->ai_addrlen) != 0) {
 			printf("ERROR: no se pudo conectar con el servidor.\n");
        	return -1;
 		}

	 	sprintf(buffer, "%d %s", id, option);
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
	    		printf("Deposito hecho con exito.\n");
	    		close(sock);
	    		return 0;
	    	case '1':
	    		printf("Retiro hecho con exito.\n");
	    		close(sock);
	    		return 0;
	    	case '2':
	    		printf("No es posible establecer el retiro.\n");
	    		close(sock);
	    		return -1;
	    	case '3':
	    		printf("Ha retirado la cantidad maxima establecida por el cajero\n");
	    		close(sock);
	    		return -1;
	    	default:
	    		printf("El servidor esta presentando una falla. Recibido: %c.\n", buffer[0]);
	    		close(sock);
	    		return -1;
	    }
	}
	return 0;
}