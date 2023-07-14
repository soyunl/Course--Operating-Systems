#ifndef _LINKEDLIST_H_
#define _LINKEDLIST_H_

typedef struct Node Node;

struct Node{
    int first_cluster;
    int offset;
    int parent_addr;
    Node * next;
};


Node * add_newNode(Node* head, int first_cluster, int offset, int parent_addr);
// Node * deleteNode(Node* head, int first_cluster, int offset);
void printList(struct Node *node);
int numSubDir(struct Node *node);
int searchParent(Node *node, int parent_addr);

// int PifExist(Node *node, pid_t pid);



#endif