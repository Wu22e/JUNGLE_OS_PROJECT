/**
 *	@file		Implementation of B-Plus-Tree
 *	@brief      B-Plus-Tree 탐색, 삽입, 삭제 구현
 *
 *	@date		2021-01-14
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>


#define ORDER			4								// 최소 차수 4로 구현
#define MAX_CHILD		ORDER							// 최대 자식 갯수						
#define MAX_KEY			MAX_CHILD-1						// 최대 키 갯수
#define MIN_KEY			(int)(ceil(ORDER/2.0))-1		// 최소 키 갯수

 /**
  *	@struct	  Node
  *
  *	@brief	  비플러스 트리의 노드 구조체
  *
  * @member	  leaf                     (리프인지 아닌지 판단)
  * @member   key[Node_Keys + 1]       (키 값)
  * @member   data[MAX_KEY + 1]        (데이터 값)
  * @member   keyNum                   (키 갯수)
  * @member   childNum   		       (자식 개수)
  * @member   endLeaf				   (leaf node 연결리스트의 끝을 판단)
  *
  * @link     child[Node_Childs + 1]   (자식의 주소를 저장할 포인터)
  * @link     next					   (leaf node 연결리스트)
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

struct Node* root; // 루트 포인터

/**
 *	@fn		searchNode
 *
 *	@brief                  트리의 형태에 따라 탐색함, 이진탐색 (시간 복잡도 O(logn))
 *
 *  @param  node            (루트 노드 부터 리프노트 까지 탐색)
 *	@param	val             (찾고자 하는 키 값)
 *
 *	@return	int             (TRUE or False로 이용)
 */

int searchNode(struct Node* node, int val) {		
	if (!node) { 									 // 트리가 비었다면		
		printf("Empty tree!!\n");
		return 0;
	}
	struct Node* level = node;						 // root부터 leaf까지 탐색	
	while (1) {
		int pos;
		for (pos = 0; pos < level->keyNum; pos++) {
			if (val == level->key[pos]) {			 // 찾으면 리턴 1(TRUE)	
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
	printf("%d not exist!!\n", val);			     // leaf 까지와서도 못찾으면 리턴 0 (FALSE)		
	return 0;
}

/**
 *	@fn		createNode
 *
 *	@brief				루트 노드를 만들기 위해 처음에 값을 할당해주는 함수
 *
 *	@param	val			(넣어줄 키 값)
 *  @param  info        (넣어줄 데이터 값)
 * 
 *	@return				(스트럭쳐 포인터)
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
 *	@brief	삽입할때 노드가 꽉 찼다면 ( keyNum == MAX_KEY + 1), 노드를 분할 해주는 함수
 *
 *	@param	pos			(값을 넣어줘야할 위치)
 *  @param	node		(분할할 노드)
 *  @param	parent		(부모 노드)
 *
 *	@return				(스트럭쳐 포인터)
 */

struct Node* splitNode(int pos, struct Node* node, struct Node* parent) {
	int median = node->keyNum / 2;
	struct Node* child;

	child = (struct Node*)malloc(sizeof(struct Node));
	child->leaf = node->leaf;
	child->keyNum = 0;
	child->childNum = 0;
	child->endLeaf = node->endLeaf;						// 나눌 노드가 마지막 리프 노드일 경우 새로운 노드가 마지막 리프다.
	child->next = node->next;							

	if (node->leaf) {									// 만약 리프 노드에서의 분할이라면
		node->next = child;								// 새 노드와 기존 노드를 링크 시켜준다.
		node->endLeaf = false;							// 리프노드가 분할된다면 기존 노드는 더이상 마지막 리프일 수 없다.

		int num_iter = node->keyNum;
		for (int i = median; i < num_iter; i++) {		// child right에 키 담아주기. 기존 노드는 child left가 된다.
			child->key[i - median] = node->key[i];
			child->data[i - median] = node->data[i];
			node->keyNum--;
			child->keyNum++;
		}
	}
	else {												// leaf가 아닌 경우는  child를 넘겨줘야 함
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

	if (node == root) {									// 나눌 노드가 루트라면
		struct Node* new_parent;						// 새로운 aprent에는 median 키를 넣어준다.
		new_parent = createNode(node->key[median], node->data[median]);
		if (!node->leaf) {								// 리프가 아닐때만 기존 노드에서 median이 사라진다.
			node->keyNum--;								// median 보냈으므로 -1
		}
		new_parent->child[0] = node;
		new_parent->child[1] = child;
		new_parent->childNum = 2;
		parent = new_parent;
		return parent;
	}
	else {												// 나눌 노드가 루트가 아니라면
		for (int i = parent->keyNum; i > pos; i--) {    // 부모에 들어갈 자리를 만든다.
			parent->key[i] = parent->key[i - 1];
			parent->data[i] = parent->data[i - 1];
			parent->child[i + 1] = parent->child[i];
		}
		parent->key[pos] = node->key[median];           // 부모 노드에 key를 추가해 준다.
		parent->data[pos] = node->data[median];         // 부모 노드에 data를 추가해 준다.
		parent->keyNum++;
		if (!node->leaf) {								// 리프가 아닐 때만 기존 노드에서 median이 사라진다.
			node->keyNum--;								// median 보냈으므로 -1
		}
		parent->child[pos + 1] = child;					// 부모 노드에 새 child를 추가해 준다.
		parent->childNum += 1;
	}
	return node;
}


struct Node* insertNode(int parent_pos, int val, int info, struct Node* node, struct Node* parent) {
	int pos;												// pos는 삽입 될 포지션
	for (pos = 0; pos < node->keyNum; pos++) {
		if (val == node->key[pos]) {
			printf("Duplicates are not permitted!!\n");		// 중복된 키는 금지
			return node;
		}
		else if (val < node->key[pos]) {					// val이 들어갈 위치를 찾는다.
			break;
		}
	}
	if (!node->leaf) {										// leaf가 아닐 경우에는 child로 내려간다.
		node->child[pos] = insertNode(pos, val,info, node->child[pos], node);
		if (node->keyNum == MAX_KEY + 1) {					// 아래 삽입에 의해 키가 꽉 찼다면, 또 split 해준다.
			node = splitNode(parent_pos, node, parent);
		}
	}		
	else {													// leaf에만 삽입한다.													
		for (int i = node->keyNum; i > pos; i--) {
			node->key[i] = node->key[i - 1];
		}
		node->key[pos] = val;
		node->data[pos] = info;
		node->keyNum++;
		if (node->keyNum == MAX_KEY + 1) {					// 키가 꽉찻다면 split 해준다.
			node = splitNode(parent_pos, node, parent);
		}
	}

	return node;											// 노드 반환
}

void insert(int val, int info) {
	if (!root) { 											// 루트가 없다면 루트를 만든다.
		root = createNode(val, info);
		root->leaf = true;
		root->endLeaf = true;
		return;
	}

	root = insertNode(0, val, info, root, root);			// 루트가 있다면 노드를 찾아 삽입한다.
}



void borrowFromLeft(struct Node* node, int pos) {

	if (node->child[pos]->leaf) {												// leaf에서 borrow가 일어난다면
		int target = 0;															// 부족한 위치에 들어가야할 index
		int borrow = node->child[pos - 1]->keyNum - 1;							// 가져올 위치의 index

		for (int i = 0; i < node->child[pos]->keyNum; i++) {					// 채울 위치를 비워 둔다.
			node->child[pos]->key[i + 1] = node->child[pos]->key[i];
		}
		node->child[pos]->key[target] = node->child[pos - 1]->key[borrow];	    // 빌려올 형제로 채운다.
		node->child[pos]->keyNum++;

		node->child[pos - 1]->keyNum--;											// 가져온 곳의 키 갯수를 지운다.

		int successor = findSuccessor(node->child[pos]);						// successor로 나를 채운다.
		node->key[pos - 1] = successor;
	}
	else {																		// 내부에서 borrow가 일어난다면
		int target = 0;															// 부족한 위치에 들어가야할 index
		for (int i = 0; i < node->child[pos]->keyNum; i++) {
			node->child[pos]->key[i + 1] = node->child[pos]->key[i];
		}
		node->child[pos]->key[target] = node->key[pos - 1];						// 내 꺼를 내준다.
		node->child[pos]->keyNum++;

		int borrow = node->child[pos - 1]->keyNum - 1;							// 가져올 위치의 index
		node->key[pos - 1] = node->child[pos - 1]->key[borrow];					// 내 꺼를 채운다.

		node->child[pos - 1]->keyNum--;											// 가져온 곳의 키갯수를 지운다.

		if (node->child[pos - 1]->childNum > 0) {								// 빌려온 곳의 child가 있다면
			borrow = node->child[pos - 1]->childNum - 1;						// 빌려온 곳의 child를 넘겨준다.
			for (int i = node->child[pos]->childNum; i > 0; i--) {
				node->child[pos]->child[i] = node->child[pos]->child[i - 1];
			}
			node->child[pos]->child[0] = node->child[pos - 1]->child[borrow];
			node->child[pos]->childNum++;

			node->child[pos - 1]->childNum--;									// 빌려온 곳의 child 정리
		}
	}

}
void borrowFromRight(struct Node* node, int pos) {
	if (node->child[pos]->leaf) {												// 리프에서 borrow가 일어난다면
		int target = node->child[pos]->keyNum;									// 부족한 위치에 들어가야할 index
		int borrow = 0;															// 가져올 위치의 index

		node->child[pos]->key[target] = node->child[pos + 1]->key[borrow];	    // 빌려올 형제로 채운다.
		node->child[pos]->keyNum++;

		for (int i = 0; i < node->child[pos + 1]->keyNum - 1; i++) {			// 빌려온 곳의 키 정리
			node->child[pos + 1]->key[i] = node->child[pos + 1]->key[i + 1];
		}
		node->child[pos + 1]->keyNum--;

		int successor = findSuccessor(node->child[pos + 1]);				    // successor로 나를 채운다.
		node->key[pos] = successor;
	}
	else {																		// 내부에서 borrow가 일어난다면
		int target = node->child[pos]->keyNum;									// 부족한 위치에 들어가야할 index
		node->child[pos]->key[target] = node->key[pos];							// 내 꺼를 내준다.
		node->child[pos]->keyNum++;

		int borrow = 0;															// 가져올 위치의 index(맨 왼쪽)
		node->key[pos] = node->child[pos + 1]->key[borrow];						// 내 꺼를 채운다.

		for (int i = 0; i < node->child[pos + 1]->keyNum - 1; i++) {			// 빌려온 곳의 키 정리
			node->child[pos + 1]->key[i] = node->child[pos + 1]->key[i + 1];
		}
		node->child[pos + 1]->keyNum--;

		if (node->child[pos + 1]->childNum > 0) {
			target = node->child[pos]->childNum;								// 빌려온 곳의 child를 넘겨준다.
			node->child[pos]->child[target] = node->child[pos + 1]->child[borrow];
			node->child[pos]->childNum++;

			for (int i = 0; i < node->child[pos + 1]->childNum - 1; i++) {		// 빌려온 곳의 child 정리
				node->child[pos + 1]->child[i] = node->child[pos + 1]->child[i + 1];
			}
			node->child[pos + 1]->childNum--;
		}
	}
}

void mergeNode(struct Node* node, int pos, int pos_left) {						// 노드 키의 pos_left 값을 내려준다.
	if (node->child[pos]->leaf) {												// 리프에서 merge가 일어난다면
		int target = node->child[pos_left]->keyNum;

		for (int i = 0; i < node->child[pos]->keyNum; i++) {					// 오른쪽자식의 키를 왼쪽 자식에 넣어준다.
			node->child[pos_left]->key[target + i] = node->child[pos]->key[i];
			node->child[pos_left]->keyNum++;
		}

		node->child[pos_left]->next = node->child[pos]->next;
		node->child[pos_left]->endLeaf = node->child[pos]->endLeaf;

		for (int i = pos_left; i < node->keyNum - 1; i++) {						// 내 노드의 key 정리 
			node->key[i] = node->key[i + 1];
		}
		node->keyNum--;

		for (int i = pos; i < node->childNum - 1; i++) {						// 내 노드의 child 정리 
			node->child[i] = node->child[i + 1];
		}
		node->childNum--;

	}
	else {																			// 노드에서 merge가 일어난다면
		int target = node->child[pos_left]->keyNum;									// 왼쪽 자식에 들어갈 index

		node->child[pos_left]->key[target] = node->key[pos_left];					// 내 노드값을 넣어준다.
		node->child[pos_left]->keyNum++;

		for (int i = 0; i < node->child[pos]->keyNum; i++) {						// 오른쪽자식의 키를 왼쪽 자식에 넣어준다.
			node->child[pos_left]->key[target + 1 + i] = node->child[pos]->key[i];
			node->child[pos_left]->keyNum++;
		}

		target = node->child[pos_left]->childNum;
		for (int i = 0; i < node->child[pos]->childNum; i++) {						// 오른쪽자식의 자식을 왼쪽 자식에 넣어준다.
			node->child[pos_left]->child[target + i] = node->child[pos]->child[i];
			node->child[pos_left]->childNum++;
		}

		free(node->child[pos]);														// 오른쪽 자식을 free 시킨다.

		for (int i = pos_left; i < node->keyNum - 1; i++) {							// 내 노드의 key 정리
			node->key[i] = node->key[i + 1];
		}
		node->keyNum--;

		for (int i = pos; i < node->childNum - 1; i++) {							// 내 노드의 child 정리 
			node->child[i] = node->child[i + 1];
		}
		node->childNum--;
	}
}

void adjustNode(struct Node* node, int pos) {
	if (pos == 0) {													// child 가 왼쪽 끝일때, 오른쪽 형제에서만 빌려올 수 있다.
		if (node->child[pos + 1]->keyNum > MIN_KEY) {
			borrowFromRight(node, pos);
		}
		else {
			mergeNode(node, pos + 1, pos);
		}
		return;
	}
	else {
		if (pos == node->keyNum) {									// child 가 오른쪽 끝일때, 왼쪽 형제에서만 빌려올 수 있다.
			if (node->child[pos - 1]->keyNum > MIN_KEY) {					
				borrowFromLeft(node, pos);
			}
			else {
				mergeNode(node, pos, pos - 1);
			}
			return;
		}
		else {														// 그 외 상황에서는, 양쪽에서 빌려올 수 있다.
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
	for (pos = 0; pos < node->keyNum; pos++) {						// 이 노드에서 val, 혹은 들어갈 위치를 찾는다.
		if (val == node->key[pos]) {
			flag = true;
			break;
		}
		if (val < node->key[pos]) {
			break;
		}
	}
	if (flag) {
		if (node->leaf) {											// case#1 leaf에서 삭제될 때
			for (int i = pos; i < node->keyNum; i++) {
				node->key[i] = node->key[i + 1];
			}
			node->keyNum--;
		}
		else {
			flag = deleteValFromNode(val, node->child[pos + 1]);	// 내부에서 발견했을 때는 다음 child로 넘어간다.
			if (node->child[pos + 1]->keyNum < MIN_KEY) {			// 재귀로 나왔을 때 삭제했던 자식이 갯수가 모자를 때 
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
	if (node->child[pos]->keyNum < MIN_KEY) {						// 재귀로 나왔을 때 삭제했던 자식이 갯수가 모자를 때 
		adjustNode(node, pos);
	}

	return flag;
}

void delete(struct Node* node, int val) {
	if (!node) { 													// 트리가 비었을 때
		printf("Empty tree!!\n");
		return;
	}
	int flag = deleteValFromNode(val, node);
	if (!flag) { 													// 삭제 할 노드가 없을 때
		printf("%d no exist in the tree!!\n", val);
		return;
	}

	if (node->keyNum == 0) {										// case#3 높이의 변화가 있을때
		node = node->child[0];
	}
	root = node;
}



void printTree(struct Node* node, int level) {				
	if (!node) { 													// 트리가 비었을 때
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
	if (!node) { 													// 트리가 비었을 때
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

	printf("프로그램이 정상적으로 종료 되었음.");
	return 0;

}

