#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include "linkedlist.h"


 
Node * add_newNode(Node* head, int first_cluster, int offset, int parent_addr){
	// adds a new node at the back of the list
	Node *new = (Node *)malloc(sizeof(Node));

	new->first_cluster = first_cluster;
	new->offset = offset;
	new->parent_addr = parent_addr;

	// new->name = malloc(strlen(name)+1);
	// strcpy(new->path,new_path);

	if (head==NULL) {
		head = new;
		head->next=NULL;
	} else {
		Node *curr = head;
		while (curr->next!=NULL){
			curr = curr->next;
		}
		curr->next = new;
		new->next=NULL;
	}

	return head;
}

/*
Node * deleteNode(Node* head, int first_cluster, int offset){
	// delets node if PifExist(head,pid)==1
	if (head==NULL) {
		return NULL;
	} 
	Node *curr = head;
	Node *prev = NULL;
	if (PifExist(head,pid)==1) {
		while (curr!=NULL) {
			if (curr->pid==pid) {
				if (prev==NULL) {
					head = curr->next;
					free(curr);
					return head;
				} else if (curr->next==NULL) {
					prev->next = NULL;
					curr->next = NULL;
					free(curr);
					return head;
				} else {
					prev->next = curr->next;
					free(curr);
					return head;
				}
				
			} else {
				prev = curr;
				curr = curr->next;
			}
		}
	}
	return head;

}
*/

int searchParent(Node *node, int addr){
	Node *curr = node;
	Node *prev = NULL;

	while (curr!=NULL) {
		if (curr->offset==addr) {
			// return curr->addr;
			return curr->parent_addr;
		} else {
			prev = curr;
			curr = curr->next;
		}
	}
	return 0;
}

int numSubDir(Node *node) {
	Node *curr = node;
	int count = 0;
	while (curr!=NULL) {
		count++;
		curr = curr->next;
	}
	return count;
}


void printList(Node *node){
	// prints linked list from head node
	// and traverse throughout the entire list
	// just until curr!=NULL
	Node *curr = node;
	int count = 0;
	while (curr!=NULL) {
		printf("offset @%d: %d; parent address: %d\n", curr->offset, curr->first_cluster,curr->parent_addr);
		count++;
		curr = curr->next;
	}
	// printf("Total background jobs: %d\n",count);
}


// int PifExist(Node *node, pid_t pid){
	/*
	checks whether current list contains the pid
// 	if Yes, return 1; return 0 otherwise
// 	*/
// 	// your code here
// 	Node *curr = node;
// 	Node *prev = NULL;

// 	while (curr!=NULL) {
// 		if (curr->pid==pid) {
// 			return 1;
// 		} else {
// 			prev = curr;
// 			curr = curr->next;
// 		}
// 	}
// 	return 0;
// }

// */