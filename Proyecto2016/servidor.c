#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>    
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

void parse_arguments(int argc, char *argv[], char **port, FILE **bit_d, FILE **bit_r) {
	char *bad_syntax_msg = "Sintaxis errada de argumentos del programa. Sintaxis correcta: bsd_svr -l <puerto_servidor> -i <bitacora_deposito> -o <bitacora_retiro> \n";
	int opt = 0;	

	if (argc != 7) {
		printf("Cantidad de argumentos ingresados invalida.\n");
		printf("%s", bad_syntax_msg);
		exit(0);
	}

	while ((opt = getopt(argc, argv, "l:i:o:")) != -1) {
		switch(opt){
			case 'l':
				if (atoi(optarg) == 0) {
					printf("El puerto a usar solo debe contener numeros.\n");
					exit(0);
				} else {
					*port = optarg;
					printf("Numero de puerto del servidor = %s \n",*port);
					break;
				}
			case 'i':
				*bit_d = fopen(optarg,"a");
				if (!bit_d) {
					printf("No tienes permiso para escribir en este archivo \n");
					exit(0);
				} else {
					fprintf(*bit_d,"Nombre de bitacora_deposito = %s \n");
				}
				break;
			case 'o':
				*bit_r = fopen(optarg,"a");
				if(!bit_r){
					printf("No tienes permiso para escribir en este archivo \n");
					exit(0);
				} else{
					fprintf(*bit_r,"Nombre de bitacora_retiro = %s \n");
				}
				break;
			case '?':
				printf("Llamada ingresada no es valida.\n");
				printf("%s", bad_syntax_msg);
				exit(0);
		}
	}
}

int main(int argc, char *argv[]) {
	char buffer[50];
	char *port = NULL;
	FILE *bit_deposito;
	FILE *bit_retiro;
	int sockfd, clifd, size, n;
	struct sockaddr_in servidor, cliente;

	parse_arguments(argc, argv, &port, &bit_deposito, &bit_retiro);

	if ((sockfd=socket(AF_INET, SOCK_STREAM, 0)) == -1 ) {  
    	perror("Error en socket()\n");
    	exit(0);
   }

   servidor.sin_family = AF_INET;         
   servidor.sin_port = htons(atoi(port));
   servidor.sin_addr.s_addr = INADDR_ANY;
   bzero(&(servidor.sin_zero),strlen(buffer));

    if(bind(sockfd,(struct sockaddr*)&servidor, sizeof(struct sockaddr))==-1) {
    	perror("error en bind() \n");
    	exit(0);
	} 

	//Hay que ver si esto cuadra con lo que nos piden pero creo que si
	if(listen(sockfd,5) == -1) { 
    	perror("error en listen()\n");
    	exit(0);
    }

    while(1) {
    	size=sizeof(struct sockaddr_in);	
      	if ((clifd = accept(sockfd,(struct sockaddr *)&cliente, &size))==-1) {
      		perror("error en accept()\n");
     		exit(0);
      	}
	    printf("Se obtuvo una conexión desde %s\n", inet_ntoa(cliente.sin_addr)); 
	    send(clifd,"Bienvenido a mi servidor.\n",25,0); 
   	
   		bzero(buffer,strlen(buffer));
		n = read(clifd,buffer,strlen(buffer));
		if (n < 0) {
			error("ERROR al leer del socket");
			exit(0);
		}
		printf("Here is the message: %s",buffer);
   }

	printf("%s \n","prueba chill de servidor");
	return 0;
}