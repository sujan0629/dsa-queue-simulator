#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef _WIN32
#include <unistd.h>
#endif

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

int main() {
    FILE* log_fp = fopen("simulation_log.txt", "w");
    if (log_fp == NULL) {
        perror("Log file open failed");
        return 1;
    }

    printf("Receiver 2: Logging mode started.\n");
    fprintf(log_fp, "Simulation Log Started\n");

    while (1) {
        for (int i = 0; i < NUM_LANES; i++) {
            FILE* fp = fopen(lane_files[i], "r");
            if (fp) {
                char line[256];
                int count = 0;
                while (fgets(line, sizeof(line), fp)) {
                    count++;
                }
                fclose(fp);
                printf("Lane %c: %d vehicles\n", 'A' + i, count);
                fprintf(log_fp, "Lane %c: %d vehicles\n", 'A' + i, count);
                fflush(log_fp);
            }
        }
        sleep(10);
    }

    fclose(log_fp);
    return 0;
}
