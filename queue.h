// queue.h
// Day 2: Queue structure definition

#ifndef QUEUE_H
#define QUEUE_H

#define MAX_QUEUE_SIZE 100   // we will adjust later if needed

// Simple circular queue structure
typedef struct {
    int data[MAX_QUEUE_SIZE];  
    int front;
    int rear;
    int size;
} Queue;

#endif
