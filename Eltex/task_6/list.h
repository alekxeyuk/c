#ifndef LIST_H_
#define LIST_H_

#include <errno.h>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
  char name[11];
  char second_name[11];
  char tel[10];
} Abonent;

typedef struct Node {
  Abonent data;
  struct Node* prev;
  struct Node* next;
} Node;

typedef struct {
  Node* head;
  Node* tail;
} List;

Node* createNode(Abonent);
void appendNode(List*, Node*);
void appendAbonent(List*, Abonent);
Node* freeNode(Node*);
void freeAbonent(List*, Node*);
void freeList(List*);
void printList(List*);
void printAbonent(Abonent*);
Node* findNext(Node*, const char*);
Node* searchList(List*, const char*);

#endif  // LIST_H_
