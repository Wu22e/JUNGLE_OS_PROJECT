/**
 *	@file		Implementation of B-Tree
 *	@brief      B-Tree 탐색, 삽입, 삭제 구현
 *
 *	@date		2021-01-11
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#define Node_Order			5										// 최소 차수 3으로 구현
#define Node_Childs			Node_Order								// 최대 자식 갯수
#define Node_Keys			Node_Childs-1							// 최대 키 갯수
#define Num_Minimum_Keys	(int) (ceil(Node_Order/2.0)) - 1        // 최소 키 갯수

#define TESTCASE1 1													// 테스트케이스 1
#define TESTCASE2 0													// 테스트케이스 2

 /**
  *	@struct	  BTreeNode
  *
  *	@brief	  비트리의 노드 구조체
  *
  * @member	  leaf                     (리프인지 아닌지 판단)
  * @member   key[Node_Keys + 1]       (키 데이터)
  * @member   num_key                  (키 갯수)
  * @member   num_child			       (자식 개수)
  *
  * @link     child[Node_Childs + 1]   (자식의 주소를 저장할 포인터)
  */

struct BTreeNode {
	bool leaf;
	int key[Node_Keys + 1];
	int num_key;
	struct BTreeNode* child[Node_Childs + 1];
	int num_child;
};

struct BTreeNode* root;							// 루트 포인터

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

int searchNode(struct BTreeNode* node, int val) {			// 이진 탐색 구현 해보기

	if (!node) { 											// 트리가 비었다면
		printf("Empty tree!!\n");
		return 0;
	}
	struct BTreeNode* level = node;							// root부터 leaf까지 탐색
	while (1) {
		int pos;
		for (pos = 0; pos < level->num_key; pos++) {
			if (val == level->key[pos]) {					// 찾으면 리턴 1 (TRUE)
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
	printf("%d not exist!!\n", val);					    // leaf 까지와서도 못찾으면 리턴 0 (FALSE)
	return 0;
}



/**
 *	@fn		createNode
 *
 *	@brief				루트 노드를 만들기 위해 처음에 값을 할당해주는 함수
 *
 *	@param	val			(넣어줄 키 값)
 *
 *	@return				(스트럭쳐 포인터)
 */

struct BTreeNode* createNode(int val) {
	struct BTreeNode* newNode;										  
	newNode = (struct BTreeNode*)malloc(sizeof(struct BTreeNode));    // B트리 구조체만큼 동적할당

	newNode->leaf = false;
	newNode->key[0] = val;
	newNode->num_key = 1;
	newNode->num_child = 0;
	return newNode;
}

/**
 *	@fn		splitNode
 *
 *	@brief	삽입할때 노드가 꽉 찼다면 ( num_key == Node_keys + 1), 노드를 분할 해주는 함수
 *
 *	@param	pos			(값을 넣어줘야할 위치)
 *  @param	node		(분할할 노드)
 *  @param	parent		(부모 노드)
 *
 *	@return				(스트럭쳐 포인터)
 */

 //Split Node
struct BTreeNode* splitNode(int pos, struct BTreeNode* node, struct BTreeNode* parent) {
	int median;													 // 분리를 위한 중앙값
	if (Node_Order % 2 == 0) {									 // 차수가 짝수일 때
		median = node->num_key / 2 - 0.5;
	}
	else {														 // 차수가 홀수일 때
		median = node->num_key / 2;
	}
	struct BTreeNode* child;
	child = (struct BTreeNode*)malloc(sizeof(struct BTreeNode));
	child->leaf = node->leaf;									 
	child->num_key = 0;
	child->num_child = 0;
	if (!node->leaf) {											 // leaf가 아닌데 분할할 경우, child를 넘겨줘야함
		int num_iter = node->num_child;							 // 미리 빈 child 노드를 만들고 원래 노드의 자식들을 연결시킨다.
		for (int i = median + 1; i < num_iter; i++) {
			child->child[i - median - 1] = node->child[i];
			child->num_child++;
			node->num_child--;
		}
	}
	int num_iter = node->num_key;
	for (int i = median + 1; i < num_iter; i++) {				 // child right에 키 담아주기 기존 노드는 child left가 된다
		child->key[i - median - 1] = node->key[i];
		child->num_key++;
		node->num_key--;
	}
	if (node == root) {											 // 나눌 노드가 루트라면
		struct BTreeNode* new_parent;							 // 새로운 parent에는 median 키를 넣어준다.
		new_parent = createNode(node->key[median]);
		node->num_key--;										 // median 보냈으므로 -1
		new_parent->child[0] = node;
		new_parent->child[1] = child;
		new_parent->num_child = 2;
		parent = new_parent;
		return parent;
	}
	else {														 // 나눌 노드가 루트가 아니라면
		for (int i = parent->num_key; i > pos; i--){
			parent->key[i] = parent->key[i - 1];
			parent->child[i + 1] = parent->child[i];			 
		}
		parent->key[pos] = node->key[median];					 // 부모 노드에 key를 추가해준다.
		parent->num_key++;
		node->num_key--;										 // median 보냈으므로 -1
		parent->child[pos + 1] = child;							 // 부모 노드에 새 child를 추가해준다.
		parent->num_child += 1;
	}
	return node;
}

/**
 *	@fn		insertNode
 *
 *	@brief	키 값을 노드에 삽입하는 함수 (재귀)
 *
 *	@param	parent_pose					(부모의 위치)
 *  @param	val							(삽입할 키 값)
 *  @param	node						(현재 노드)
 *  @param	parent						(부모 노드)
 *
 *	@return								(스트럭쳐 포인터)
 */
struct BTreeNode* insertNode(int parent_pos, int val, struct BTreeNode* node, struct BTreeNode* parent) {

	int pos;																// pos는 삽입 될 포지션
	for (pos = 0; pos < node->num_key; pos++) {
		if (val == node->key[pos]) {
			printf("Duplicates are not permitted!!\n");						// 중복된 키는 금지
			return node;
		}
		else if (val < node->key[pos]) {									// val이 들어갈 위치를 찾는다.
			break;
		}
	}

	if (!node->leaf) {														// leaf가 아닐 경우에는 child로 내려간다.
		node->child[pos] = insertNode(pos, val, node->child[pos], node);	// 재귀로 들어가면 parent_pos가 바뀜
		if (node->num_key == Node_Keys + 1) {								// 아래 삽입에 의해 키가 찼다면, 또 split을 해준다
			node = splitNode(parent_pos, node, parent);
		}
	}
	else {																	// leaf에만 삽입한다.
		for (int i = node->num_key; i > pos; i--) {
			node->key[i] = node->key[i - 1];
			node->child[i + 1] = node->child[i];							
		}
		node->key[pos] = val;
		node->num_key++;
		if (node->num_key == Node_Keys + 1) {								// 키가 꽉찼다면? split을 해준다!
			node = splitNode(parent_pos, node, parent);
		}
	}

	return node;															// 노드 반환
}


/**
 *	@fn		insert
 *	@brief			처음에 root가 없다면 만들어주고, root가 있다면 노드를 찾아 삽입하는 함수
 *
 *	@param	val		(삽입할 키값)
 *
 *	@return			(void)
 */

void insert(int val) {
	if (!root) { 											// root 가 없다면 root를 만든다.
		root = createNode(val);
		root->leaf = true;
		return;
	}

	root = insertNode(0, val, root, root);					// root 가 있다면 노드를 찾아 삽입한다.
}


/**
 *	@fn		findPredecessor
 *	@brief  predecessor를 찾아서 반환해주는 함수 (재귀로 찾음)
 *
 *	@param	node				(해당 노드)
 *
 *	@return						(int)
 */

int findPredecessor(struct BTreeNode* node) {
	int predecessor;												// 선행자 선언
	if (node->leaf) {												// 리프일 경우 가장 오른쪽 값을 반환
		return node->key[node->num_key - 1];
	}
	return findPredecessor(node->child[node->num_child - 1]);		// 리프가 아닐 경우 재귀로 들어감 (현재 노드의 가장 오른쪽 자식으로 들어감)
}

/**
 *	@fn		findSuccessor
 *	@brief  sucessor를 찾아서 반환해주는 함수 (재귀로 찾음)
 *
 	@param	node				(해당 노드)
 *
 *	@return						(int)
 */

int findSuccessor(struct BTreeNode* node) {
	int successor;													// 후행자 선언
	if (node->leaf) {												// 리프일 경우 가장 왼쪽 값을 반환
		return node->key[0];

		return findSuccessor(node->child[0]);
	}// 리프가 아닐 경우 재귀로 들어감 (현재 노드의 가장 왼쪽 자식으로 들어감)
}

/**
 *	@fn		inorderPredecessor
 *	@brief  해당 노드에서 inorderpredecessor를 찾아서 반환한다.
 *
 *	@param	node				(해당 노드)
 *  @param	pos				    (해당 위치)
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
 *	@brief  해당 노드에서 inordersuccessor를 찾아서 반환한다.
 * 
 *	@param	node				(해당 노드)
 *  @param	pos				    (해당 위치)
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
 *	@param	node				(삽입할 키값)
 *  @param	pos				    (삽입할 키값)
 *
 *	@return						(void)
 */


int inorderMerge(struct BTreeNode* node, int pos) {
	int target = node->child[pos]->num_key;					// 합칠 위치에 들어가야할 idx
	int send = node->key[pos];
	node->child[pos]->key[target] = node->key[pos];			// 왼쪽 child에 지워질 값 붙여넣기
	node->child[pos]->num_key++;

	for (int i = 0; i < node->child[pos + 1]->num_key; i++) {   // 오른쪽 child의 키 왼쪽에 붙여넣기
		node->child[pos]->key[target + 1 + i] = node->child[pos + 1]->key[i];
		node->child[pos]->num_key++;
	}
	for (int i = 0; i < node->child[pos + 1]->num_child; i++) { // 오른쪽 child의 child 왼쪽에 붙여넣기
		node->child[pos]->child[target + 1 + i] = node->child[pos + 1]->child[i];
		node->child[pos]->num_child++;
	}

	for (int i = pos; i < node->num_key; i++) {				// 내 노드 키값 정리
		node->key[i] = node->key[i + 1];
		node->num_key--;
	}
	for (int i = pos + 1; i < node->num_child; i++) {			// 내 노드 child값 정리
		node->child[i] = node->child[i + 1];
		node->num_child--;
	}
	return send;
}



/**
 *	@fn		deleteInnerTree
 *	@brief  내부 노드를 삭제하기 위한 여러 함수를 모아둔 함수
 *
 *	@param	node				(해당 노드)
 *  @param	pos				    (해당 위치)
 *
 *	@return						(void)
 */

void deleteInnerTree(struct BTreeNode* node, int pos) {
	int result_deletion = 0; 
	if (node->child[pos]->num_key >= node->child[pos + 1]->num_key) { // 왼쪽 자식의 키 갯수가 크거나 같을때
		if (node->child[pos]->num_key > Num_Minimum_Keys) {
			result_deletion = inorderPredecessor(node, pos);
			deleteValFromNode(result_deletion, node->child[pos]);

		}
		else {
			result_deletion = inorderMerge(node, pos);
			deleteValFromNode(result_deletion, node->child[pos]);
		}
	}
	else {															  // 오른쪽 자식의 키 갯수가 클 때
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
 *	@brief  왼쪽에서 빌려오기
 *
 *	@param	node				(해당 노드)
 *  @param	pos				    (해당 위치)
 *
 *	@return						(void)
 */

void borrowFromLeft(struct BTreeNode* node, int pos) {
	int target = 0;													 // 부족한 위치에 들어가야할 idx
	for (int i = 0; i < node->child[pos]->num_key; i++) {
		node->child[pos]->key[i + 1] = node->child[pos]->key[i];
	}
	node->child[pos]->key[target] = node->key[pos - 1];				 // 내 꺼를 내준다.
	node->child[pos]->num_key++;

	int borrow = node->child[pos - 1]->num_key - 1;						 // 가져올 위치의 idx
	node->key[pos - 1] = node->child[pos - 1]->key[borrow];			 // 내 꺼를 채운다.

	node->child[pos - 1]->num_key--;								 // 가져온 곳의 키갯수를 지운다.


	if (node->child[pos - 1]->num_child > 0) {						 // 빌려온 곳의 child가 있다면
		borrow = node->child[pos - 1]->num_child - 1;			     // 빌려온 곳의 child를 넘겨준다.
		for (int i = node->child[pos]->num_child; i > 0; i--) {
			node->child[pos]->child[i] = node->child[pos]->child[i - 1];
		}
		node->child[pos]->child[0] = node->child[pos - 1]->child[borrow];
		node->child[pos]->num_child++;

		node->child[pos - 1]->num_child--;							 // 빌려온 곳의 child 정리
	}
}

/**
 *	@fn		borrowFromRight
 *	@brief  오른쪽에서 빌려오기
 *
 *	@param	node				(해당 노드)
 *  @param	pos				    (해당 위치)
 *
 *	@return						(void)
 */

void borrowFromRight(struct BTreeNode* node, int pos) {
	int target = node->child[pos]->num_key;							 // 부족한 위치에 들어가야할 idx
	node->child[pos]->key[target] = node->key[pos];				     // 내 꺼를 내준다.
	node->child[pos]->num_key++;

	int borrow = 0;												     // 가져올 위치의 idx(맨왼쪽)
	node->key[pos] = node->child[pos + 1]->key[borrow];				 // 내 꺼를 채운다.

	for (int i = 0; i < node->child[pos + 1]->num_key - 1; i++) {	 // 빌려온 곳의 키 정리
		node->child[pos + 1]->key[i] = node->child[pos + 1]->key[i + 1];
	}
	node->child[pos + 1]->num_key--;

	if (node->child[pos + 1]->num_child > 0) {
		target = node->child[pos]->num_child;							 // 빌려온 곳의 child를 넘겨준다.
		node->child[pos]->child[target] = node->child[pos + 1]->child[borrow];
		node->child[pos]->num_child++;

		for (int i = 0; i < node->child[pos + 1]->num_child - 1; i++) {	 // 빌려온 곳의 child 정리
			node->child[pos + 1]->child[i] = node->child[pos + 1]->child[i + 1];
		}
		node->child[pos + 1]->num_child--;
	}
}

/**
 *	@fn		mergeNode
 *	@brief  못 빌려올 때 합치는 함수
 *
 *	@param	node				(해당 노드)
 *  @param	pos				    (해당 위치)
 *  @param	mer_pos			(합칠 곳의 node position)
 *
 *	@return						(void)
 */

void mergeNode(struct BTreeNode* node, int pos, int mer_pos) {		// 노드 키의 poe_left 값을 내려준다. TODO:: 왜 오른쪽 기준에도 문제없이 돌아가는가?
	int target = node->child[mer_pos]->num_key;						// 왼쪽 자식에 들어갈 idx

	node->child[mer_pos]->key[target] = node->key[mer_pos];			// 내 노드값을 넣어준다.
	node->child[mer_pos]->num_key++;

	for (int i = 0; i < node->child[pos]->num_key; i++) {			// 오른쪽자식의 키를 왼쪽 자식에 넣어준다.
		node->child[mer_pos]->key[target + 1 + i] = node->child[pos]->key[i];
		node->child[mer_pos]->num_key++;
	}

	target = node->child[mer_pos]->num_child;
	for (int i = 0; i < node->child[pos]->num_child; i++) {			// 오른쪽자식의 키를 왼쪽 자식에 넣어준다.
		node->child[mer_pos]->child[target + i] = node->child[pos]->child[i];
		node->child[mer_pos]->num_child++;
	}

	free(node->child[pos]);											// 오른쪽 자식을 free 시킨다.

	for (int i = mer_pos; i < node->num_key - 1; i++) {				// 내 노드의 key 정리 
		node->key[i] = node->key[i + 1];
	}
	node->num_key--;

	for (int i = pos; i < node->num_child - 1; i++) {				// 내 노드의 child 정리 
		node->child[i] = node->child[i + 1];
	}
	node->num_child--;
}

/**
 *	@fn		adjustNode
 *	@brief  상황에 맞게 빌려올지 합칠지 결정해주는 함수
 *
 *	@param	node				(해당 노드)
 *  @param	pos				    (해당 위치)
 *
 *	@return						(void)
 */

void adjustNode(struct BTreeNode* node, int pos) {
	if (pos == 0) {														// child 가 왼쪽 끝일때, 오른쪽 형제에서만 빌려올 수 있다.
		if (node->child[pos + 1]->num_key > Num_Minimum_Keys) {
			borrowFromRight(node, pos);
		}
		else {
			mergeNode(node, pos + 1, pos);
		}
		return;
	}
	else {
		if (pos == node->num_key) {										// child 가 오른쪽 끝일때, 왼쪽 형제에서만 빌려올 수 있다.
			if (node->child[pos - 1]->num_key > Num_Minimum_Keys) {
				borrowFromLeft(node, pos);
			}
			else {
				mergeNode(node, pos, pos - 1);
			}
			return;
		}
		else {															// 그 외 상황에서는, 양쪽에서 빌려올 수 있다.
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
 *	@brief					삭제할 노드의 값을 찾아서 삭제 (case #1 리프노드 삭제 / case #2 내부노드 삭제), 경우에 따라 다른 함수를 호출
 *
 *	@param	val				(삭제할 키값)
 *  @param	node		    (현재 노드)
 *
 *	@return						(int)
 */

int deleteValFromNode(int val, struct BTreeNode* node) {
	int pos;
	int flag = false;
	for (pos = 0; pos < node->num_key; pos++) {				 // 현재 노드에서 val(삭제할 값)을 찾아서 flag를 true해주고 pos를 찾거나,
		if (val == node->key[pos]) {
			flag = true;
			break;
		}
		else if (val < node->key[pos]) {					 // 찾지 못했다면 자식으로 들어갈 곳의 pos를 찾는다
			break;
		}
	}

	if (flag) {
		if (node->leaf) {									 // case#1 leaf에서 삭제될 때
			for (int i = pos; i < node->num_key; i++) {
				node->key[i] = node->key[i + 1];
			}
			node->num_key--;
		}
		else {
			deleteInnerTree(node, pos);						 // case#2 inner node에서 삭제될 때
		}
		return flag;
	}
	else {
		if (node->leaf) {
			return flag;
		}
		else {
			flag = deleteValFromNode(val, node->child[pos]); // 여기까지 오면 현재 노드가 리프 노드가 아니면서 아직 삭제할 노드를 찾지 못한 경우다. 
															 // ( 재귀를 통해 자식 노드로 진입 )
		}
	}

	if (node->child[pos]->num_key < Num_Minimum_Keys) {      // 재귀로 나왔을 때 삭제했던 자식이 갯수가 모자를 때 
		adjustNode(node, pos);
	}

	return flag;
}

/**
 *	@fn		delete
 *	@brief					루트 부터 시작해서 삭제할 키값을 찾아서 지워주는 함수
 *
 *	@param	node			(현재 노드, 루트부터 시작)
 *  @param	val				(삭제할 키값)
 *
 *	@return					(void)
 */

void delete(struct BTreeNode* node, int val) {
	if (!node) { 											// 트리가 비었다면
		printf("Empty tree!!\n");
		return;
	}

	int flag = deleteValFromNode(val, node);

	if (!flag) { 											// 삭제 할 노드가 없다면 경고메세지 출력
		printf("%d no exist in the tree!!\n", val);
		return;
	}

	if (node->num_key == 0) {								// case#3, 높이의 변화가 있을때 (여기 물어봐야함)
		node = node->child[0];
	}
	root = node;

}

/**
 *	@fn		printTree
 *	@brief				모든 트리의 레벨과 키 값들을 프린트하는 함수 (재귀)
 *
 *	@param	node		(현재 노드, 루트부터 시작)
 * 	@param	level		(트리의 레벨)
 *
 *	@return				(void)
 */
void printTree(struct BTreeNode* node, int level) {
	if (!node) { 											 // 트리가 비어 있다면
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
 *	@brief  메인함수, 테스트케이스 실행
 */

int main(void) {

	printf("Minimum key numbers are %d\n", Num_Minimum_Keys);
	// 테스트케이스 분할
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