/*
    module  : btree.h
    version : 1.5
    date    : 05/26/23
*/
#define T	3
#define MIN	(T - 1)
#define FULL	(2 * T - 1)
#define SIZE	32

typedef struct node_t {
    unsigned char n;		/* number of keys */
    unsigned char key[FULL];	/* key space */
    unsigned ptr[FULL + 1];	/* pointers */
    unsigned char unused[2];
    unsigned offset;
} node_t;
