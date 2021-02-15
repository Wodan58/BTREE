/*
    module  : btree.h
    version : 1.2
    date    : 02/15/21
*/
#define T	3
#define MIN	(T - 1)
#define FULL	(2 * T - 1)
#define SIZE	(sizeof(node_t) - sizeof(int))

typedef struct node_t {
    unsigned char n;			/* number of keys */
    unsigned char key[FULL];		/* key space */
    unsigned char ptr[FULL + 1];	/* pointers */
    unsigned char filler[4];
    int offset;
} node_t;
