#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef _WIN32
#include <unistd.h>
#endif
#include "queue.h"

#ifdef _WIN32
#include <windows.h>
#define sleep(x) Sleep(x * 1000)
#endif

#define NUM_LANES 4
#define PRIORITY_LANE 0  // AL2 is lane A (index 0)

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

    // Load initial vehicles and truncate the files so generator won't duplicate entries
    for (int i = 0; i < NUM_LANES; i++) {
        load_vehicles_from_file(i);
        // truncate file after loading to indicate we've consumed entries
        FILE* tf = fopen(lane_files[i], "w");
        if (tf) fclose(tf);
    }

    printf("Initial load complete.\n");
    for (int i = 0; i < NUM_LANES; i++) {
        printf("Lane %c: %d vehicles\n", 'A' + i, getSize(vehicle_queues[i]));
    }

    // Simple state for priority lane (-1 means none)
    int priority_lane = -1;

    // Simulate polling and processing
    while (1) {
        sleep(5); // Poll every 5 seconds

        // Load any new vehicles appended by generator and truncate
        for (int i = 0; i < NUM_LANES; i++) {
            load_vehicles_from_file(i);
            FILE* tf = fopen(lane_files[i], "w");
            if (tf) fclose(tf);
        }

        // Detect priority lane: only AL2 (lane A) can be priority if >10 vehicles
        if (priority_lane == -1) {
            if (getSize(vehicle_queues[PRIORITY_LANE]) > 10) {
                priority_lane = PRIORITY_LANE;
                printf("Priority lane detected: %c (size=%d)\n", 'A' + PRIORITY_LANE, getSize(vehicle_queues[PRIORITY_LANE]));
            }
        }

        // If we have a priority lane, serve it until size < 5
        if (priority_lane != -1) {
            int serve_count = 2; // number of vehicles to serve in this tick for priority
            printf("Serving priority lane %c: serving up to %d vehicles\n", 'A' + priority_lane, serve_count);
            for (int s = 0; s < serve_count && !isEmpty(vehicle_queues[priority_lane]); s++) {
                Vehicle v = dequeue(vehicle_queues[priority_lane]);
                printf("Vehicle %d passed from lane %c\n", v.id, 'A' + priority_lane);
            }
            if (getSize(vehicle_queues[priority_lane]) < 5) {
                printf("Priority lane %c dropped below 5, returning to normal scheduling\n", 'A' + priority_lane);
                priority_lane = -1;
            }
        } else {
            // Normal scheduling: serve each lane proportionally
            int total = 0;
            for (int i = 0; i < NUM_LANES; i++) total += getSize(vehicle_queues[i]);
            int n = NUM_LANES;
            int serve_each = (n == 0) ? 0 : (total / n);
            if (serve_each < 1) serve_each = 1; // ensure progress

            for (int i = 0; i < NUM_LANES; i++) {
                int to_serve = serve_each;
                if (isEmpty(vehicle_queues[i])) continue;
                printf("Serving lane %c: up to %d vehicles\n", 'A' + i, to_serve);
                for (int s = 0; s < to_serve && !isEmpty(vehicle_queues[i]); s++) {
                    Vehicle v = dequeue(vehicle_queues[i]);
                    printf("Vehicle %d passed from lane %c\n", v.id, 'A' + i);
                }
            }
        }

        // Status
        printf("Updated queues:\n");
        for (int i = 0; i < NUM_LANES; i++) {
            printf("Lane %c: %d vehicles\n", 'A' + i, getSize(vehicle_queues[i]));
        }
    }

    // Cleanup
    for (int i = 0; i < NUM_LANES; i++) {
        freeQueue(vehicle_queues[i]);
    }

    return 0;
}
