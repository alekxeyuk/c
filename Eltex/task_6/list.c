#include "list.h"

Node* createNode(Abonent d) {
  Node* ptr = malloc(sizeof(Node));
  if (ptr == NULL) {
    perror("Failed to create a new Node");
    exit(-1);
  }

  ptr->data = d;
  ptr->next = NULL;
  ptr->prev = NULL;

  return ptr;
}

void appendNode(List* lst, Node* n) {
  n->next = NULL;
  if (lst->tail == NULL) {
    lst->head = n;
    lst->tail = n;
    n->prev = NULL;
  } else {
    lst->tail->next = n;
    n->prev = lst->tail;
    lst->tail = n;
  }
}

Node* freeNode(Node* n) {
  if (n == NULL) {
    perror("Fatal error when freeing node");
    exit(-1);
  }

  Node* next = n->next;

  if (n->prev != NULL && n->next != NULL) {
    n->prev->next = n->next;
    n->next->prev = n->prev;
  } else if (n->prev != NULL) {
    n->prev->next = NULL;
  } else if (n->next != NULL) {
    n->next->prev = NULL;
  }

  free(n);
  return next;
}

void freeList(List* lst) {
  Node* ptr = lst->head;
  while (ptr != NULL) {
    ptr = freeNode(ptr);
  }
}

void printList(List* lst) {
  Node* ptr = lst->head;
  while (ptr != NULL) {
    printAbonent(&ptr->data);
    ptr = ptr->next;
  }
}

Node* findNext(Node* n, const char* name) {
  if (n == NULL) return NULL;
  Node* ptr = n->next;
  while (ptr != NULL) {
    if (strncmp(name, ptr->data.name, 11) == 0) {
      return ptr;
    }
    ptr = ptr->next;
  }
  return NULL;
}

Node* searchList(List* lst, const char* name) {
  if (lst->head != NULL) {
    if (strncmp(name, lst->head->data.name, 11) == 0) {
      return lst->head;
    }
  }
  return findNext(lst->head, name);
}

int strncmp(const char* ptr1, const char* ptr2, size_t n) {
  char* f = (char*)ptr1;
  char* s = (char*)ptr2;
  for (size_t i = 0; i < n; i++) {
    if (f[i] == '\0' && s[i] == '\0') return 0;
    if (f[i] > s[i]) return 1;
    if (f[i] < s[i]) return -1;
  }
  return 0;
}
