#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#ifndef _WIN32
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#endif

#ifdef _WIN32
#include <windows.h>
#define sleep(x) Sleep(x * 1000)
#endif

#define NUM_LANES 4
#define INITIAL_VEHICLES 5
#define BASE_INTERVAL 2
#define PRIORITY_BOOST 1

const char* lane_files[NUM_LANES] = {
    "data/lanea.txt",
    "data/laneb.txt",
    "data/lanec.txt",
    "data/laned.txt"
};

int main() {
    srand(time(NULL));
    int vehicle_id = 1;

    // IPC: Create named pipe for communication
#ifndef _WIN32
    mkfifo("traffic_pipe", 0666);
#endif

    // Generate initial vehicles
    for (int i = 0; i < NUM_LANES; i++) {
        FILE* fp = fopen(lane_files[i], "w");
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

    // Continuously generate more vehicles with varying rates
    while (1) {
        int lane = rand() % NUM_LANES;
        if (lane != 0 && rand() % 10 < PRIORITY_BOOST) {
            lane = 0;
        }
        FILE* fp = fopen(lane_files[lane], "a");
        if (fp == NULL) {
            perror("Error opening file");
            return 1;
        }
        fprintf(fp, "%d\n", vehicle_id++);
        fclose(fp);
        printf("Added vehicle %d to lane %c\n", vehicle_id - 1, 'A' + lane);

        // IPC: Write to pipe
#ifndef _WIN32
        int fd = open("traffic_pipe", O_WRONLY);
        if (fd != -1) {
            char msg[50];
            sprintf(msg, "Vehicle %d to lane %c\n", vehicle_id - 1, 'A' + lane);
            write(fd, msg, strlen(msg));
            close(fd);
        }
#endif

        int sleep_time = BASE_INTERVAL + (rand() % 3);
        sleep(sleep_time);
    }

    return 0;
}
