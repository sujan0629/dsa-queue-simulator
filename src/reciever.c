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
    printf("Receiver started: monitoring lane files...\n");
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
                printf("Lane %c: %d vehicles waiting\n", 'A' + i, count);
            }
        }
        sleep(10); // Check every 10 seconds
    }
    return 0;
}
