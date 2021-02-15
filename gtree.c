/*
    module  : gtree.c
    version : 1.3
    date    : 02/15/21
*/
#include <stdio.h>
#include <stdlib.h>

int my_rand(void);

int main(int argc, char *argv[])
{
    int i;
    unsigned char mode = 'q';

    if (argc > 1)
	mode = *argv[1];
    my_rand();
    for (i = 0; i < 100; i++)
	printf("%c %d\n", mode, (unsigned char)my_rand());
    if (mode == 's')
	printf("s 0\n");
    printf("q\n");
    return 0;
}
