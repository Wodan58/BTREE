/*
    module  : btree.h
    version : 1.1
    date    : 04/06/20
*/
#define T	3
#define MIN	(T - 1)
#define FULL	(2 * T - 1)
#define SIZE	(sizeof(node_t) - sizeof(int))

typedef struct node_t {
    unsigned char n;			/* aantal keys */
    unsigned char key[FULL];		/* key space */
    unsigned char ptr[FULL + 1];	/* pointers */
    unsigned char filler[4];
    int offset;
} node_t;
