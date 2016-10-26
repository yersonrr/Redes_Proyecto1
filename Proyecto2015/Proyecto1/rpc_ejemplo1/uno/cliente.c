#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "rand.h"

int main (int argc, char *argv[])
{
	int iters;		/* Cantidad de iteraciones */
	int i;
	long semilla;		/* Semilla a utilizar */

	if (argc != 3)
	{
		fprintf (stderr, "Uso: %s semilla iteraciones\n", argv[0]);
		exit (1);
	}

	semilla=(long) atoi (argv[1]);
	iters=atoi(argv[2]);
	initialize_random(semilla);
	for (i=0; i<iters; i++)
		printf("%d: %f\n", i, get_next_random());
	exit (0);
}
