/*
    module  : gtree.c
    version : 1.1
    date    : 04/14/20
*/
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    unsigned char i, mode;

    if (argc > 1)
	mode = *argv[1];
    for (i = 0; i < 50; i++)
	printf("%c %d\n", mode, (unsigned char)rand());
    if (mode == 's')
	printf("s 0\n");
    printf("q\n");
    return 0;
}
