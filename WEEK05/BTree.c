/**
 *	@file		Implementation of B-Tree
 *	@brief      B-Tree Ž��, ����, ���� ����
 *
 *	@date		2021-01-11
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#define Node_Order			5										// �ּ� ���� 3���� ����
#define Node_Childs			Node_Order								// �ִ� �ڽ� ����
#define Node_Keys			Node_Childs-1							// �ִ� Ű ����
#define Num_Minimum_Keys	(int) (ceil(Node_Order/2.0)) - 1        // �ּ� Ű ����

#define TESTCASE1 1													// �׽�Ʈ���̽� 1
#define TESTCASE2 0													// �׽�Ʈ���̽� 2

 /**
  *	@struct	  BTreeNode
  *
  *	@brief	  ��Ʈ���� ��� ����ü
  *
  * @member	  leaf                     (�������� �ƴ��� �Ǵ�)
  * @member   key[Node_Keys + 1]       (Ű ������)
  * @member   num_key                  (Ű ����)
  * @member   num_child			       (�ڽ� ����)
  *
  * @link     child[Node_Childs + 1]   (�ڽ��� �ּҸ� ������ ������)
  */

struct BTreeNode {
	bool leaf;
	int key[Node_Keys + 1];
	int num_key;
	struct BTreeNode* child[Node_Childs + 1];
	int num_child;
};

struct BTreeNode* root;							// ��Ʈ ������

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

int searchNode(struct BTreeNode* node, int val) {			// ���� Ž�� ���� �غ���

	if (!node) { 											// Ʈ���� ����ٸ�
		printf("Empty tree!!\n");
		return 0;
	}
	struct BTreeNode* level = node;							// root���� leaf���� Ž��
	while (1) {
		int pos;
		for (pos = 0; pos < level->num_key; pos++) {
			if (val == level->key[pos]) {					// ã���� ���� 1 (TRUE)
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
	printf("%d not exist!!\n", val);					    // leaf �����ͼ��� ��ã���� ���� 0 (FALSE)
	return 0;
}



/**
 *	@fn		createNode
 *
 *	@brief				��Ʈ ��带 ����� ���� ó���� ���� �Ҵ����ִ� �Լ�
 *
 *	@param	val			(�־��� Ű ��)
 *
 *	@return				(��Ʈ���� ������)
 */

struct BTreeNode* createNode(int val) {
	struct BTreeNode* newNode;										  
	newNode = (struct BTreeNode*)malloc(sizeof(struct BTreeNode));    // BƮ�� ����ü��ŭ �����Ҵ�

	newNode->leaf = false;
	newNode->key[0] = val;
	newNode->num_key = 1;
	newNode->num_child = 0;
	return newNode;
}

/**
 *	@fn		splitNode
 *
 *	@brief	�����Ҷ� ��尡 �� á�ٸ� ( num_key == Node_keys + 1), ��带 ���� ���ִ� �Լ�
 *
 *	@param	pos			(���� �־������ ��ġ)
 *  @param	node		(������ ���)
 *  @param	parent		(�θ� ���)
 *
 *	@return				(��Ʈ���� ������)
 */

 //Split Node
struct BTreeNode* splitNode(int pos, struct BTreeNode* node, struct BTreeNode* parent) {
	int median;													 // �и��� ���� �߾Ӱ�
	if (Node_Order % 2 == 0) {									 // ������ ¦���� ��
		median = node->num_key / 2 - 0.5;
	}
	else {														 // ������ Ȧ���� ��
		median = node->num_key / 2;
	}
	struct BTreeNode* child;
	child = (struct BTreeNode*)malloc(sizeof(struct BTreeNode));
	child->leaf = node->leaf;									 
	child->num_key = 0;
	child->num_child = 0;
	if (!node->leaf) {											 // leaf�� �ƴѵ� ������ ���, child�� �Ѱ������
		int num_iter = node->num_child;							 // �̸� �� child ��带 ����� ���� ����� �ڽĵ��� �����Ų��.
		for (int i = median + 1; i < num_iter; i++) {
			child->child[i - median - 1] = node->child[i];
			child->num_child++;
			node->num_child--;
		}
	}
	int num_iter = node->num_key;
	for (int i = median + 1; i < num_iter; i++) {				 // child right�� Ű ����ֱ� ���� ���� child left�� �ȴ�
		child->key[i - median - 1] = node->key[i];
		child->num_key++;
		node->num_key--;
	}
	if (node == root) {											 // ���� ��尡 ��Ʈ���
		struct BTreeNode* new_parent;							 // ���ο� parent���� median Ű�� �־��ش�.
		new_parent = createNode(node->key[median]);
		node->num_key--;										 // median �������Ƿ� -1
		new_parent->child[0] = node;
		new_parent->child[1] = child;
		new_parent->num_child = 2;
		parent = new_parent;
		return parent;
	}
	else {														 // ���� ��尡 ��Ʈ�� �ƴ϶��
		for (int i = parent->num_key; i > pos; i--){
			parent->key[i] = parent->key[i - 1];
			parent->child[i + 1] = parent->child[i];			 
		}
		parent->key[pos] = node->key[median];					 // �θ� ��忡 key�� �߰����ش�.
		parent->num_key++;
		node->num_key--;										 // median �������Ƿ� -1
		parent->child[pos + 1] = child;							 // �θ� ��忡 �� child�� �߰����ش�.
		parent->num_child += 1;
	}
	return node;
}

/**
 *	@fn		insertNode
 *
 *	@brief	Ű ���� ��忡 �����ϴ� �Լ� (���)
 *
 *	@param	parent_pose					(�θ��� ��ġ)
 *  @param	val							(������ Ű ��)
 *  @param	node						(���� ���)
 *  @param	parent						(�θ� ���)
 *
 *	@return								(��Ʈ���� ������)
 */
struct BTreeNode* insertNode(int parent_pos, int val, struct BTreeNode* node, struct BTreeNode* parent) {

	int pos;																// pos�� ���� �� ������
	for (pos = 0; pos < node->num_key; pos++) {
		if (val == node->key[pos]) {
			printf("Duplicates are not permitted!!\n");						// �ߺ��� Ű�� ����
			return node;
		}
		else if (val < node->key[pos]) {									// val�� �� ��ġ�� ã�´�.
			break;
		}
	}

	if (!node->leaf) {														// leaf�� �ƴ� ��쿡�� child�� ��������.
		node->child[pos] = insertNode(pos, val, node->child[pos], node);	// ��ͷ� ���� parent_pos�� �ٲ�
		if (node->num_key == Node_Keys + 1) {								// �Ʒ� ���Կ� ���� Ű�� á�ٸ�, �� split�� ���ش�
			node = splitNode(parent_pos, node, parent);
		}
	}
	else {																	// leaf���� �����Ѵ�.
		for (int i = node->num_key; i > pos; i--) {
			node->key[i] = node->key[i - 1];
			node->child[i + 1] = node->child[i];							
		}
		node->key[pos] = val;
		node->num_key++;
		if (node->num_key == Node_Keys + 1) {								// Ű�� ��á�ٸ�? split�� ���ش�!
			node = splitNode(parent_pos, node, parent);
		}
	}

	return node;															// ��� ��ȯ
}


/**
 *	@fn		insert
 *	@brief			ó���� root�� ���ٸ� ������ְ�, root�� �ִٸ� ��带 ã�� �����ϴ� �Լ�
 *
 *	@param	val		(������ Ű��)
 *
 *	@return			(void)
 */

void insert(int val) {
	if (!root) { 											// root �� ���ٸ� root�� �����.
		root = createNode(val);
		root->leaf = true;
		return;
	}

	root = insertNode(0, val, root, root);					// root �� �ִٸ� ��带 ã�� �����Ѵ�.
}


/**
 *	@fn		findPredecessor
 *	@brief  predecessor�� ã�Ƽ� ��ȯ���ִ� �Լ� (��ͷ� ã��)
 *
 *	@param	node				(�ش� ���)
 *
 *	@return						(int)
 */

int findPredecessor(struct BTreeNode* node) {
	int predecessor;												// ������ ����
	if (node->leaf) {												// ������ ��� ���� ������ ���� ��ȯ
		return node->key[node->num_key - 1];
	}
	return findPredecessor(node->child[node->num_child - 1]);		// ������ �ƴ� ��� ��ͷ� �� (���� ����� ���� ������ �ڽ����� ��)
}

/**
 *	@fn		findSuccessor
 *	@brief  sucessor�� ã�Ƽ� ��ȯ���ִ� �Լ� (��ͷ� ã��)
 *
 	@param	node				(�ش� ���)
 *
 *	@return						(int)
 */

int findSuccessor(struct BTreeNode* node) {
	int successor;													// ������ ����
	if (node->leaf) {												// ������ ��� ���� ���� ���� ��ȯ
		return node->key[0];

		return findSuccessor(node->child[0]);
	}// ������ �ƴ� ��� ��ͷ� �� (���� ����� ���� ���� �ڽ����� ��)
}

/**
 *	@fn		inorderPredecessor
 *	@brief  �ش� ��忡�� inorderpredecessor�� ã�Ƽ� ��ȯ�Ѵ�.
 *
 *	@param	node				(�ش� ���)
 *  @param	pos				    (�ش� ��ġ)
 *
 *	@return						(int)
 */

int inorderPredecessor(struct BTreeNode* node, int pos) {
	//int predecessor = node->child[pos]->num_key - 1;
	int predecessor = findPredecessor(node->child[pos]);
	node->key[pos] = predecessor;
	return predecessor;
}


/**
 *	@fn		inorderSuccessor
 *	@brief  �ش� ��忡�� inordersuccessor�� ã�Ƽ� ��ȯ�Ѵ�.
 * 
 *	@param	node				(�ش� ���)
 *  @param	pos				    (�ش� ��ġ)
 *
 *	@return						(int)
 */

int inorderSuccessor(struct BTreeNode* node, int pos) {
	//int successor = 0;
	int successor = findSuccessor(node->child[pos + 1]);
	node->key[pos] = successor;
	return successor;
}



/**
 *	@fn		inorderMerge
 *	@brief
 *
 *	@param	node				(������ Ű��)
 *  @param	pos				    (������ Ű��)
 *
 *	@return						(void)
 */


int inorderMerge(struct BTreeNode* node, int pos) {
	int target = node->child[pos]->num_key;					// ��ĥ ��ġ�� ������ idx
	int send = node->key[pos];
	node->child[pos]->key[target] = node->key[pos];			// ���� child�� ������ �� �ٿ��ֱ�
	node->child[pos]->num_key++;

	for (int i = 0; i < node->child[pos + 1]->num_key; i++) {   // ������ child�� Ű ���ʿ� �ٿ��ֱ�
		node->child[pos]->key[target + 1 + i] = node->child[pos + 1]->key[i];
		node->child[pos]->num_key++;
	}
	for (int i = 0; i < node->child[pos + 1]->num_child; i++) { // ������ child�� child ���ʿ� �ٿ��ֱ�
		node->child[pos]->child[target + 1 + i] = node->child[pos + 1]->child[i];
		node->child[pos]->num_child++;
	}

	for (int i = pos; i < node->num_key; i++) {				// �� ��� Ű�� ����
		node->key[i] = node->key[i + 1];
		node->num_key--;
	}
	for (int i = pos + 1; i < node->num_child; i++) {			// �� ��� child�� ����
		node->child[i] = node->child[i + 1];
		node->num_child--;
	}
	return send;
}



/**
 *	@fn		deleteInnerTree
 *	@brief  ���� ��带 �����ϱ� ���� ���� �Լ��� ��Ƶ� �Լ�
 *
 *	@param	node				(�ش� ���)
 *  @param	pos				    (�ش� ��ġ)
 *
 *	@return						(void)
 */

void deleteInnerTree(struct BTreeNode* node, int pos) {
	int result_deletion = 0; 
	if (node->child[pos]->num_key >= node->child[pos + 1]->num_key) { // ���� �ڽ��� Ű ������ ũ�ų� ������
		if (node->child[pos]->num_key > Num_Minimum_Keys) {
			result_deletion = inorderPredecessor(node, pos);
			deleteValFromNode(result_deletion, node->child[pos]);

		}
		else {
			result_deletion = inorderMerge(node, pos);
			deleteValFromNode(result_deletion, node->child[pos]);
		}
	}
	else {															  // ������ �ڽ��� Ű ������ Ŭ ��
		if (node->child[pos + 1]->num_key > Num_Minimum_Keys) {
			result_deletion = inorderSuccessor(node, pos);
			deleteValFromNode(result_deletion, node->child[pos + 1]);
		}
		else {
			result_deletion = inorderMerge(node, pos);
			deleteValFromNode(result_deletion, node->child[pos]);
		}
	}
}

/**
 *	@fn		borrowFromLeft
 *	@brief  ���ʿ��� ��������
 *
 *	@param	node				(�ش� ���)
 *  @param	pos				    (�ش� ��ġ)
 *
 *	@return						(void)
 */

void borrowFromLeft(struct BTreeNode* node, int pos) {
	int target = 0;													 // ������ ��ġ�� ������ idx
	for (int i = 0; i < node->child[pos]->num_key; i++) {
		node->child[pos]->key[i + 1] = node->child[pos]->key[i];
	}
	node->child[pos]->key[target] = node->key[pos - 1];				 // �� ���� ���ش�.
	node->child[pos]->num_key++;

	int borrow = node->child[pos - 1]->num_key - 1;						 // ������ ��ġ�� idx
	node->key[pos - 1] = node->child[pos - 1]->key[borrow];			 // �� ���� ä���.

	node->child[pos - 1]->num_key--;								 // ������ ���� Ű������ �����.


	if (node->child[pos - 1]->num_child > 0) {						 // ������ ���� child�� �ִٸ�
		borrow = node->child[pos - 1]->num_child - 1;			     // ������ ���� child�� �Ѱ��ش�.
		for (int i = node->child[pos]->num_child; i > 0; i--) {
			node->child[pos]->child[i] = node->child[pos]->child[i - 1];
		}
		node->child[pos]->child[0] = node->child[pos - 1]->child[borrow];
		node->child[pos]->num_child++;

		node->child[pos - 1]->num_child--;							 // ������ ���� child ����
	}
}

/**
 *	@fn		borrowFromRight
 *	@brief  �����ʿ��� ��������
 *
 *	@param	node				(�ش� ���)
 *  @param	pos				    (�ش� ��ġ)
 *
 *	@return						(void)
 */

void borrowFromRight(struct BTreeNode* node, int pos) {
	int target = node->child[pos]->num_key;							 // ������ ��ġ�� ������ idx
	node->child[pos]->key[target] = node->key[pos];				     // �� ���� ���ش�.
	node->child[pos]->num_key++;

	int borrow = 0;												     // ������ ��ġ�� idx(�ǿ���)
	node->key[pos] = node->child[pos + 1]->key[borrow];				 // �� ���� ä���.

	for (int i = 0; i < node->child[pos + 1]->num_key - 1; i++) {	 // ������ ���� Ű ����
		node->child[pos + 1]->key[i] = node->child[pos + 1]->key[i + 1];
	}
	node->child[pos + 1]->num_key--;

	if (node->child[pos + 1]->num_child > 0) {
		target = node->child[pos]->num_child;							 // ������ ���� child�� �Ѱ��ش�.
		node->child[pos]->child[target] = node->child[pos + 1]->child[borrow];
		node->child[pos]->num_child++;

		for (int i = 0; i < node->child[pos + 1]->num_child - 1; i++) {	 // ������ ���� child ����
			node->child[pos + 1]->child[i] = node->child[pos + 1]->child[i + 1];
		}
		node->child[pos + 1]->num_child--;
	}
}

/**
 *	@fn		mergeNode
 *	@brief  �� ������ �� ��ġ�� �Լ�
 *
 *	@param	node				(�ش� ���)
 *  @param	pos				    (�ش� ��ġ)
 *  @param	mer_pos			(��ĥ ���� node position)
 *
 *	@return						(void)
 */

void mergeNode(struct BTreeNode* node, int pos, int mer_pos) {		// ��� Ű�� poe_left ���� �����ش�. TODO:: �� ������ ���ؿ��� �������� ���ư��°�?
	int target = node->child[mer_pos]->num_key;						// ���� �ڽĿ� �� idx

	node->child[mer_pos]->key[target] = node->key[mer_pos];			// �� ��尪�� �־��ش�.
	node->child[mer_pos]->num_key++;

	for (int i = 0; i < node->child[pos]->num_key; i++) {			// �������ڽ��� Ű�� ���� �ڽĿ� �־��ش�.
		node->child[mer_pos]->key[target + 1 + i] = node->child[pos]->key[i];
		node->child[mer_pos]->num_key++;
	}

	target = node->child[mer_pos]->num_child;
	for (int i = 0; i < node->child[pos]->num_child; i++) {			// �������ڽ��� Ű�� ���� �ڽĿ� �־��ش�.
		node->child[mer_pos]->child[target + i] = node->child[pos]->child[i];
		node->child[mer_pos]->num_child++;
	}

	free(node->child[pos]);											// ������ �ڽ��� free ��Ų��.

	for (int i = mer_pos; i < node->num_key - 1; i++) {				// �� ����� key ���� 
		node->key[i] = node->key[i + 1];
	}
	node->num_key--;

	for (int i = pos; i < node->num_child - 1; i++) {				// �� ����� child ���� 
		node->child[i] = node->child[i + 1];
	}
	node->num_child--;
}

/**
 *	@fn		adjustNode
 *	@brief  ��Ȳ�� �°� �������� ��ĥ�� �������ִ� �Լ�
 *
 *	@param	node				(�ش� ���)
 *  @param	pos				    (�ش� ��ġ)
 *
 *	@return						(void)
 */

void adjustNode(struct BTreeNode* node, int pos) {
	if (pos == 0) {														// child �� ���� ���϶�, ������ ���������� ������ �� �ִ�.
		if (node->child[pos + 1]->num_key > Num_Minimum_Keys) {
			borrowFromRight(node, pos);
		}
		else {
			mergeNode(node, pos + 1, pos);
		}
		return;
	}
	else {
		if (pos == node->num_key) {										// child �� ������ ���϶�, ���� ���������� ������ �� �ִ�.
			if (node->child[pos - 1]->num_key > Num_Minimum_Keys) {
				borrowFromLeft(node, pos);
			}
			else {
				mergeNode(node, pos, pos - 1);
			}
			return;
		}
		else {															// �� �� ��Ȳ������, ���ʿ��� ������ �� �ִ�.
			if (node->child[pos - 1]->num_key > Num_Minimum_Keys) {
				borrowFromLeft(node, pos);
			}
			else if (node->child[pos + 1]->num_key > Num_Minimum_Keys) {
				borrowFromRight(node, pos);
			}
			else {
				mergeNode(node, pos, pos - 1);
			}
			return;
		}

	}

}

/**
 *	@fn		deleteValFromNode
 *	@brief					������ ����� ���� ã�Ƽ� ���� (case #1 ������� ���� / case #2 ���γ�� ����), ��쿡 ���� �ٸ� �Լ��� ȣ��
 *
 *	@param	val				(������ Ű��)
 *  @param	node		    (���� ���)
 *
 *	@return						(int)
 */

int deleteValFromNode(int val, struct BTreeNode* node) {
	int pos;
	int flag = false;
	for (pos = 0; pos < node->num_key; pos++) {				 // ���� ��忡�� val(������ ��)�� ã�Ƽ� flag�� true���ְ� pos�� ã�ų�,
		if (val == node->key[pos]) {
			flag = true;
			break;
		}
		else if (val < node->key[pos]) {					 // ã�� ���ߴٸ� �ڽ����� �� ���� pos�� ã�´�
			break;
		}
	}

	if (flag) {
		if (node->leaf) {									 // case#1 leaf���� ������ ��
			for (int i = pos; i < node->num_key; i++) {
				node->key[i] = node->key[i + 1];
			}
			node->num_key--;
		}
		else {
			deleteInnerTree(node, pos);						 // case#2 inner node���� ������ ��
		}
		return flag;
	}
	else {
		if (node->leaf) {
			return flag;
		}
		else {
			flag = deleteValFromNode(val, node->child[pos]); // ������� ���� ���� ��尡 ���� ��尡 �ƴϸ鼭 ���� ������ ��带 ã�� ���� ����. 
															 // ( ��͸� ���� �ڽ� ���� ���� )
		}
	}

	if (node->child[pos]->num_key < Num_Minimum_Keys) {      // ��ͷ� ������ �� �����ߴ� �ڽ��� ������ ���ڸ� �� 
		adjustNode(node, pos);
	}

	return flag;
}

/**
 *	@fn		delete
 *	@brief					��Ʈ ���� �����ؼ� ������ Ű���� ã�Ƽ� �����ִ� �Լ�
 *
 *	@param	node			(���� ���, ��Ʈ���� ����)
 *  @param	val				(������ Ű��)
 *
 *	@return					(void)
 */

void delete(struct BTreeNode* node, int val) {
	if (!node) { 											// Ʈ���� ����ٸ�
		printf("Empty tree!!\n");
		return;
	}

	int flag = deleteValFromNode(val, node);

	if (!flag) { 											// ���� �� ��尡 ���ٸ� ���޼��� ���
		printf("%d no exist in the tree!!\n", val);
		return;
	}

	if (node->num_key == 0) {								// case#3, ������ ��ȭ�� ������ (���� ���������)
		node = node->child[0];
	}
	root = node;

}

/**
 *	@fn		printTree
 *	@brief				��� Ʈ���� ������ Ű ������ ����Ʈ�ϴ� �Լ� (���)
 *
 *	@param	node		(���� ���, ��Ʈ���� ����)
 * 	@param	level		(Ʈ���� ����)
 *
 *	@return				(void)
 */
void printTree(struct BTreeNode* node, int level) {
	if (!node) { 											 // Ʈ���� ��� �ִٸ�
		printf("Empty tree!!\n");
		return;
	}
	printf("Level %d :   ", level);
	for (int i = 0; i < level - 1; i++) {
		printf("            ");
	}
	for (int i = 0; i < node->num_key; i++) {
		printf("%d ", node->key[i]);
	}
	printf("\n");
	level++;
	for (int i = 0; i < node->num_child; i++) {
		printTree(node->child[i], level);
	}
}


/**
 *	@fn		main
 *	@brief  �����Լ�, �׽�Ʈ���̽� ����
 */

int main(void) {

	printf("Minimum key numbers are %d\n", Num_Minimum_Keys);
	// �׽�Ʈ���̽� ����
#if TESTCASE1

	insert(10);
	insert(20);
	insert(30);
	insert(40);
	insert(50);
	insert(60);
	insert(70);
	insert(80);
	insert(90);
	insert(100);
	insert(110);
	insert(120);
	insert(130);
	insert(140);
	insert(150);
	insert(160);
	insert(170);
	insert(180);
	insert(190);
	insert(200);
	insert(210);
	insert(220);
	insert(230);
	insert(240);
	insert(250);
	insert(260);
	insert(9);
	insert(39);
	insert(101);
	insert(102);
	insert(103);
	insert(104);
	insert(161);
	insert(191);
	insert(251);
	printTree(root, 1);
	printf("****************************************************\n");

	delete(root, 103);
	delete(root, 70);
	delete(root, 130);
	delete(root, 104);
	delete(root, 60);
	delete(root, 120);
	delete(root, 240);
	delete(root, 160);
	printTree(root, 1);
	//printf("****************************************************\n");

	delete(root, 180);
	delete(root, 250);
	delete(root, 20);
	delete(root, 80);
	delete(root, 102);
	delete(root, 50);
	
	delete(root, 90);


	//printTree(root, 1);
#endif

#if TESTCASE2
	for (int i = 1; i <= 300; i++) {
		insert(i);
	}

	printTree(root, 1);
	printf("****************************************************\n");

	for (int i = 150; i <= 294; i++) {
		delete(root, i);
	}
	printTree(root, 1);
	searchNode(root, 295);
	searchNode(root, 149);
	searchNode(root, 300);
	searchNode(root, 150);
	searchNode(root, 175);
#endif

	return 0;
}