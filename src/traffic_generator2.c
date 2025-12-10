#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#ifndef _WIN32
#include <unistd.h>
#endif

#ifdef _WIN32
#include <windows.h>
#define sleep(x) Sleep(x * 1000)
#endif

#define NUM_LANES 4
#define BURST_SIZE 5

const char* lane_files[NUM_LANES] = {
    "data/lanea.txt",
    "data/laneb.txt",
    "data/lanec.txt",
    "data/laned.txt"
};

int main() {
    srand(time(NULL));
    int vehicle_id = 1000; // Different ID range

    printf("Traffic Generator 2: Burst mode started.\n");

    while (1) {
        int lane = rand() % NUM_LANES;
        FILE* fp = fopen(lane_files[lane], "a");
        if (fp == NULL) {
            perror("Error opening file");
            return 1;
        }
        for (int b = 0; b < BURST_SIZE; b++) {
            fprintf(fp, "%d\n", vehicle_id++);
        }
        fclose(fp);
        printf("Burst: Added %d vehicles to lane %c (ID %d-%d)\n", BURST_SIZE, 'A' + lane, vehicle_id - BURST_SIZE, vehicle_id - 1);
        sleep(5); // Burst every 5 seconds
    }

    return 0;
}
