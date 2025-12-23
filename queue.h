

#ifndef QUEUE_H
#define QUEUE_H

#define MAX_QUEUE_SIZE 100

// Circular queue structure
typedef struct {
    int data[MAX_QUEUE_SIZE];
    int front;
    int rear;
    int size;
} Queue;

// Day 3: new function prototype
void initQueue(Queue *q);

#endif

