/*
    module  : btree.c
    version : 1.10
    date    : 09/05/23
*/
#include <stdio.h>
#include <assert.h>
#include "btree.h"
#include "gc.h"

#define INVALID		(unsigned)-1

char *bottom_of_stack;	/* global variable */

node_t *btree_new_node(void)
{
    int i;
    node_t *x;

    x = GC_malloc_atomic(sizeof(node_t));
    assert(x);
    for (i = 0; i <= FULL; i++)
	x->ptr[i] = INVALID;
    x->offset = INVALID;
    return x;
}

node_t *btree_disk_read(FILE *fp, int ptr)
{
    int rv;
    node_t *x;
    size_t count;

    x = btree_new_node();
    rv = fseek(fp, x->offset = ptr, 0);
    assert(!rv);
    count = fread(x, SIZE, 1, fp);
    assert(count);
    assert(x->n <= FULL);
    return x;
}

void btree_disk_write(FILE *fp, node_t *x)
{
    int rv;

    if (!x->offset)
	rewind(fp);
    else if (x->offset != INVALID) {
	rv = fseek(fp, x->offset, 0);
	assert(!rv);
    } else {
	rv = fseek(fp, 0, SEEK_END);
	assert(!rv);
	x->offset = ftell(fp);
    }
    fwrite(x, SIZE, 1, fp);
}

int btree_locate(node_t *x, int k, int *pos)
{
    int i;

    for (i = 0; i < x->n && x->key[i] < k; i++)
	;
    *pos = i;
    return i < x->n && x->key[i] == k;
}

node_t *btree_search(FILE *fp, node_t *x, int k, int *pos)
{
    for (;;) {
	if (btree_locate(x, k, pos))
	    return x;
	if (x->ptr[0] == INVALID)
	    return 0;
	x = btree_disk_read(fp, x->ptr[*pos]);
    }
}

void btree_create(FILE *fp, node_t **root)
{
    *root = btree_new_node();
    fwrite(root, SIZE, 1, fp);
}

void btree_read_root(FILE *fp, node_t **root)
{
    size_t count;

    *root = btree_new_node();
    rewind(fp);
    count = fread(*root, SIZE, 1, fp);
    assert(count);
}

void Exit(FILE *fp, node_t *root)
{
    rewind(fp);
    fwrite(root, SIZE, 1, fp);
}

void Init(FILE **fp, node_t **root)
{
    if ((*fp = fopen("btree.dat", "rb+")) == 0) {
	*fp = fopen("btree.dat", "wb+");
	btree_create(*fp, root);
    } else
	btree_read_root(*fp, root);
    (*root)->offset = 0;
}

void btree_split_child(FILE *fp, node_t *x, int i, node_t *y)
{
    int j;
    node_t *z;

    z = btree_new_node();
    z->n = y->n = MIN;
    for (j = 0; j < MIN; j++)
	z->key[j] = y->key[j + T];
    if (y->ptr[0] != INVALID)
	for (j = 0; j < T; j++)
	    z->ptr[j] = y->ptr[j + T];
    x->ptr[x->n + 1] = x->ptr[x->n];
    for (j = x->n - 1; j >= i; j--) {
	x->key[j + 1] = x->key[j];
	x->ptr[j + 1] = x->ptr[j];
    }
    x->key[i] = y->key[MIN];
    x->n++;
    btree_disk_write(fp, y);
    x->ptr[i] = y->offset;
    btree_disk_write(fp, z);
    x->ptr[i + 1] = z->offset;
    btree_disk_write(fp, x);
}

int btree_insert_nonfull(FILE *fp, node_t *x, int k)
{
    int i, j;
    node_t *y;

    for (;;) {
	if (btree_locate(x, k, &i))
	    return 0;
	if (x->ptr[0] == INVALID) {
	    for (j = x->n++; j > i; j--)
		x->key[j] = x->key[j - 1];
	    x->key[i] = k;
	    btree_disk_write(fp, x);
	    return 1;
	} else {
	    y = btree_disk_read(fp, x->ptr[i]);
	    if (y->n == FULL) {
		btree_split_child(fp, x, i, y);
		if (x->key[i] < k)
		    y = btree_disk_read(fp, x->ptr[i + 1]);
	    }
	    x = y;
	}
    }
}

int btree_insert(FILE *fp, node_t **root, int k)
{
    node_t *old_root;

    if ((*root)->n == FULL) {
	old_root = *root;
	old_root->offset = INVALID;
	*root = btree_new_node();
	(*root)->offset = 0;
	btree_split_child(fp, *root, 0, old_root);
    }
    return btree_insert_nonfull(fp, *root, k);
}

void PrintTree(FILE *fp, node_t *x, int level)
{
    int i;

    if (x) {
	for (i = 0; i < level; i++)
	    printf("  ");
	for (i = 0; i < x->n; i++)
	    printf("%d ", x->key[i]);
	putchar('\n');
	for (i = 0; i <= x->n; i++)
	    if (x->ptr[i] != INVALID)
		PrintTree(fp, btree_disk_read(fp, x->ptr[i]), level + 1);
    }
}

void Insert(FILE *fp, node_t **root)
{
    int i;

    printf("insert: ");
    if (scanf("%d", &i) == 1) {
	printf("%d\n", (unsigned char)i);
	btree_insert(fp, root, i);
    }
    PrintTree(fp, *root, 0);
}

void Search(FILE *fp, node_t *root)
{
    int i, pos;

    printf("search: ");
    if (scanf("%d", &i) == 1) {
	printf("%d\n", (unsigned char)i);
	btree_search(fp, root, i, &pos);
    }
    PrintTree(fp, root, 0);
}

int btree_delete(FILE *fp, node_t **root, int k);

void Delete(FILE *fp, node_t **root)
{
    int i;

    printf("delete: ");
    if (scanf("%d", &i) == 1) {
	printf("%d\n", (unsigned char)i);
	btree_delete(fp, root, i);
    }
    PrintTree(fp, *root, 0);
}

int start_main(int argc, char *argv[])
{
    int ch;
    char rv;
    FILE *fp;
    node_t *root;

    if (argc > 1 && !freopen(argv[1], "r", stdin)) {
	fprintf(stderr, "failed to open the file '%s'.\n", argv[1]);
	return 0;
    }
    Init(&fp, &root);
    PrintTree(fp, root, 0);
    for (;;) {
	printf("I, D, S, Q: ");
	if (scanf("%c", &rv) == 1)
	    switch (rv) {
	    case 'i':
	    case 'I': Insert(fp, &root);
		      break;
	    case 'd':
	    case 'D': Delete(fp, &root);
		      break;
	    case 's':
	    case 'S': Search(fp, root);
		      break;
	    case 'q':
	    case 'Q': putchar('\n');
		      goto einde;
	    }
	while ((ch = getchar()) != '\n')
	    ;
    }
einde:
    Exit(fp, root);
    return 0;
}

int main(int argc, char *argv[])
{
    int (* volatile m)(int, char **) = start_main;

    setbuf(stdout, 0);
    bottom_of_stack = &argc;
    GC_INIT();
    return (*m)(argc, argv);
}

void btree_delete_from_leaf(FILE *fp, node_t *x, int i)
{
    for (--x->n; i < x->n; i++)
	x->key[i] = x->key[i + 1];
    btree_disk_write(fp, x);
}

void btree_copy_in_predecessor(FILE *fp, node_t *x, int pos)
{
    node_t *y;

    y = btree_disk_read(fp, x->ptr[pos]);
    while (y->ptr[y->n] != INVALID)
	y = btree_disk_read(fp, y->ptr[y->n]);
    x->key[pos] = y->key[y->n - 1];
    btree_disk_write(fp, x);
}

void btree_move_left(FILE *fp, node_t *x, int pos)
{
    int i;
    node_t *left, *right;

    left = btree_disk_read(fp, x->ptr[pos - 1]);
    right = btree_disk_read(fp, x->ptr[pos]);
    left->key[left->n] = x->key[pos - 1];
    left->ptr[++left->n] = right->ptr[0];
    x->key[pos - 1] = right->key[0];
    right->n--;
    for (i = 0; i < right->n; i++) {
	right->key[i] = right->key[i + 1];
	right->ptr[i] = right->ptr[i + 1];
    }
    right->ptr[right->n] = right->ptr[right->n + 1];
    btree_disk_write(fp, x);
    btree_disk_write(fp, left);
    btree_disk_write(fp, right);
}

void btree_move_right(FILE *fp, node_t *x, int pos)
{
    int i;
    node_t *left, *right;

    left = btree_disk_read(fp, x->ptr[pos]);
    right = btree_disk_read(fp, x->ptr[pos + 1]);
    right->ptr[right->n + 1] = right->ptr[right->n];
    for (i = right->n++; i > 0; i--) {
	right->key[i] = right->key[i - 1];
	right->ptr[i] = right->ptr[i - 1];
    }
    right->key[0] = x->key[pos];
    right->ptr[0] = left->ptr[left->n--];
    x->key[pos] = left->key[left->n];
    btree_disk_write(fp, x);
    btree_disk_write(fp, left);
    btree_disk_write(fp, right);
}

void btree_combine(FILE *fp, node_t *x, int pos)
{
    int i;
    node_t *left, *right;

    left = btree_disk_read(fp, x->ptr[pos - 1]);
    right = btree_disk_read(fp, x->ptr[pos]);
    left->key[left->n] = x->key[pos - 1];
    left->ptr[++left->n] = right->ptr[0];
    for (i = 0; i < right->n; i++) {
	left->key[left->n] = right->key[i];
	left->ptr[++left->n] = right->ptr[i + 1];
    }
    x->n--;
    for (i = pos - 1; i < x->n; i++) {
	x->key[i] = x->key[i + 1];
	x->ptr[i + 1] = x->ptr[i + 2];
    }
    btree_disk_write(fp, x);
    btree_disk_write(fp, left);
    right->n = 0;
    btree_disk_write(fp, right);
}

void btree_restore(FILE *fp, node_t *x, int pos)
{
    if (pos == x->n)
	if (btree_disk_read(fp, x->ptr[pos - 1])->n > MIN)
	    btree_move_right(fp, x, pos - 1);
	else
	    btree_combine(fp, x, pos);
    else if (!pos)
	if (btree_disk_read(fp, x->ptr[1])->n > MIN)
	    btree_move_left(fp, x, 1);
	else
	    btree_combine(fp, x, 1);
    else if (btree_disk_read(fp, x->ptr[pos - 1])->n > MIN)
	btree_move_right(fp, x, pos - 1);
    else if (btree_disk_read(fp, x->ptr[pos + 1])->n > MIN)
	btree_move_left(fp, x, pos + 1);
    else
	btree_combine(fp, x, pos);
}

int btree_deleting(FILE *fp, node_t *x, int k)
{
    int rv, pos;

    if ((rv = btree_locate(x, k, &pos)) == 1)
	if (x->ptr[pos] != INVALID) {
	    btree_copy_in_predecessor(fp, x, pos);
	    btree_deleting(fp, btree_disk_read(fp, x->ptr[pos]), x->key[pos]);
	} else
	    btree_delete_from_leaf(fp, x, pos);
    else if (x->ptr[pos] != INVALID)
	rv = btree_deleting(fp, btree_disk_read(fp, x->ptr[pos]), k);
    if (x->ptr[pos] != INVALID)
	if (btree_disk_read(fp, x->ptr[pos])->n < MIN)
	    btree_restore(fp, x, pos);
    return rv;
}

int btree_delete(FILE *fp, node_t **root, int k)
{
    int rv;
    node_t *new_root;

    rv = btree_deleting(fp, *root, k);
    if (*root && !(*root)->n && (*root)->ptr[0] != INVALID) {
	new_root = btree_disk_read(fp, (*root)->ptr[0]);
	k = new_root->n;
	new_root->n = 0;
	btree_disk_write(fp, new_root);
	new_root->n = k;
	*root = new_root;
	(*root)->offset = 0;
    }
    return rv;
}
