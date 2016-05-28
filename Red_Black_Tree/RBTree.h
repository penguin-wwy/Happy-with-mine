
typedef enum {RED, BLACK} Color;
typedef struct RBNode
{
	struct RBNode *parent;
	struct RBNode *left;
	struct RBNode *right;
	Color color;
	void *key;
}node;

node *create_node(void *key, int size);			//�����ڵ�
int clear_node(node *, void(*proc)(void *));	//����ڵ�

typedef struct
{
	node *root;
	node *nil;
	unsigned count;

	int(*comp)(void *, void *);		//�ڵ����ݱȽϹ���
	node *(*create_node)(void *, int);
	int(*clear_node)(node *x, void(*proc)(void *));
}RBTree;

RBTree *create_tree(int size, int(*comp)(void *, void *));
int clear_tree(RBTree *tree, void(*proc)(void *));
int inorder_tree(RBTree *tree, void(*proc)(void *));
RBTree *rbMini(RBTree *tree);
RBTree *rbMax(RBTree *tree);
node *rbSearch(RBTree *tree, void *key);
void rbInsert(RBTree *tree, void *key);
node *rbDelete(RBTree *tree, node *x);