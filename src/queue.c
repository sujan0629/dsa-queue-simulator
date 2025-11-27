#include "queue.h"
#include <stdlib.h>
#include <stdio.h>

// Create a new queue
Queue* createQueue() {
    Queue* q = (Queue*)malloc(sizeof(Queue));
    if (q == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }
    q->front = q->rear = NULL;
    q->size = 0;
    return q;
}

// Enqueue a vehicle
void enqueue(Queue* q, Vehicle v) {
    Node* newNode = (Node*)malloc(sizeof(Node));
    if (newNode == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }
    newNode->vehicle = v;
    newNode->next = NULL;
    if (q->rear == NULL) {
        q->front = q->rear = newNode;
    } else {
        q->rear->next = newNode;
        q->rear = newNode;
    }
    q->size++;
}

// Dequeue a vehicle
Vehicle dequeue(Queue* q) {
    if (isEmpty(q)) {
        fprintf(stderr, "Queue is empty\n");
        exit(1);
    }
    Node* temp = q->front;
    Vehicle v = temp->vehicle;
    q->front = q->front->next;
    if (q->front == NULL) {
        q->rear = NULL;
    }
    free(temp);
    q->size--;
    return v;
}

// Check if queue is empty
bool isEmpty(Queue* q) {
    return q->front == NULL;
}

// Get size of queue
int getSize(Queue* q) {
    return q->size;
}

// Free the queue
void freeQueue(Queue* q) {
    while (!isEmpty(q)) {
        dequeue(q);
    }
    free(q);
}
