#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "RBTree.h"

node *create_node(void *key, int size)
{
	node *x = (node *)malloc(sizeof(node));
	if (NULL == x){
		printf("malloc node error!\n");
		return NULL;
	}

	x->key = (void *)malloc(size);
	if (NULL == x->key){
		printf("malloc key error!\n");
		free(x);
		return NULL;
	}

	memcpy(x->key, key, size);
	x->parent = NULL;
	x->left = NULL;
	x->right = NULL;
	x->color = RED;

	return x;
}

int clear_node(node *x, void(*proc)(void *))
{
	x->parent = NULL;
	x->left = NULL;
	x->right = NULL;
	if (proc){
		proc(x->key);
	}
	free(x->key);
	return 0;
}

static node nil = { NULL, NULL, NULL, BLACK, NULL };
RBTree *create_tree(int size, int(*comp)(void *, void *))
{
	RBTree *tree = (RBTree *)malloc(sizeof(RBTree));
	if (NULL == tree){
		printf("malloc tree error!\n");
		return NULL;
	}

	tree->count = size;
	tree->nil = &nil;
	tree->root = tree->nil;
	tree->comp = comp;
	tree->create_node = create_node;
	tree->clear_node = clear_node;

	return tree;
}

int clear_tree(node *x, void(*proc)(void *))
{
	if (x == &nil)
		return 1;

	if (x->left != &nil)
		clear_tree(x->left, proc);

	if (x->right != &nil)
		clear_tree(x->right, proc);

	clear_node(x, proc);
	free(x);
	return 0;
}

static int inorder(node *x, void(*proc)(void *))
{
	if (x->left != &nil)
		inorder(x->left, proc);

	proc(x->key);

	if (x->right != &nil)
		inorder(x->right, proc);

	return 0;
}

int inorder_tree(RBTree *tree, void(*proc)(void *))
{
	if (tree->root == tree->nil)
		return 1;

	inorder(tree->root, proc);

	return 0;
}

node *rbSearch(RBTree *tree, void *key)
{
	int comp = 0;
	node *r = tree->root;
	while (r != tree->nil && (comp = tree->comp(r->key, key))){
		if (comp > 0)
			r = r->left;
		else
			r = r->right;
	}
	if (r == tree->nil)
		r = &nil;
	return r;
}

static node *_rbMini(node *n)
{
	while (n->left != &nil){
		n = n->left;
	}
	return n;
}

node *rbMini(RBTree *tree)
{
	if (tree->root == tree->nil)
		return NULL;
	return _rbMini(tree->root);
}

static node *_rbMax(node *n)
{
	while (n->right != &nil)
		n = n->right;
	return n;
}

node *rbMax(RBTree *tree)
{
	if (tree->root == tree->nil)
		return NULL;
	return _rbMax(tree->root);
}

static void rb_rotate_left(node *x, RBTree *tree)
{
	node *y = x->right;
	x->right = y->left;
	y->left->parent = x;
	y->parent = x->parent;
	if (x->parent == tree->nil)
		tree->root = y;
	else if (x == x->parent->left)
		x->parent->left = y;
	else
		x->parent->right = y;
	y->left = x;
	x->parent = y;
}

static void rb_rotate_right(node *y, RBTree *tree)
{
	node *x = y->left;
	y->left = x->right;
	x->right->parent = y;
	x->parent = y->parent;
	if (y->parent == tree->nil)
		tree->root = x;
	else if (y == y->parent->left)
		y->parent->left = x;
	else
		y->parent->right = x;
	x->right = y;
	y->parent = x;
}

static void insert_fixup(RBTree *tree, node *z)
{
	node *y;
	while (RED == z->parent->color){
		if (z->parent == z->parent->parent->left){
			y = z->parent->parent->right;
			if (RED == y->color){//z叔叔为红色
				//父节点和叔叔节点设为黑色，祖父节点设为红色，z指针指向祖父节点
				z->parent->color = BLACK;
				y->parent = BLACK;
				z->parent->parent->color = RED;
				z = z->parent->parent;
			}
			else{
				if (z == z->parent->right){//z的叔叔是黑色，且z是parent的右节点
					z = z->parent;
					rb_rotate_left(z, tree);//进行一次左旋转即可
				}
				//z的叔叔是黑色，且z是parent的右节点
				//z的父节点设为黑色，祖父节点设为红色，对祖父节点进行右旋转
				z->parent->color = BLACK;
				z->parent->parent->color = RED;
				rb_rotate_right(z->parent->parent, tree);
			}
		}
		else{//方法一样，交换左右
			y = z->parent->parent->left;
			if (RED == y->color){
				z->parent->color = BLACK;
				y->color = BLACK;
				z->parent->parent->color = RED;
				z = z->parent->parent;
			}
			else{
				if (z == z->parent->left){
					z = z->parent;
					rb_rotate_right(z, tree);
				}
				z->parent->color = BLACK;
				z->parent->parent->color = RED;
				rb_rotate_left(z->parent->parent, tree);
			}
		}
	}
	tree->root->color = BLACK;
}

void rbInsert(RBTree *tree, void *data)
{
	int(*comp)(void *, void *) = tree->comp;
	node *x = tree->root;
	node *y = tree->nil;
	node *z = create_node(data, tree->count);
	while (x != tree->nil){
		y = x;
		if (comp(z->key, x->key) < 0)
			x = x->left;
		else
			x = x->right;
	}
	z->parent = y;
	if (y == tree->nil)
		tree->root = z;
	else if (comp(z->key, y->key) < 0)
		y->left = z;
	else
		y->right = z;
	z->left = tree->nil;
	z->right = tree->nil;
	z->color = RED;
	insert_fixup(tree, z);
}

node *tree_successor(node *r)
{
	node *y;
	if (r->right != &nil)
		return rbMini(r->right);
	y = r->parent;
	while (y != &nil && r == y->right){
		r = y;
		y = y->parent;
	}
	return y;
}

static void delete_fixup(RBTree *tree, node *x)
{
	node *w;
	while (x != tree->root && x->color == BLACK){
		if (x == x->parent->left){
			w = x->parent->right;
			if (RED == w->color){//x的兄弟节点为红色
				//w设为黑色，x的父节点设为红色，然后左旋转，
				w->color = BLACK;
				x->parent->color = RED;
				rb_rotate_left(tree, x->parent);
				w = x->parent->right;
			}
			if (BLACK == w->left->color && BLACK == w->right->color){//兄弟节点为黑色，兄弟节点的左右子节点为红色
				//w的颜色设为红，x为x的父节点
				w->color = RED;
				x = x->parent;
			}
			else{
				if (w->right->color == BLACK){//兄弟节点的右子节点为黑色
					//w左子节点颜色设为黑，w颜色为红，对w右旋转
					w->left->color = BLACK;
					w->color = RED;
					rb_rotate_right(tree, w);
					w = x->parent->right;
				}
				//兄弟节点的左子节点为黑色
				//w颜色设为root的颜色，对x父节点左旋转
				w->color = x->parent->color;
				x->parent->color = BLACK;
				if (w != tree->nil)
					w->right->color = BLACK;
				rb_rotate_left(tree, x->parent);
				x = tree->root;
			}
		}
		else{
			w = x->parent->left;
			if (RED == w->color){
				w->color = BLACK;
				x->parent->color = RED;
				rb_rotate_right(tree, x->parent);
				w = x->parent->left;
			}
			if (BLACK == w->right->color && BLACK == w->left->color){
				w->color = RED;
				x = x->parent;
			}
			else{
				if (BLACK == w->left->color){
					w->right->color = BLACK;
					w->color = RED;
					rb_rotate_left(tree, w);
					w = x->parent->left;
				}
				w->color = x->parent->color;
				x->parent->color = BLACK;
				if (w != tree->nil)
					w->left->color = BLACK;
				rb_rotate_right(tree, x->parent);
				x = tree->root;
			}
		}
	}
	x->color = BLACK;
}

node *rbDelete(RBTree *tree, node *x)
{
	node *y, *z;
	if (x->left == tree->nil || x->right == tree->nil)
		y = x;
	else
		y = tree_successor(x);
	if (y->left != tree->nil)
		z = y->left;
	else
		z = y->right;
	z->parent = y->parent;
	if (y->parent == tree->nil)
		tree->root = z;
	else if (y == y->parent->left)
		y->parent->left = z;
	else
		y->parent->right = z;
	if (y != z)
		memcpy(x->key, y->key, tree->count);
	if (BLACK == y->color && x != tree->nil)
		delete_fixup(tree, z);
	return y;
}