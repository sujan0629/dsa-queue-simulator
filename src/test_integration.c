#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "queue.h"

void test_integration() {
    printf("Running integration tests...\n");

    // Test queue operations
    Queue* q = createQueue();
    for (int i = 1; i <= 10; i++) {
        Vehicle v = {i};
        enqueue(q, v);
    }
    assert(getSize(q) == 10);

    for (int i = 1; i <= 5; i++) {
        Vehicle v = dequeue(q);
        assert(v.id == i);
    }
    assert(getSize(q) == 5);

    freeQueue(q);
    printf("Integration tests passed!\n");
}

int main() {
    test_integration();
    return 0;
}