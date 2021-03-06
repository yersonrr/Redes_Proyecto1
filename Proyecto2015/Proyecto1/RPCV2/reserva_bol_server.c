/*
 * This is sample code generated by rpcgen.
 * These are only templates and you can use them
 * as a guideline for developing your own functions.
 */

#include "reserva_bol.h"

int m[10][4];
int used_seats = 0,
    MAX_SEATS = 10*4,
    R = 10,
    C = 4;

char ** ask_for_seat_1_svc(int r, int c,  struct svc_req *rqstp) {
	static char * result;

	result = (char *) malloc(1000 * sizeof(char));
	char numbers[] = {'0','1','2','3','4','5','6','7','8','9','0'};
	int i,j;

	if (r < 1 || r > R || c < 1 || c > C) {
        strcpy(result, "3");
        return &result;
    }

    if (used_seats == MAX_SEATS)
    	strcpy(result, "0");
    else {
        if (m[r-1][c-1] != 0) {
        	sprintf(result, "1%c%c0000000000000000000000000000000000000000", numbers[R],numbers[C]);
            for (i=0; i<R; i++) {
                for (j=0; j<C; j++) {
                    (result+3)[i*C+j] = (m[i][j]) ? '1':'0';
                }
            }
            //printf("%s\n", result);
        }
        else {
            m[r-1][c-1] = 1;      // 1-index => 0-index
            used_seats++;
            strcpy(result, "2");
        }
    }

	return &result;
}
