/*
    module  : btree.c
    version : 1.4
    date    : 05/29/20
*/
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "btree.h"

FILE *fp;
node_t *root;

node_t *btree_new_node(void)
{
    node_t *x;

    x = calloc(sizeof(node_t), 1);
    assert(x);
    return x;
}

node_t *btree_disk_read(int ptr)
{
    node_t *x;

    x = btree_new_node();
    fseek(fp, (x->offset = ptr) * SIZE, 0);
    fread(x, SIZE, 1, fp);
    return x;
}

void btree_disk_write(node_t *x)
{
    if (x == root)
	rewind(fp);
    else if (x->offset)
	fseek(fp, x->offset * SIZE, 0);
    else {
	fseek(fp, 0, SEEK_END);
	x->offset = ftell(fp) / SIZE;
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

node_t *btree_search(node_t *x, int k, int *pos)
{
    for (;;) {
	if (btree_locate(x, k, pos))
	    return x;
	if (!x->ptr[0])
	    return 0;
	x = btree_disk_read(x->ptr[*pos]);
    }
}

void btree_create(void)
{
    root = btree_new_node();
    fwrite(root, SIZE, 1, fp);
}

void btree_read_root(void)
{
    root = btree_new_node();
    fread(root, SIZE, 1, fp);
}

void btree_write_root(void)
{
    rewind(fp);
    fwrite(root, SIZE, 1, fp);
}

void Init(void)
{
    atexit(btree_write_root);
    if ((fp = fopen("btree.dat", "rb+")) == 0) {
	 fp = fopen("btree.dat", "wb+");
	btree_create();
    } else
	btree_read_root();
}

void btree_split_child(node_t *x, int i, node_t *y)
{
    int j;
    node_t *z;

    z = btree_new_node();
    z->n = y->n = MIN;
    for (j = 0; j < MIN; j++)
	z->key[j] = y->key[j + T];
    if (y->ptr[0])
	for (j = 0; j < T; j++)
	    z->ptr[j] = y->ptr[j + T];
    x->ptr[x->n + 1] = x->ptr[x->n];
    for (j = x->n - 1; j >= i; j--) {
	x->key[j + 1] = x->key[j];
	x->ptr[j + 1] = x->ptr[j];
    }
    x->key[i] = y->key[MIN];
    x->n++;
    btree_disk_write(y);
    x->ptr[i] = y->offset;
    btree_disk_write(z);
    x->ptr[i + 1] = z->offset;
    btree_disk_write(x);
}

int btree_insert_nonfull(node_t *x, int k)
{
    int i, j;
    node_t *y;

    for (;;) {
	if (btree_locate(x, k, &i))
	    return 0;
	if (!x->ptr[0]) {
	    for (j = x->n++; j > i; j--)
		x->key[j] = x->key[j - 1];
	    x->key[i] = k;
	    btree_disk_write(x);
	    return 1;
	} else {
	    y = btree_disk_read(x->ptr[i]);
	    if (y->n == FULL) {
		btree_split_child(x, i, y);
		if (x->key[i] < k)
		    y = btree_disk_read(x->ptr[i + 1]);
	    }
	    x = y;
	}
    }
}

int btree_insert(int k)
{
    node_t *new_root, *old_root;

    if (root->n == FULL) {
	new_root = btree_new_node();
	old_root = root;
	btree_split_child(root = new_root, 0, old_root);
    }
    return btree_insert_nonfull(root, k);
}

void PrintTree(node_t *x, int level)
{
    int i;

    if (x) {
	for (i = 0; i < level; i++)
	    printf("  ");
	for (i = 0; i < x->n; i++)
	    printf("%d ", x->key[i]);
	putchar('\n');
	for (i = 0; i <= x->n; i++)
	    if (x->ptr[i])
		PrintTree(btree_disk_read(x->ptr[i]), level + 1);
    }
}

void Insert(void)
{
    int i;

    printf("insert: ");
    scanf("%d", &i);
    printf("%d\n", (unsigned char)i);
    btree_insert(i);
    PrintTree(root, 0);
}

void Search(void)
{
    int i, pos;

    printf("search: ");
    scanf("%d", &i);
    printf("%d\n", (unsigned char)i);
    btree_search(root, i, &pos);
    PrintTree(root, 0);
}

int btree_delete(int k);

void Delete(void)
{
    int i;

    printf("delete: ");
    scanf("%d", &i);
    printf("%d\n", (unsigned char)i);
    btree_delete(i);
    PrintTree(root, 0);
}

int main()
{
    char rv, ch;

    printf("node: %d\n", (int)sizeof(node_t));
    Init();
    for (;;) {
	printf("I, D, S, Q: ");
	scanf("%c", &rv);
	switch (rv) {
	case 'i':
	case 'I': Insert();
		  break;
	case 'd':
	case 'D': Delete();
		  break;
	case 's':
	case 'S': Search();
		  break;
	case 'q':
	case 'Q': putchar('\n');
		  return 0;
	}
	while ((ch = getchar()) != '\n')
	    ;
    }
    return 0;
}

void btree_delete_from_leaf(node_t *x, int i)
{
    for (--x->n; i < x->n; i++)
	x->key[i] = x->key[i + 1];
    btree_disk_write(x);
}

void btree_copy_in_predecessor(node_t *x, int pos)
{
    node_t *y;

    for (y = btree_disk_read(x->ptr[pos]); y->ptr[y->n];
	 y = btree_disk_read(y->ptr[y->n]))
	;
    x->key[pos] = y->key[y->n - 1];
    btree_disk_write(x);
}

void btree_move_left(node_t *x, int pos)
{
    int i;
    node_t *left, *right;

    left = btree_disk_read(x->ptr[pos - 1]);
    right = btree_disk_read(x->ptr[pos]);
    left->key[left->n] = x->key[pos - 1];
    left->ptr[++left->n] = right->ptr[0];
    x->key[pos - 1] = right->key[0];
    right->n--;
    for (i = 0; i < right->n; i++) {
	right->key[i] = right->key[i + 1];
	right->ptr[i] = right->ptr[i + 1];
    }
    right->ptr[right->n] = right->ptr[right->n + 1];
    btree_disk_write(x);
    btree_disk_write(left);
    btree_disk_write(right);
}

void btree_move_right(node_t *x, int pos)
{
    int i;
    node_t *left, *right;

    left = btree_disk_read(x->ptr[pos]);
    right = btree_disk_read(x->ptr[pos + 1]);
    right->ptr[right->n + 1] = right->ptr[right->n];
    for (i = right->n++; i > 0; i--) {
	right->key[i] = right->key[i - 1];
	right->ptr[i] = right->ptr[i - 1];
    }
    right->key[0] = x->key[pos];
    right->ptr[0] = left->ptr[left->n--];
    x->key[pos] = left->key[left->n];
    btree_disk_write(x);
    btree_disk_write(left);
    btree_disk_write(right);
}

void btree_combine(node_t *x, int pos)
{
    int i;
    node_t *left, *right;

    left = btree_disk_read(x->ptr[pos - 1]);
    right = btree_disk_read(x->ptr[pos]);
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
    btree_disk_write(x);
    btree_disk_write(left);
    right->n = 0;
    btree_disk_write(right);
}

void btree_restore(node_t *x, int pos)
{
    if (pos == x->n)
	if (btree_disk_read(x->ptr[pos - 1])->n > MIN)
	    btree_move_right(x, pos - 1);
	else
	    btree_combine(x, pos);
    else if (!pos)
	if (btree_disk_read(x->ptr[1])->n > MIN)
	    btree_move_left(x, 1);
	else
	    btree_combine(x, 1);
    else if (btree_disk_read(x->ptr[pos - 1])->n > MIN)
	btree_move_right(x, pos - 1);
    else if (btree_disk_read(x->ptr[pos + 1])->n > MIN)
	btree_move_left(x, pos + 1);
    else
	btree_combine(x, pos);
}

int btree_deleting(node_t *x, int k)
{
    int rv, pos;

    if ((rv = btree_locate(x, k, &pos)) == 1)
	if (x->ptr[pos]) {
	    btree_copy_in_predecessor(x, pos);
	    btree_deleting(btree_disk_read(x->ptr[pos]), x->key[pos]);
	} else
	    btree_delete_from_leaf(x, pos);
    else if (x->ptr[pos])
	rv = btree_deleting(btree_disk_read(x->ptr[pos]), k);
    if (x->ptr[pos])
	if (btree_disk_read(x->ptr[pos])->n < MIN)
	    btree_restore(x, pos);
    return rv;
}

int btree_delete(int k)
{
    int rv;
    node_t *new_root;

    rv = btree_deleting(root, k);
    if (root && !root->n) {
	new_root = btree_disk_read(root->ptr[0]);
	k = new_root->n;
	new_root->n = 0;
	btree_disk_write(new_root);
	root = new_root;
	root->n = k;
    }
    return rv;
}
