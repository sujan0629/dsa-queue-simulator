#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#ifndef _WIN32
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
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

#ifdef _WIN32
    WSADATA wsa;
    WSAStartup(MAKEWORD(2,2), &wsa);
#endif

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connect failed");
        return 1;
    }

    printf("Connected to simulator.\n");

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

        // Socket: Send message
        char msg[50];
        sprintf(msg, "Vehicle %d to lane %c\n", vehicle_id - 1, 'A' + lane);
        send(sock, msg, strlen(msg), 0);

        int sleep_time = BASE_INTERVAL + (rand() % 3);
        sleep(sleep_time);
    }

    return 0;
}
