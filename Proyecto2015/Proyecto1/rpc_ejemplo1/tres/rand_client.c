#include <stdlib.h>
#include <stdio.h>
#include "rand.h"


int
main (int argc, char *argv[])
{
	char *host;

	CLIENT *clnt;
	void  *result_1;
	double  *result_2;
	char *get_next_random_1_arg;

	long semilla;
	int i, iters;

	if (argc != 4) {
		fprintf (stderr, "uso: %s servidor semilla iteraciones\n", argv[0]);
		exit (1);
	}

	host = argv[1];
	semilla = (long) atoi(argv[2]);
	iters = atoi(argv[3]);

	clnt = clnt_create (host, RAND_PROG, RAND_VERS, "udp");
	if (clnt == NULL) {
		clnt_pcreateerror (host);
		exit (1);
	}

	result_1 = initialize_random_1(&semilla, clnt);
	if (result_1 == (void *) NULL) {
		clnt_perror (clnt, "call failed");
		exit (2);
	}

	for (i=0; i<iters ;i++) {

		result_2 = get_next_random_1((void*)&get_next_random_1_arg, clnt);
		if (result_2 == (double *) NULL) {
			clnt_perror (clnt, "call failed");
			exit (3);
		} else
			printf("%d: %f\n", i, *result_2);
	}

	clnt_destroy(clnt);
	exit (0);
}
