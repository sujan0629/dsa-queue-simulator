#ifndef QUEUE_H
#define QUEUE_H

#include <stdbool.h>

// Define Vehicle structure
typedef struct {
    int id;
    // Add more fields if needed, e.g., arrival_time, etc.
} Vehicle;

// Node for linked list
typedef struct Node {
    Vehicle vehicle;
    struct Node* next;
} Node;

// Queue structure
typedef struct {
    Node* front;
    Node* rear;
    int size;
} Queue;

// Function prototypes
Queue* createQueue();
void enqueue(Queue* q, Vehicle v);
Vehicle dequeue(Queue* q);
bool isEmpty(Queue* q);
int getSize(Queue* q);
void freeQueue(Queue* q);

#endif // QUEUE_H
