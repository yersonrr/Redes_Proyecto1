/* Proyecto 1 - CI-4825
Integrantes:
Yerson Roa - carnet:
Douglas Torres - carnet: 11-11027

Programa concurrente que realiza las labores del servidor. */

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
#include <time.h>
#include <sys/time.h>
#include <signal.h>

int cantidad_retiros = 0;	// Contador sobre retiros totales realizados. 
int usuarios[30000];		// Arreglo de id de usuarios que han retirado.
int total = 80000;			// Cantidad de efectivo disponible.
char *bit_retiro;			// Apuntador al archivo de bitácora de retiros.
char *bit_deposito;			// Apuntador al archivo de bitácora de depósitos.
pthread_mutex_t semaforo;	// Evita conflicto en operaciones criticas.

/* Manejador de la señal SIGALRM que es originada al haber cambio de dia. */
void timer_handler (int signum) {	
	printf("Cambio de día. \n");
	cantidad_retiros = 0;
  	memset(usuarios, 0, 30000);
}

/*  Función encargada de procesar la llamada del programa y en caso de presentar
	algún error, es debidamente informado al usuario. A su vez, se inicializan 
	variables globales, como es el caso de los file descriptors de las bitacoras. 
*/
void parse_arguments(int argc, char *argv[], char **port, char **bit_d, char **bit_r) {
	char *bad_syntax_msg = "Sintaxis errada de argumentos del programa. Sintaxis correcta: bsd_svr -l <puerto_servidor> -i <bitacora_deposito> -o <bitacora_retiro> \n";
	int opt = 0;	

	if (argc != 7) {
		printf("Cantidad de argumentos ingresados inválida.\n");
		printf("%s", bad_syntax_msg);
		exit(0);
	}

	while ((opt = getopt(argc, argv, "l:i:o:")) != -1) {
		switch(opt){
			case 'l':
				if (atoi(optarg) == 0) {
					printf("El puerto a usar sólo debe contener numeros.\n");
					exit(0);
				} else {
					*port = optarg;
					printf("Número de puerto del servidor = %s \n",*port);
					break;
				}
			case 'i':
				*bit_d = optarg;
				break;
			case 'o':
				*bit_r = optarg;
				break;
			case '?':
				printf("Llamada ingresada no es válida.\n");
				printf("%s", bad_syntax_msg);
				exit(0);
		}
	}
}

/* 	Función que se encarga del manejo de información que se recibe como de la 
	que se envia en respuesta. Se procesa la solicitud del usuario y se escribe
	en los archivos que corresponda a cada caso.
*/
void *conexion(void *cli_fd_ptr){
	int cantidad_op = 0, n, id_usuario, monto;
	char *op;
	FILE *fd_d, *fd_r;
	char buffer[55];
	char *token;
	int *cli_fd = (int *) cli_fd_ptr;
	struct tm t;
	time_t tiempo;

    send(*cli_fd,"Bienvenido al servicio de cajeros Banco Simón Bolívar.\n",55,0); 
  	bzero(buffer,55);
	n = recv(*cli_fd,buffer,55,0); // Hay que hacer esto mas generico
	if (n < 0) {
		perror("ERROR al leer del socket.\n");
		exit(0);
	}

	token = strtok(buffer," ");
	int contador = 0;
	int error = 0;
	while (token != NULL) {
		if(atoi(token) == 9 ){
			error = 1;
			break;
		}
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

	// A partir de este punto hay varias operaciones claves, por lo que se usa
	// semaforo a fin de evitar conflictos en las operaciones.
	pthread_mutex_lock(&semaforo);
	int i = 0;
	while (i < cantidad_retiros) {
		if (usuarios[i] == id_usuario){
			cantidad_op++;
		}
		if (cantidad_op == 3) {
			break;
		}
		i++;
	}

	time(&tiempo);
	t = *localtime_r(&tiempo,&t);
	// Sección donde se verifica la operación solicitada. 
	if (error) {
		printf("El usuario cerró la conexión.\n");
	} else if (!strcmp(op,"d")) {
		fd_d = fopen(bit_deposito,"a");
		send(*cli_fd,"2",55,0);
		n = recv(*cli_fd,buffer,55,0); 
		if (n < 0) {
			perror("ERROR al leer del socket.\n");
			exit(0);
		}
		total = total + monto;
		fprintf(fd_r,"Fecha: %d/%d/%d Hora: %d:%d:%d deposito %d bs %d \n",t.tm_mday,
			t.tm_mon,t.tm_year,t.tm_hour,t.tm_min,t.tm_sec,monto,id_usuario);
		fclose(fd_d);
	} else if (total > 5000 && !strcmp(op,"r") && cantidad_op < 3){
		fd_r = fopen(bit_retiro,"a");
		send(*cli_fd,"3",55,0);
		n = recv(*cli_fd,buffer,55,0);
		if (n < 0) {
			perror("ERROR al leer del socket.\n");
			exit(0);
		}
		if (atoi(buffer) == id_usuario) {
			total = total - monto;
			usuarios[cantidad_retiros] = id_usuario;
			cantidad_retiros++;
			fprintf(fd_r,"Fecha: %d/%d/%d Hora: %d:%d:%d retiro %d bs %d \n",t.tm_mday,
				t.tm_mon,t.tm_year,t.tm_hour,t.tm_min,t.tm_sec,monto,id_usuario);
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

/*  Ciclo principal del programa, donde se crea el socket del servidor, y donde
	se crean hilos, donde cada uno corresponde a una conexión que realiza un 
	cliente.
*/
int main(int argc, char *argv[]) {
	char *port = NULL;
	int sockfd, size; 
	int iterador_hilos = 0;
	int clifd[100]; 		// Arreglo donde se almacenan los sockects del cliente creados.
	pthread_t hilos[100];	// Arreglo donde se almacenan los hilos creados.
	pthread_attr_t attr;
	struct sockaddr_in servidor, cliente;
 	struct itimerval cronometro; // temporizador

	parse_arguments(argc, argv, &port, &bit_deposito, &bit_retiro);
    // Se asigna el manejador de señal originada por el cambio de dia.
    signal(SIGALRM, timer_handler);

	// Inicialización del socket y de la estructura del servidor.
	if ((sockfd=socket(AF_INET, SOCK_STREAM, 0)) == -1 ) {  
    	perror("ERROR en la creación del socket()\n");
    	exit(0);
   }
   servidor.sin_family = AF_INET;         
   servidor.sin_port = htons(atoi(port));
   servidor.sin_addr.s_addr = INADDR_ANY;
   bzero(&(servidor.sin_zero),55);

    if(bind(sockfd,(struct sockaddr*)&servidor, sizeof(struct sockaddr))==-1) {
    	perror("ERROR en llamada bind().\n");
    	exit(0);
	} 
	if(listen(sockfd,100) == -1) { 
    	perror("ERROR en llamada listen().\n");
    	exit(0);
    }
    if (pthread_attr_init(&attr)) {
    	perror("ERROR en la inicializaron de los atributos de los hilos.\n");
    	exit(0);
    }
    if (pthread_mutex_init(&semaforo,NULL) != 0) {
    	perror("ERROR en inicialización del semáforo.\n");
    	exit(0);
    }

    size=sizeof(struct sockaddr_in);	
  	pthread_mutex_unlock(&semaforo);
    
	/* Se configura el cronómetro de "cambio de día" a 60 segundos, esto para 
		efectos de que sea fácil de comprobar dicho cambio. */
	cronometro.it_value.tv_sec = 60;
	cronometro.it_value.tv_usec = 0;
	cronometro.it_interval.tv_sec = 60; //Segundos en reinicializar el temporizador.
	cronometro.it_interval.tv_usec = 0;
	// Inicio del cronometro.
	setitimer(ITIMER_REAL, &cronometro, NULL);

  	// Ciclo donde el servidor esperará conexiones de usuarios.
    while(1) {
      	if ((clifd[iterador_hilos] = accept(sockfd,(struct sockaddr *)&cliente, &size))==-1) {
      		perror("ERROR en el accept() de la conexión\n");
     		exit(0);
      	}
  		printf("Se obtuvo una conexión desde %s\n", inet_ntoa(cliente.sin_addr));
  		if (pthread_create(&hilos[iterador_hilos], NULL, &conexion,&clifd[iterador_hilos])){
  			fprintf(stderr, "ERROR en la creacion de hilos.\n");
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