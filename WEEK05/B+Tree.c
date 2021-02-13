/**
 *	@file		Implementation of B-Plus-Tree
 *	@brief      B-Plus-Tree Ž��, ����, ���� ����
 *
 *	@date		2021-01-14
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>


#define ORDER			4								// �ּ� ���� 4�� ����
#define MAX_CHILD		ORDER							// �ִ� �ڽ� ����						
#define MAX_KEY			MAX_CHILD-1						// �ִ� Ű ����
#define MIN_KEY			(int)(ceil(ORDER/2.0))-1		// �ּ� Ű ����

 /**
  *	@struct	  Node
  *
  *	@brief	  ���÷��� Ʈ���� ��� ����ü
  *
  * @member	  leaf                     (�������� �ƴ��� �Ǵ�)
  * @member   key[Node_Keys + 1]       (Ű ��)
  * @member   data[MAX_KEY + 1]        (������ ��)
  * @member   keyNum                   (Ű ����)
  * @member   childNum   		       (�ڽ� ����)
  * @member   endLeaf				   (leaf node ���Ḯ��Ʈ�� ���� �Ǵ�)
  *
  * @link     child[Node_Childs + 1]   (�ڽ��� �ּҸ� ������ ������)
  * @link     next					   (leaf node ���Ḯ��Ʈ)
  */

struct Node {							    
	bool leaf;
	int key[MAX_KEY + 1];
	int data[MAX_KEY + 1];
	int keyNum;							
	struct Node* child[MAX_CHILD + 1];
	int childNum;		
	struct Node* next;
	bool endLeaf;		
};

struct Node* root; // ��Ʈ ������

/**
 *	@fn		searchNode
 *
 *	@brief                  Ʈ���� ���¿� ���� Ž����, ����Ž�� (�ð� ���⵵ O(logn))
 *
 *  @param  node            (��Ʈ ��� ���� ������Ʈ ���� Ž��)
 *	@param	val             (ã���� �ϴ� Ű ��)
 *
 *	@return	int             (TRUE or False�� �̿�)
 */

int searchNode(struct Node* node, int val) {		
	if (!node) { 									 // Ʈ���� ����ٸ�		
		printf("Empty tree!!\n");
		return 0;
	}
	struct Node* level = node;						 // root���� leaf���� Ž��	
	while (1) {
		int pos;
		for (pos = 0; pos < level->keyNum; pos++) {
			if (val == level->key[pos]) {			 // ã���� ���� 1(TRUE)	
				printf("%d exist!!\n", val);
				return 1;
			}
			else if (val < level->key[pos]) {
				break;
			}
		}
		if (level->leaf) break;
		level = level->child[pos];
	}
	printf("%d not exist!!\n", val);			     // leaf �����ͼ��� ��ã���� ���� 0 (FALSE)		
	return 0;
}

/**
 *	@fn		createNode
 *
 *	@brief				��Ʈ ��带 ����� ���� ó���� ���� �Ҵ����ִ� �Լ�
 *
 *	@param	val			(�־��� Ű ��)
 *  @param  info        (�־��� ������ ��)
 * 
 *	@return				(��Ʈ���� ������)
 */

struct Node* createNode(int val, int info) {
	struct Node* newNode;
	newNode = (struct Node*)malloc(sizeof(struct Node)); 
	newNode->leaf = false;
	newNode->key[0] = val;
	newNode->keyNum = 1;
	newNode->childNum = 0;
	newNode->endLeaf = false;
	return newNode;
}

/**
 *	@fn		splitNode
 *
 *	@brief	�����Ҷ� ��尡 �� á�ٸ� ( keyNum == MAX_KEY + 1), ��带 ���� ���ִ� �Լ�
 *
 *	@param	pos			(���� �־������ ��ġ)
 *  @param	node		(������ ���)
 *  @param	parent		(�θ� ���)
 *
 *	@return				(��Ʈ���� ������)
 */

struct Node* splitNode(int pos, struct Node* node, struct Node* parent) {
	int median = node->keyNum / 2;
	struct Node* child;

	child = (struct Node*)malloc(sizeof(struct Node));
	child->leaf = node->leaf;
	child->keyNum = 0;
	child->childNum = 0;
	child->endLeaf = node->endLeaf;						// ���� ��尡 ������ ���� ����� ��� ���ο� ��尡 ������ ������.
	child->next = node->next;							

	if (node->leaf) {									// ���� ���� ��忡���� �����̶��
		node->next = child;								// �� ���� ���� ��带 ��ũ �����ش�.
		node->endLeaf = false;							// ������尡 ���ҵȴٸ� ���� ���� ���̻� ������ ������ �� ����.

		int num_iter = node->keyNum;
		for (int i = median; i < num_iter; i++) {		// child right�� Ű ����ֱ�. ���� ���� child left�� �ȴ�.
			child->key[i - median] = node->key[i];
			child->data[i - median] = node->data[i];
			node->keyNum--;
			child->keyNum++;
		}
	}
	else {												// leaf�� �ƴ� ����  child�� �Ѱ���� ��
		int num_iter = node->keyNum;
		for (int i = median + 1; i < num_iter; i++) {  
			child->key[i - median - 1] = node->key[i];
			child->data[i - median - 1] = node->data[i];
			child->keyNum++;
			node->keyNum--;
		}
		num_iter = node->childNum;
		for (int i = median + 1; i < num_iter; i++) {
			child->child[i - median - 1] = node->child[i];
			child->childNum++;
			node->childNum--;
		}
	}

	if (node == root) {									// ���� ��尡 ��Ʈ���
		struct Node* new_parent;						// ���ο� aprent���� median Ű�� �־��ش�.
		new_parent = createNode(node->key[median], node->data[median]);
		if (!node->leaf) {								// ������ �ƴҶ��� ���� ��忡�� median�� �������.
			node->keyNum--;								// median �������Ƿ� -1
		}
		new_parent->child[0] = node;
		new_parent->child[1] = child;
		new_parent->childNum = 2;
		parent = new_parent;
		return parent;
	}
	else {												// ���� ��尡 ��Ʈ�� �ƴ϶��
		for (int i = parent->keyNum; i > pos; i--) {    // �θ� �� �ڸ��� �����.
			parent->key[i] = parent->key[i - 1];
			parent->data[i] = parent->data[i - 1];
			parent->child[i + 1] = parent->child[i];
		}
		parent->key[pos] = node->key[median];           // �θ� ��忡 key�� �߰��� �ش�.
		parent->data[pos] = node->data[median];         // �θ� ��忡 data�� �߰��� �ش�.
		parent->keyNum++;
		if (!node->leaf) {								// ������ �ƴ� ���� ���� ��忡�� median�� �������.
			node->keyNum--;								// median �������Ƿ� -1
		}
		parent->child[pos + 1] = child;					// �θ� ��忡 �� child�� �߰��� �ش�.
		parent->childNum += 1;
	}
	return node;
}


struct Node* insertNode(int parent_pos, int val, int info, struct Node* node, struct Node* parent) {
	int pos;												// pos�� ���� �� ������
	for (pos = 0; pos < node->keyNum; pos++) {
		if (val == node->key[pos]) {
			printf("Duplicates are not permitted!!\n");		// �ߺ��� Ű�� ����
			return node;
		}
		else if (val < node->key[pos]) {					// val�� �� ��ġ�� ã�´�.
			break;
		}
	}
	if (!node->leaf) {										// leaf�� �ƴ� ��쿡�� child�� ��������.
		node->child[pos] = insertNode(pos, val,info, node->child[pos], node);
		if (node->keyNum == MAX_KEY + 1) {					// �Ʒ� ���Կ� ���� Ű�� �� á�ٸ�, �� split ���ش�.
			node = splitNode(parent_pos, node, parent);
		}
	}		
	else {													// leaf���� �����Ѵ�.													
		for (int i = node->keyNum; i > pos; i--) {
			node->key[i] = node->key[i - 1];
		}
		node->key[pos] = val;
		node->data[pos] = info;
		node->keyNum++;
		if (node->keyNum == MAX_KEY + 1) {					// Ű�� �����ٸ� split ���ش�.
			node = splitNode(parent_pos, node, parent);
		}
	}

	return node;											// ��� ��ȯ
}

void insert(int val, int info) {
	if (!root) { 											// ��Ʈ�� ���ٸ� ��Ʈ�� �����.
		root = createNode(val, info);
		root->leaf = true;
		root->endLeaf = true;
		return;
	}

	root = insertNode(0, val, info, root, root);			// ��Ʈ�� �ִٸ� ��带 ã�� �����Ѵ�.
}



void borrowFromLeft(struct Node* node, int pos) {

	if (node->child[pos]->leaf) {												// leaf���� borrow�� �Ͼ�ٸ�
		int target = 0;															// ������ ��ġ�� ������ index
		int borrow = node->child[pos - 1]->keyNum - 1;							// ������ ��ġ�� index

		for (int i = 0; i < node->child[pos]->keyNum; i++) {					// ä�� ��ġ�� ��� �д�.
			node->child[pos]->key[i + 1] = node->child[pos]->key[i];
		}
		node->child[pos]->key[target] = node->child[pos - 1]->key[borrow];	    // ������ ������ ä���.
		node->child[pos]->keyNum++;

		node->child[pos - 1]->keyNum--;											// ������ ���� Ű ������ �����.

		int successor = findSuccessor(node->child[pos]);						// successor�� ���� ä���.
		node->key[pos - 1] = successor;
	}
	else {																		// ���ο��� borrow�� �Ͼ�ٸ�
		int target = 0;															// ������ ��ġ�� ������ index
		for (int i = 0; i < node->child[pos]->keyNum; i++) {
			node->child[pos]->key[i + 1] = node->child[pos]->key[i];
		}
		node->child[pos]->key[target] = node->key[pos - 1];						// �� ���� ���ش�.
		node->child[pos]->keyNum++;

		int borrow = node->child[pos - 1]->keyNum - 1;							// ������ ��ġ�� index
		node->key[pos - 1] = node->child[pos - 1]->key[borrow];					// �� ���� ä���.

		node->child[pos - 1]->keyNum--;											// ������ ���� Ű������ �����.

		if (node->child[pos - 1]->childNum > 0) {								// ������ ���� child�� �ִٸ�
			borrow = node->child[pos - 1]->childNum - 1;						// ������ ���� child�� �Ѱ��ش�.
			for (int i = node->child[pos]->childNum; i > 0; i--) {
				node->child[pos]->child[i] = node->child[pos]->child[i - 1];
			}
			node->child[pos]->child[0] = node->child[pos - 1]->child[borrow];
			node->child[pos]->childNum++;

			node->child[pos - 1]->childNum--;									// ������ ���� child ����
		}
	}

}
void borrowFromRight(struct Node* node, int pos) {
	if (node->child[pos]->leaf) {												// �������� borrow�� �Ͼ�ٸ�
		int target = node->child[pos]->keyNum;									// ������ ��ġ�� ������ index
		int borrow = 0;															// ������ ��ġ�� index

		node->child[pos]->key[target] = node->child[pos + 1]->key[borrow];	    // ������ ������ ä���.
		node->child[pos]->keyNum++;

		for (int i = 0; i < node->child[pos + 1]->keyNum - 1; i++) {			// ������ ���� Ű ����
			node->child[pos + 1]->key[i] = node->child[pos + 1]->key[i + 1];
		}
		node->child[pos + 1]->keyNum--;

		int successor = findSuccessor(node->child[pos + 1]);				    // successor�� ���� ä���.
		node->key[pos] = successor;
	}
	else {																		// ���ο��� borrow�� �Ͼ�ٸ�
		int target = node->child[pos]->keyNum;									// ������ ��ġ�� ������ index
		node->child[pos]->key[target] = node->key[pos];							// �� ���� ���ش�.
		node->child[pos]->keyNum++;

		int borrow = 0;															// ������ ��ġ�� index(�� ����)
		node->key[pos] = node->child[pos + 1]->key[borrow];						// �� ���� ä���.

		for (int i = 0; i < node->child[pos + 1]->keyNum - 1; i++) {			// ������ ���� Ű ����
			node->child[pos + 1]->key[i] = node->child[pos + 1]->key[i + 1];
		}
		node->child[pos + 1]->keyNum--;

		if (node->child[pos + 1]->childNum > 0) {
			target = node->child[pos]->childNum;								// ������ ���� child�� �Ѱ��ش�.
			node->child[pos]->child[target] = node->child[pos + 1]->child[borrow];
			node->child[pos]->childNum++;

			for (int i = 0; i < node->child[pos + 1]->childNum - 1; i++) {		// ������ ���� child ����
				node->child[pos + 1]->child[i] = node->child[pos + 1]->child[i + 1];
			}
			node->child[pos + 1]->childNum--;
		}
	}
}

void mergeNode(struct Node* node, int pos, int pos_left) {						// ��� Ű�� pos_left ���� �����ش�.
	if (node->child[pos]->leaf) {												// �������� merge�� �Ͼ�ٸ�
		int target = node->child[pos_left]->keyNum;

		for (int i = 0; i < node->child[pos]->keyNum; i++) {					// �������ڽ��� Ű�� ���� �ڽĿ� �־��ش�.
			node->child[pos_left]->key[target + i] = node->child[pos]->key[i];
			node->child[pos_left]->keyNum++;
		}

		node->child[pos_left]->next = node->child[pos]->next;
		node->child[pos_left]->endLeaf = node->child[pos]->endLeaf;

		for (int i = pos_left; i < node->keyNum - 1; i++) {						// �� ����� key ���� 
			node->key[i] = node->key[i + 1];
		}
		node->keyNum--;

		for (int i = pos; i < node->childNum - 1; i++) {						// �� ����� child ���� 
			node->child[i] = node->child[i + 1];
		}
		node->childNum--;

	}
	else {																			// ��忡�� merge�� �Ͼ�ٸ�
		int target = node->child[pos_left]->keyNum;									// ���� �ڽĿ� �� index

		node->child[pos_left]->key[target] = node->key[pos_left];					// �� ��尪�� �־��ش�.
		node->child[pos_left]->keyNum++;

		for (int i = 0; i < node->child[pos]->keyNum; i++) {						// �������ڽ��� Ű�� ���� �ڽĿ� �־��ش�.
			node->child[pos_left]->key[target + 1 + i] = node->child[pos]->key[i];
			node->child[pos_left]->keyNum++;
		}

		target = node->child[pos_left]->childNum;
		for (int i = 0; i < node->child[pos]->childNum; i++) {						// �������ڽ��� �ڽ��� ���� �ڽĿ� �־��ش�.
			node->child[pos_left]->child[target + i] = node->child[pos]->child[i];
			node->child[pos_left]->childNum++;
		}

		free(node->child[pos]);														// ������ �ڽ��� free ��Ų��.

		for (int i = pos_left; i < node->keyNum - 1; i++) {							// �� ����� key ����
			node->key[i] = node->key[i + 1];
		}
		node->keyNum--;

		for (int i = pos; i < node->childNum - 1; i++) {							// �� ����� child ���� 
			node->child[i] = node->child[i + 1];
		}
		node->childNum--;
	}
}

void adjustNode(struct Node* node, int pos) {
	if (pos == 0) {													// child �� ���� ���϶�, ������ ���������� ������ �� �ִ�.
		if (node->child[pos + 1]->keyNum > MIN_KEY) {
			borrowFromRight(node, pos);
		}
		else {
			mergeNode(node, pos + 1, pos);
		}
		return;
	}
	else {
		if (pos == node->keyNum) {									// child �� ������ ���϶�, ���� ���������� ������ �� �ִ�.
			if (node->child[pos - 1]->keyNum > MIN_KEY) {					
				borrowFromLeft(node, pos);
			}
			else {
				mergeNode(node, pos, pos - 1);
			}
			return;
		}
		else {														// �� �� ��Ȳ������, ���ʿ��� ������ �� �ִ�.
			if (node->child[pos - 1]->keyNum > MIN_KEY) {
				borrowFromLeft(node, pos);
			}
			else if (node->child[pos + 1]->keyNum > MIN_KEY) {
				borrowFromRight(node, pos);
			}
			else {
				mergeNode(node, pos, pos - 1);
			}
			return;
		}

	}
}

int findSuccessor(struct Node* node) {
	int successor;
	if (node->leaf) {
		return node->key[0];
	}
	return findSuccessor(node->child[0]);
}

void deleteInnerTree(struct Node* node, int pos) {
	int sucessor = findSuccessor(node->child[pos + 1]);
	node->key[pos] = sucessor;
}


int deleteValFromNode(int val, struct Node* node) {
	int pos;
	int flag = false;
	for (pos = 0; pos < node->keyNum; pos++) {						// �� ��忡�� val, Ȥ�� �� ��ġ�� ã�´�.
		if (val == node->key[pos]) {
			flag = true;
			break;
		}
		if (val < node->key[pos]) {
			break;
		}
	}
	if (flag) {
		if (node->leaf) {											// case#1 leaf���� ������ ��
			for (int i = pos; i < node->keyNum; i++) {
				node->key[i] = node->key[i + 1];
			}
			node->keyNum--;
		}
		else {
			flag = deleteValFromNode(val, node->child[pos + 1]);	// ���ο��� �߰����� ���� ���� child�� �Ѿ��.
			if (node->child[pos + 1]->keyNum < MIN_KEY) {			// ��ͷ� ������ �� �����ߴ� �ڽ��� ������ ���ڸ� �� 
				adjustNode(node, pos + 1);
				deleteValFromNode(val, node);
			}
			else {
				deleteInnerTree(node, pos);
			}
		}

		return flag;
	}
	else {
		if (node->leaf) {
			return flag;
		}
		else {
			flag = deleteValFromNode(val, node->child[pos]);

		}
	}
	if (node->child[pos]->keyNum < MIN_KEY) {						// ��ͷ� ������ �� �����ߴ� �ڽ��� ������ ���ڸ� �� 
		adjustNode(node, pos);
	}

	return flag;
}

void delete(struct Node* node, int val) {
	if (!node) { 													// Ʈ���� ����� ��
		printf("Empty tree!!\n");
		return;
	}
	int flag = deleteValFromNode(val, node);
	if (!flag) { 													// ���� �� ��尡 ���� ��
		printf("%d no exist in the tree!!\n", val);
		return;
	}

	if (node->keyNum == 0) {										// case#3 ������ ��ȭ�� ������
		node = node->child[0];
	}
	root = node;
}



void printTree(struct Node* node, int level) {				
	if (!node) { 													// Ʈ���� ����� ��
		printf("Empty tree!!\n");
		return;
	}
	printf("Level %d :   ", level);
	for (int i = 0; i < level - 1; i++) {
		printf("            ");
	}
	for (int i = 0; i < node->keyNum; i++) {
		printf("%d ", node->key[i]);
	}
	printf("\n");
	level++;
	for (int i = 0; i < node->childNum; i++) {
		printTree(node->child[i], level);
	}
}

void printLeaves(struct Node* node) {			
	if (!node) { 													// Ʈ���� ����� ��
		printf("Empty tree!!\n");
		return;
	}
	if (node->leaf) {
		for (int i = 0; i < node->keyNum; i++) {
			printf("%d %d", node->key[i],node->data[i]);
		}
		printf("| ");
		if (!node->endLeaf) {
			printLeaves(node->next);
		}
		else {
			printf("\n");
		}
	}
	else {
		printLeaves(node->child[0]);
	}
}


#define TEST { 10,1,3,63,82,6,31,8,2,16,11,77,96,14,92,21,47,23,24,78,26,97,15,4,30,69,37,38,76,90,17,87,53,44,45,46,9,41,54,43,22,84,58,39,65,28,42,59,99,70,71,72,29,74,73,68,13,60,79,80,81,85,83,64,94,86,66,88,93,40,91,62,25,20,36,95,19,52,55,100 }

int main()
{
	int arr[80] = TEST;
	// TEST 1 CASE
	insert(4, 4 * 1000);
	insert(1, 1 * 1000);
	insert(3, 3 * 1000);
	insert(2, 2 * 1000);
	delete(root, 4);
	delete(root, 2);
	delete(root, 3);

	printf("---- TEST1 ----\n");
	printTree(root, 1);
	printLeaves(root);

	// TEST 2 CASE
	for (int i = 2; i <= 100; i++) {
		insert(i, i * 1000);
	}
	for (int i = 50; i <= 70; i++) {
		delete(root, i);
	}

	printf("\n\n\n\n\n\n");
	printf("---- TEST2 ----");
	printTree(root, 1);
	printLeaves(root);

	// TEST3 CASE
	for (int i = 50; i <= 70; i++) {
		insert(i, i * 1000);
	}

	for (int i = 0; i < 80; i++) {
		delete(root, arr[i]);
	}

	printf("\n\n\n\n\n\n");
	printf("---- TEST3 ----");
	printTree(root, 1);
	printLeaves(root);

	printf("���α׷��� ���������� ���� �Ǿ���.");
	return 0;

}

