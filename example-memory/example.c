/* ejemplo calloc */
#include <stdio.h>
#include <stdlib.h>

#define CANT_FRAME 4
#define TAMANIO_FRAME 4

int main ()
{
	int i;
	char* memoriaPrinc;
    i=4;
	memoriaPrinc =(char*) calloc (CANT_FRAME,sizeof(TAMANIO_FRAME));
	printf ("Los datos son: ");
	printf ("%d ",memoriaPrinc);
	printf ("%i ",memoriaPrinc[0]);
	printf ("%d ",memoriaPrinc[1]);
	printf ("%s ",memoriaPrinc[2]);
	printf ("%d ",memoriaPrinc[3]);

	free (memoriaPrinc);
	return 0;
}
