// TODO: Optimize by using threads for concurrent processing
// Example: pthread_create for light timer and vehicle processing
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef _WIN32
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif
#include "queue.h"

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#define sleep(x) Sleep(x * 1000)
#endif

typedef enum {
    RED,
    GREEN
} LightState;

#define NUM_LANES 4
#define PRIORITY_LANE 0  // AL2 is lane A (index 0)
#define GREEN_TIME 10    // seconds for green light
#define RED_TIME 5       // seconds for red light
#define VEHICLE_PASS_TIME 2  // seconds per vehicle

int estimate_pass_time(int vehicles) {
    return vehicles * VEHICLE_PASS_TIME;
}

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

#ifdef _WIN32
    WSADATA wsa;
    WSAStartup(MAKEWORD(2,2), &wsa);
#endif

    int server_sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(8080);

    bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr));
    listen(server_sock, 5);
    printf("Simulator listening on port 8080\n");

    int client_sock = accept(server_sock, NULL, NULL);
    if (client_sock < 0) {
        perror("Accept failed");
        close(server_sock);
        return 1;
    }
    printf("Generator connected.\n");

    FILE* log_fp = fopen("simulation_log.txt", "a");
    if (log_fp) {
        fprintf(log_fp, "Simulation started at %s\n", __DATE__ " " __TIME__);
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

    // Traffic light state
    LightState current_light = GREEN;
    int light_timer = GREEN_TIME;

    // Simulate polling and processing
    while (1) {
        sleep(1); // Poll every 1 second for finer control

        // Socket: Read from client
        char buffer[256];
        int bytes = recv(client_sock, buffer, sizeof(buffer) - 1, 0);
        if (bytes > 0) {
            buffer[bytes] = '\0';
            printf("Socket: %s", buffer);
        }

        // Update light timer
        light_timer--;
        if (light_timer <= 0) {
            if (current_light == GREEN) {
                current_light = RED;
                light_timer = RED_TIME;
                printf("Light turned RED\n");
            } else {
                current_light = GREEN;
                light_timer = GREEN_TIME;
                printf("Light turned GREEN\n");
            }
        }

        // Load any new vehicles appended by generator and truncate
        for (int i = 0; i < NUM_LANES; i++) {
            load_vehicles_from_file(i);
            FILE* tf = fopen(lane_files[i], "w");
            if (tf) fclose(tf);
        }

        // Process vehicles only when light is green
        if (current_light == GREEN) {
            // Detect priority lane: only AL2 (lane A) can be priority if >10 vehicles
            if (priority_lane == -1) {
                if (getSize(vehicle_queues[PRIORITY_LANE]) > 10) {
                    priority_lane = PRIORITY_LANE;
                    printf("Priority lane detected: %c (size=%d)\n", 'A' + PRIORITY_LANE, getSize(vehicle_queues[PRIORITY_LANE]));
                }
            }

            // If we have a priority lane, serve it until size < 5
            if (priority_lane != -1) {
                if (!isEmpty(vehicle_queues[priority_lane])) {
                    Vehicle v = dequeue(vehicle_queues[priority_lane]);
                    printf("Vehicle %d passed from priority lane %c\n", v.id, 'A' + priority_lane);
                }
                if (getSize(vehicle_queues[priority_lane]) < 5) {
                    printf("Priority lane %c dropped below 5, returning to normal scheduling\n", 'A' + priority_lane);
                    priority_lane = -1;
                }
            } else {
                // Normal scheduling: serve proportionally as per formula |V| = (1/n) * sum Li
                int total_vehicles = 0;
                for (int i = 0; i < NUM_LANES; i++) total_vehicles += getSize(vehicle_queues[i]);
                int n = NUM_LANES;
                int vehicles_to_serve = total_vehicles / n;
                if (vehicles_to_serve < 1 && total_vehicles > 0) vehicles_to_serve = 1;

                int estimated_time = estimate_pass_time(vehicles_to_serve);
                printf("Estimated pass time for %d vehicles: %d seconds\n", vehicles_to_serve, estimated_time); // ensure progress

                // Distribute proportionally, but simplified to round-robin for now
                int served = 0;
                for (int attempt = 0; attempt < NUM_LANES && served < vehicles_to_serve; attempt++) {
                    int i = attempt % NUM_LANES;
                    if (!isEmpty(vehicle_queues[i])) {
                        Vehicle v = dequeue(vehicle_queues[i]);
                        printf("Vehicle %d passed from lane %c\n", v.id, 'A' + i);
                        served++;
                    }
                }
            }
        }

        // Status every 5 seconds
        static int status_timer = 0;
        status_timer++;
        if (status_timer >= 5) {
            status_timer = 0;
            printf("Light: %s (%d sec left), Queues:\n", current_light == GREEN ? "GREEN" : "RED", light_timer);
            for (int i = 0; i < NUM_LANES; i++) {
                printf("Lane %c: %d vehicles\n", 'A' + i, getSize(vehicle_queues[i]));
                if (log_fp) {
                    fprintf(log_fp, "Lane %c: %d vehicles\n", 'A' + i, getSize(vehicle_queues[i]));
                    fflush(log_fp);
                }
            }
        }
    }

    // Cleanup
    for (int i = 0; i < NUM_LANES; i++) {
        freeQueue(vehicle_queues[i]);
    }
    if (log_fp) fclose(log_fp);
    close(client_sock);
    close(server_sock);
#ifdef _WIN32
    WSACleanup();
#endif
    return 0;
}
