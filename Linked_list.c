#include <stdio.h>
#include <stdlib.h>
#include "Linked_list.h"
#define OK 1;


void insertFirst(struct LinkedList **first, struct LinkedList *el) {
  /*
  Function inserts el at the beginning of the list and updates first.
  */

  struct LinkedList *tempNode = *first; // asigns tempnode to first

  if (tempNode == el) {
    el = *first; 
  } else {
    el->next = tempNode;
    el = *first;
  }
}

int isMember(struct LinkedList **first, struct LinkedList *el) {
  /*
  Function returns 1 if el is a member in the list and 0 otherwise.
  */
  struct LinkedList *tempNode;

  tempNode = *first;
  while (tempNode != NULL) {
    if (tempNode == el) {
      return 1;
    }
    tempNode = tempNode->next;
  }

  return 0;
}

void printList(struct LinkedList **first) {
  /*
  Function prints the list from the beginning till the end.
  Each element should be printed on a separate line with the id value and the
  sensorData value separated by a tab.
  */

  struct LinkedList *tempNode = *first;

  while (tempNode != NULL) {
    printf("%d%s%f\n", tempNode->id, "	", tempNode->sensorData);
    tempNode = tempNode->next;
  }
}

void remover(struct LinkedList **first, struct LinkedList *el) {
  /*
  Function removes el from the list (if it's a member) without deleting it from
  memory.
  */

  struct LinkedList *tempNode;
  struct LinkedList *currentNode = *first;

  if (isMember(first, el) == 1) {
    // Check if el is a member
    if (currentNode == el) {
      // The first node was el
      tempNode = el->next;
      el->next = NULL;
      tempNode = *first;
      return;
    }

    while (currentNode->next != NULL) {
      // Finding the node before el
      if (currentNode->next == el) {
        // We have found the node before el
        tempNode = currentNode->next;
        currentNode->next = tempNode->next;
        return;
      }

      currentNode = currentNode->next;
    }
  } else {
    // El wasn't a member
    printf("ERROR: That node is not a member!\n");
  }
}

struct LinkedList *createNode(int i, double data) {
  /* 
  Creates a node with id i, and sensorData data
  */
  struct LinkedList *p;
  p = (struct LinkedList *)malloc(sizeof(struct LinkedList));
  if(p == NULL){
    return NULL;
  } 
  p->id = i;
  p->sensorData = data;
  p->next = NULL;
  return p;
}

struct LinkedList *readSensor(int id) {
  struct LinkedList *sensorNode =
      createNode(id, (double)(rand() % 10000) / 10000.0);

  return sensorNode;
}

struct LinkedList *sort(struct LinkedList **first) {
  /*
  Sort the list in ascending order (smallest to largest) based on sensorData.
*/

  int length;
  struct LinkedList *headNode = *first;
  struct LinkedList *currentNode = *first;
  struct LinkedList *biggestNode = *first;

  length = size(first);

  for (int i = 0; i < length; i++) {
    while (currentNode != NULL) {
      // printf("I:%d %lf < %lf ", i, biggestNode->sensorData,
      // currentNode->sensorData);
      if (biggestNode->sensorData < currentNode->sensorData) {
        // printf("TRUE");
        biggestNode = currentNode;
      }
      // printf("\n");
      currentNode = currentNode->next;
    }
    // printf("ID:%d, %lf \n", biggestNode->id, biggestNode->sensorData);
    remover(&headNode, biggestNode);
    insertFirst(&headNode, biggestNode);
    headNode = biggestNode;
    currentNode = headNode;

    for (int j = 0; j < (i + 1); j++) {
      if (currentNode != NULL) {
        currentNode = currentNode->next;
        biggestNode = currentNode;
      }
    }
  }
  return headNode;
}

int size(struct LinkedList **first) {
  int count = 0;
  struct LinkedList *tempNode = *first;

  while (tempNode != NULL) {
    count++;
    tempNode = tempNode->next;
  }

  return count;
}
                                   
void release(struct LinkedList **first) {
  // free memory function
  struct LinkedList *tempNode = *first;
  struct LinkedList *currentNode = *first;

  while (currentNode != NULL) {
    tempNode = currentNode;
    currentNode = currentNode->next;
    free(tempNode);
  }
}

/*
int isMin(struct LinkedList *currentNode, struct LinkedList *minNode){ // testa att ha dessa kvar
  if ((currentNode->sensorData) < (minNode->sensorData)){ // currentnode gets sensordata and vv for minnode
    return 1;
  } else{
    return 0;
  }
}

int isMax(struct LinkedList *currentNode, struct LinkedList *maxNode){
  if ((currentNode->sensorData) > (maxNode->sensorData)){
    return 1;
  } else{
    return 0;
  }
}
    
double getSensorData(struct LinkedList *el){
  double data = el->sensorData;
  printf("HORUNGE%f\n", data);
  return data;
}
                                     
void pointToNext(struct LinkedList *el){
  el = el->next;
}
*/