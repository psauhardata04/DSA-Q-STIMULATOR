#include "queue.h"

// Day 4
void initQueue(Queue *q) {
    q->front = 0;
    q->rear = -1;
    q->size = 0;
}

// Day 5: Added isQueueFull() and isQueueEmpty()
int isQueueFull(Queue *q) {
    return q->size == MAX_QUEUE_SIZE;
}

int isQueueEmpty(Queue *q) {
    return q->size == 0;
}
