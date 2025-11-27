#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#ifndef _WIN32
#include <unistd.h> // for sleep on Unix
#endif

#ifdef _WIN32
#include <windows.h>
#define sleep(x) Sleep(x * 1000)
#endif

#define NUM_LANES 4
#define INITIAL_VEHICLES 5

const char* lane_files[NUM_LANES] = {
    "data/lanea.txt",
    "data/laneb.txt",
    "data/lanec.txt",
    "data/laned.txt"
};

int main() {
    srand(time(NULL));
    int vehicle_id = 1;

    // Generate initial vehicles
    for (int i = 0; i < NUM_LANES; i++) {
        FILE* fp = fopen(lane_files[i], "w"); // Start fresh
        if (fp == NULL) {
            perror("Error opening file");
            return 1;
        }
        for (int j = 0; j < INITIAL_VEHICLES; j++) {
            fprintf(fp, "%d\n", vehicle_id++);
        }
        fclose(fp);
    }

    printf("Initial vehicles generated.\n");

    // Continuously generate more vehicles
    while (1) {
        int lane = rand() % NUM_LANES;
        FILE* fp = fopen(lane_files[lane], "a");
        if (fp == NULL) {
            perror("Error opening file");
            return 1;
        }
        fprintf(fp, "%d\n", vehicle_id++);
        fclose(fp);
        printf("Added vehicle %d to lane %c\n", vehicle_id - 1, 'A' + lane);
        sleep(2); // Wait 2 seconds
    }

    return 0;
}
