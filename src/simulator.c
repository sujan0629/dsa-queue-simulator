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
#include <windows.h>
#pragma comment(lib, "ws2_32.lib")
#define sleep(x) Sleep(x * 1000)
#endif

#include <errno.h>

#ifdef _WIN32
typedef SOCKET sock_t;
#define CLOSE_SOCKET(s) closesocket(s)
#define SOCKET_ERRNO() WSAGetLastError()
#else
typedef int sock_t;
#define CLOSE_SOCKET(s) close(s)
#define SOCKET_ERRNO() errno
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

int main(int argc, char* argv[]) {
    // Initialize queues
    for (int i = 0; i < NUM_LANES; i++) {
        vehicle_queues[i] = createQueue();
    }

#ifdef _WIN32
    WSADATA wsa;
    WSAStartup(MAKEWORD(2,2), &wsa);
#endif

    sock_t server_sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;

    /* allow optional port via argv, default 8080 */
    int port = 8080;
    if (argc > 1) {
        int p = atoi(argv[1]);
        if (p > 0 && p < 65536) port = p;
    }
    server_addr.sin_port = htons(port);
#ifdef _WIN32
    if (server_sock == INVALID_SOCKET) {
        fprintf(stderr, "socket() failed, WSA error: %d\n", SOCKET_ERRNO());
        WSACleanup();
        return 1;
    }

    /* set SO_REUSEADDR so restarting quickly doesn't fail bind */
#ifdef _WIN32
    {
        char reuse = 1;
        setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    }
#else
    {
        int reuse = 1;
        setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    }
#endif

    if (bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
#ifdef _WIN32
        fprintf(stderr, "bind() failed on port %d, WSA error: %d\n", port, SOCKET_ERRNO());
#else
        fprintf(stderr, "bind() failed on port %d: ", port);
        perror("");
#endif
        CLOSE_SOCKET(server_sock);
#ifdef _WIN32
        WSACleanup();
#endif
        return 1;
    }

    if (listen(server_sock, 5) == SOCKET_ERROR) {
        fprintf(stderr, "listen() failed, WSA error: %d\n", SOCKET_ERRNO());
        CLOSE_SOCKET(server_sock);
        WSACleanup();
        return 1;
    }
    printf("Simulator listening on port %d\n", port);

    struct sockaddr_in client_addr;
    int client_len = sizeof(client_addr);
    sock_t client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &client_len);
    if (client_sock == INVALID_SOCKET) {
        fprintf(stderr, "Accept failed, WSA error: %d\n", SOCKET_ERRNO());
        CLOSE_SOCKET(server_sock);
        WSACleanup();
        return 1;
    }
#else
    if (server_sock < 0) {
        perror("socket() failed");
        return 1;
    }

    if (bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind() failed");
        CLOSE_SOCKET(server_sock);
        return 1;
    }

    if (listen(server_sock, 5) < 0) {
        perror("listen() failed");
        CLOSE_SOCKET(server_sock);
        return 1;
    }
    printf("Simulator listening on port 8080\n");

    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    sock_t client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &client_len);
    if (client_sock < 0) {
        perror("Accept failed");
        CLOSE_SOCKET(server_sock);
        return 1;
    }
#endif
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

    // Simulate polling and processing (optimized to 1s for responsiveness)
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
            // Write graphics state file so external renderer can display counts
            {
                FILE* gs = fopen("data/graphics_state.txt", "w");
                if (gs) {
                    for (int i = 0; i < NUM_LANES; i++) {
                        fprintf(gs, "%d\n", getSize(vehicle_queues[i]));
                    }
                    fclose(gs);
                }
            }
        }
    }

    // Cleanup
    for (int i = 0; i < NUM_LANES; i++) {
        freeQueue(vehicle_queues[i]);
    }
    if (log_fp) fclose(log_fp);
    closesocket(client_sock);
    closesocket(server_sock);
#ifdef _WIN32
    WSACleanup();
#endif
    return 0;
}
