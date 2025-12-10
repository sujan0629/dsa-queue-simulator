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
#define STEADY_INTERVAL 1

const char* lane_files[NUM_LANES] = {
    "data/lanea.txt",
    "data/laneb.txt",
    "data/lanec.txt",
    "data/laned.txt"
};

int main() {
    srand(time(NULL));
    int vehicle_id = 2000; // Different ID range

    printf("Traffic Generator 3: Steady mode started.\n");

    while (1) {
        for (int i = 0; i < NUM_LANES; i++) {
            FILE* fp = fopen(lane_files[i], "a");
            if (fp == NULL) {
                perror("Error opening file");
                return 1;
            }
            fprintf(fp, "%d\n", vehicle_id++);
            fclose(fp);
            printf("Steady: Added vehicle %d to lane %c\n", vehicle_id - 1, 'A' + i);
        }
        sleep(STEADY_INTERVAL);
    }

    return 0;
}
