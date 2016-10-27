#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>    
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

int cantidad_retiros = 0;
int usuarios[30000]; // Arreglo de id de usuarios que han retirado.
int total = 80000;
char *bit_retiro;
char *bit_deposito;

pthread_mutex_t semaforo;

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

void *conexion(void *cli_fd_ptr){
	int cantidad_op = 0, n, id_usuario, monto;
	char *op;
	FILE *fd_d, *fd_r;
	char buffer[55];
	char *token;
	int *cli_fd = (int *) cli_fd_ptr;

    send(*cli_fd,"Bienvenido al servicio de cajeros Banco Simon Bolivar.\n",55,0); 
  	bzero(buffer,55);
	n = recv(*cli_fd,buffer,50,0); // Hay que hacer esto mas generico
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
		}
		token = strtok(NULL," ");
		contador++;
	} 
	int i = 0;
	pthread_mutex_lock(&semaforo);

	while (i < cantidad_retiros) {
		if (usuarios[i] == id_usuario){
			cantidad_op++;
		}
		if (cantidad_op == 3) {
			break;
		}
		i++;
	}

	if (!strcmp(op,"d")) {
		fd_d = fopen(bit_deposito,"a");
		send(*cli_fd,"2",55,0);
		n = recv(*cli_fd,buffer,50,0); // Hay que hacer esto mas generico
		if (n < 0) {
			error("ERROR al leer del socket");
			exit(0);
		}
		total = total + monto;
		fprintf(fd_d, "Fecha Hora %s %d %d \n",op,monto,id_usuario);
		fclose(fd_d);
	} else if (total > 5000 && !strcmp(op,"r") && cantidad_op < 3){
		fd_r = fopen(bit_retiro,"a");
		send(*cli_fd,"3",55,0);
		n = recv(*cli_fd,buffer,50,0); // Hay que hacer esto mas generico
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
	} else if (monto > 3000 && !strcmp(op,"r")){
		send(*cli_fd,"4",55,0);
	} else if (cantidad_op == 3) {
		send(*cli_fd,"5",55,0);
	} else if (total < 5001 && !strcmp(op,"r")) {
		send(*cli_fd,"6",55,0);
	}
	cantidad_op = 0;		
	printf("El total restante es de: %d \n",total);
	pthread_mutex_unlock(&semaforo);
	close(*cli_fd);
}

int main(int argc, char *argv[]) {
	char *port = NULL;
	int sockfd, clifd[100], size;
	struct sockaddr_in servidor, cliente;
	pthread_t hilos[100];
	pthread_attr_t attr;
	int iterador_hilos = 0;

	parse_arguments(argc, argv, &port, &bit_deposito, &bit_retiro);

	if ((sockfd=socket(AF_INET, SOCK_STREAM, 0)) == -1 ) {  
    	perror("Error en socket()\n");
    	exit(0);
   }

   servidor.sin_family = AF_INET;         
   servidor.sin_port = htons(atoi(port));
   servidor.sin_addr.s_addr = INADDR_ANY;
   //bzero(&(servidor.sin_zero),strlen(buffer));

    if(bind(sockfd,(struct sockaddr*)&servidor, sizeof(struct sockaddr))==-1) {
    	perror("error en bind() \n");
    	exit(0);
	} 

	//Hay que ver si esto cuadra con lo que nos piden pero creo que si
	if(listen(sockfd,10) == -1) { 
    	perror("error en listen()\n");
    	exit(0);
    }

    if (pthread_attr_init(&attr)) {
    	perror("No se inicializaron los atributos de los hilos");
    	exit(0);
    }

    if (pthread_mutex_init(&semaforo,NULL) != 0) {
    	perror("Fallo el semaforo. \n");
    	exit(0);
    }

    size=sizeof(struct sockaddr_in);	
  	pthread_mutex_unlock(&semaforo);
    while(1) {
    	
      	if ((clifd[iterador_hilos] = accept(sockfd,(struct sockaddr *)&cliente, &size))==-1) {
      		perror("error en accept()\n");
     		exit(0);
      	}
  		printf("Se obtuvo una conexión desde %s\n", inet_ntoa(cliente.sin_addr));
  		if (pthread_create(&hilos[iterador_hilos], NULL, &conexion,&clifd[iterador_hilos])){
  			fprintf(stderr, "Error en la creacion del hilo.\n");
  			exit(0);
  		}
  		iterador_hilos++;
  		if (iterador_hilos == 100) {
  			iterador_hilos = 0;
  		}
	}
	pthread_mutex_destroy(&semaforo);
	return 0;
}