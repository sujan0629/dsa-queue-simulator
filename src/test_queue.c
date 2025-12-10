#include <stdio.h>
#include <assert.h>
#include "queue.h"

void test_queue() {
    Queue* q = createQueue();
    assert(isEmpty(q));
    assert(getSize(q) == 0);

    Vehicle v1 = {1};
    enqueue(q, v1);
    assert(!isEmpty(q));
    assert(getSize(q) == 1);

    Vehicle v2 = dequeue(q);
    assert(v2.id == 1);
    assert(isEmpty(q));

    freeQueue(q);
    printf("Queue tests passed!\n");
}

int main() {
    test_queue();
    return 0;
}