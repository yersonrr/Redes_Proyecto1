/*
 * Please do not edit this file.
 * It was generated using rpcgen.
 */

#include <memory.h> /* for memset */
#include "reserva_bol.h"

/* Default timeout can be changed using clnt_control() */
static struct timeval TIMEOUT = { 25, 0 };

char **
ask_for_seat_1(int arg1, int arg2,  CLIENT *clnt)
{
	ask_for_seat_1_argument arg;
	static char *clnt_res;

	memset((char *)&clnt_res, 0, sizeof(clnt_res));
	arg.arg1 = arg1;
	arg.arg2 = arg2;
	if (clnt_call (clnt, ask_for_seat, (xdrproc_t) xdr_ask_for_seat_1_argument, (caddr_t) &arg,
		(xdrproc_t) xdr_wrapstring, (caddr_t) &clnt_res,
		TIMEOUT) != RPC_SUCCESS) {
		return (NULL);
	}
	return (&clnt_res);
}