#include "rand.h"

void initialize_random (long int semilla)
{
	srand48(semilla);
}

double get_netx_random (void)
{
	return (drand48(void));
}
