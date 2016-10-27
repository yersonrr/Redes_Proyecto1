#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>    
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

void parse_arguments(int argc, char *argv[], char **port, char **bit_d, char **bit_r) {
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
				*bit_d = optarg;
				break;
			case 'o':
				*bit_r = optarg;
				break;
			case '?':
				printf("Llamada ingresada no es valida.\n");
				printf("%s", bad_syntax_msg);
				exit(0);
		}
	}
}

int main(int argc, char *argv[]) {
	char buffer[55];
	char *token;
	char *port = NULL;
	char *op;
	int id_usuario,cantidad_retiros = 0;
	int usuarios[30000]; // Arreglo de id de usuarios que han retirado.
	char *bit_deposito;
	char *bit_retiro;
	FILE *fd_d, *fd_r;
	int sockfd, clifd, size, n, monto,tamano;
	int total = 80000;
	int cantidad_op = 0;
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
	    send(clifd,"Bienvenido al servicio de cajeros Banco Simon Bolivar.\n",55,0); 
   		bzero(buffer,55);
		n = recv(clifd,buffer,50,0); // Hay que hacer esto mas generico
		if (n < 0) {
			perror("ERROR al leer del socket");
			exit(0);
		}

		token = strtok(buffer," ");
		int contador = 0;
		while (token != NULL) {
			if (contador == 0){
				id_usuario = atoi(token);
			} else if (contador == 1) {
				op = token;
			} else if (contador == 2){
				monto = atoi(token);
			} else {
				printf("ESTO NO DEBERIA PASAR. \n");
				exit(0);
			}
			token = strtok(NULL," ");
			contador++;
		}

		int i = 0;
		while (i < cantidad_retiros) {
			if (usuarios[i] == id_usuario){
				printf("DEBERA ESTAR AQUI 3 VECES\n");
				cantidad_op++;
			}
			if (cantidad_op == 3) {
				break;
			}
			i++;
		}
		
		if (total < 5001 && !strcmp(op,"r")) {
			send(clifd,"4",55,0);
		} else if (monto > 3000 && !strcmp(op,"r")){
			send(clifd,"2",55,0);
		} else if (!strcmp(op,"d")) {
			fd_d = fopen(bit_deposito,"a");
			send(clifd,"0",55,0);
			n = recv(clifd,buffer,50,0); // Hay que hacer esto mas generico
			if (n < 0) {
				error("ERROR al leer del socket");
				exit(0);
			}
			total = total + monto;
			fprintf(fd_d, "Fecha Hora %s %d %d \n",op,monto,id_usuario);
			fclose(fd_d);
		} else if (total > 5000 && !strcmp(op,"r") && cantidad_op < 3){
			fd_r = fopen(bit_retiro,"a");
			send(clifd,"1",55,0);
			n = recv(clifd,buffer,50,0); // Hay que hacer esto mas generico
			if (n < 0) {
				error("ERROR al leer del socket");
				exit(0);
			}
			if (atoi(buffer) == id_usuario) {
				total = total - monto;
				usuarios[cantidad_retiros] = id_usuario;
				cantidad_retiros++;
				fprintf(fd_r,"Fecha Hora %s %d %d \n",op,monto,id_usuario);
				fclose(fd_r);
			}
		} else if (cantidad_op == 3) {
			send(clifd,"3",55,0);
		} else{
			printf("NO DEBERIA ENTRAR EN ESTE ELSE \n");
		}
		cantidad_op = 0;		
		printf("El total restante es de: %d \n",total);
		printf("%d \n",usuarios[cantidad_retiros-1]);
	}
	return 0;
}