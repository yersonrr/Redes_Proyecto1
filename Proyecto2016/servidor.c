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
				}
				break;
			case 'o':
				*bit_r = fopen(optarg,"a");
				if(!bit_r){
					printf("No tienes permiso para escribir en este archivo \n");
					exit(0);
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
	char buffer[55];
	char *token;
	char *port = NULL;
	char *op,*id_usuario,*usuarios;
	FILE *bit_deposito;
	FILE *bit_retiro;
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
		n = read(clifd,buffer,50); // Hay que hacer esto mas generico
		if (n < 0) {
			error("ERROR al leer del socket");
			exit(0);
		}
		usuarios = (char *) malloc(4*sizeof(char));

		token = strtok(buffer," ");
		int contador = 0;
		while (token != NULL) {
			if (contador == 0){
				id_usuario = token;
			} else if (contador == 1) {
				op = token;
			} else if (contador == 2){
				monto = atoi(token);
			} else {
				printf("ESTO NO DEBERIA PASAR. \n");
				exit(0);
			}
			token = strtok(NULL," ");
			contador = contador + 1;
		}

		int i = 0;
		tamano = sizeof(usuarios)/sizeof(usuarios[0]);
		// HAY QUE REVISAR ESTO
		while (i < tamano) {
			if (!strcmp(&(usuarios[i]),id_usuario)){
				printf("DEBERA ESTAR AQUI 3 VECES\n");
				cantidad_op = cantidad_op + 1;
			}
			i = i+1;
		}
		
		if (cantidad_op > 3 && !strcmp(op,"r")) {
			send(clifd,"3",55,0);
			exit(0);
		}
		
		if (total < 5001 && !strcmp(op,"r")) {
			send(clifd,"2",55,0);
		} else if (monto > 3000){
			send(clifd,"2",55,0);
		} else if (!strcmp(op,"d")) {
			total = total + monto;
			fprintf(bit_deposito, "Fecha Hora %s %d \n",op,id_usuario);
			send(clifd,"0",55,0);
		} else if (total > 5000 && !strcmp(op,"r")){
			total = total - monto;
			fprintf(bit_retiro, "Fecha Hora %s %d \n",op,id_usuario);
			send(clifd,"1",55,0);
		} else {
			printf("NO DEBERIA ENTRAR EN ESTE ELSE \n");
			exit(0);
		}
		cantidad_op = 0;
		tamano = sizeof(usuarios)/sizeof(usuarios[0]);
		usuarios = (char *) realloc(usuarios,sizeof(usuarios)+sizeof(usuarios[0]));
		strcpy(id_usuario, &(usuarios[tamano-1]));

		printf("El total restante es de: %d \n",total);
		printf("%d \n",usuarios[tamano-1]);
	}
   
	printf("%s \n","prueba chill de servidor");
	return 0;
}