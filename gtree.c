/*
    module  : gtree.c
    version : 1.4
    date    : 05/26/23
*/
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    int i;
    unsigned char mode = 'q';

    if (argc > 1)
	mode = *argv[1];
    rand();
    for (i = 0; i < 100; i++)
	printf("%c %d\n", mode, (unsigned char)rand());
    if (mode == 's')
	printf("s 0\n");
    printf("q\n");
    return 0;
}
