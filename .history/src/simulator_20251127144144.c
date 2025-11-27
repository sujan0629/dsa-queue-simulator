#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "queue.h"

#ifdef _WIN32
#include <windows.h>
#define sleep(x) Sleep(x * 1000)
#endif

#define NUM_LANES 4

const char* lane_files[NUM_LANES] = {
    "data/lanea.txt",
    "data/laneb.txt",
    "data/lanec.txt",
    "data/laned.txt"
};

Queue* vehicle_queues[NUM_LANES];

void load_vehicles_from_file(int lane_index) {
    FILE* fp = fopen(lane_files[lane_index], "r");
    if (fp == NULL) {
        perror("Error opening file");
        return;
    }
    char line[256];
    while (fgets(line, sizeof(line), fp)) {
        int id;
        if (sscanf(line, "%d", &id) == 1) {
            Vehicle v = {id};
            enqueue(vehicle_queues[lane_index], v);
        }
    }
    fclose(fp);
}

int main() {
    // Initialize queues
    for (int i = 0; i < NUM_LANES; i++) {
        vehicle_queues[i] = createQueue();
    }

    // Load initial vehicles
    for (int i = 0; i < NUM_LANES; i++) {
        load_vehicles_from_file(i);
    }

    printf("Initial load complete.\n");
    for (int i = 0; i < NUM_LANES; i++) {
        printf("Lane %c: %d vehicles\n", 'A' + i, getSize(vehicle_queues[i]));
    }

    // Simulate polling
    while (1) {
        sleep(5); // Poll every 5 seconds
        for (int i = 0; i < NUM_LANES; i++) {
            load_vehicles_from_file(i); // Reload, but this will duplicate if not cleared
        }
        printf("Updated queues:\n");
        for (int i = 0; i < NUM_LANES; i++) {
            printf("Lane %c: %d vehicles\n", 'A' + i, getSize(vehicle_queues[i]));
        }
        // TODO: Process vehicles based on priority
    }

    // Cleanup
    for (int i = 0; i < NUM_LANES; i++) {
        freeQueue(vehicle_queues[i]);
    }

    return 0;
}
