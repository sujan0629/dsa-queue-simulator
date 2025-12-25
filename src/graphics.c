#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define LANE_WIDTH 100
#define LANE_HEIGHT 400

void draw_lane(SDL_Renderer* renderer, int x, int y, int width, int height) {
    SDL_Rect rect = {x, y, width, height};
    SDL_SetRenderDrawColor(renderer, 128, 128, 128, 255); // Gray
    SDL_RenderFillRect(renderer, &rect);
}

void draw_vehicle(SDL_Renderer* renderer, int x, int y) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255); // Blue
    SDL_Rect rect = {x, y, 20, 10};
    SDL_RenderFillRect(renderer, &rect);
}

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL init failed: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("DSA Traffic Simulator", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        printf("Window creation failed: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        printf("Renderer creation failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    int running = 1;
    SDL_Event event;
    int light_state = 0; // 0 red, 1 green

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = 0;
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // Draw cross/junction lanes
        // North lane (A)
        draw_lane(renderer, WINDOW_WIDTH/2 - LANE_WIDTH/2, 0, LANE_WIDTH, WINDOW_HEIGHT/2 - 40);
        // South lane (B)
        draw_lane(renderer, WINDOW_WIDTH/2 - LANE_WIDTH/2, WINDOW_HEIGHT/2 + 40, LANE_WIDTH, WINDOW_HEIGHT/2 - 40);
        // West lane (D)
        draw_lane(renderer, 0, WINDOW_HEIGHT/2 - LANE_WIDTH/2, WINDOW_WIDTH/2 - 40, LANE_WIDTH);
        // East lane (C)
        draw_lane(renderer, WINDOW_WIDTH/2 + 40, WINDOW_HEIGHT/2 - LANE_WIDTH/2, WINDOW_WIDTH/2 - 40, LANE_WIDTH);

        // Read graphics state (lane vehicle counts) from file written by simulator
        int lane_counts[4] = {0,0,0,0};
        FILE* gs = fopen("data/graphics_state.txt", "r");
        if (gs) {
            for (int i = 0; i < 4; i++) {
                if (fscanf(gs, "%d", &lane_counts[i]) != 1) lane_counts[i] = 0;
            }
            fclose(gs);
        }

        // Draw vehicles for each lane, moving toward the center
        int center_x = WINDOW_WIDTH/2;
        int center_y = WINDOW_HEIGHT/2;
        int spacing = 22;
        // Lane 0: North (A)
        for (int v = 0; v < lane_counts[0] && v < 15; v++) {
            int vy = center_y - 40 - (v+1)*spacing;
            draw_vehicle(renderer, center_x - 10, vy);
        }
        // Lane 1: South (B)
        for (int v = 0; v < lane_counts[1] && v < 15; v++) {
            int vy = center_y + 40 + v*spacing;
            draw_vehicle(renderer, center_x - 10, vy);
        }
        // Lane 2: East (C)
        for (int v = 0; v < lane_counts[2] && v < 15; v++) {
            int vx = center_x + 40 + v*spacing;
            draw_vehicle(renderer, vx, center_y - 5);
        }
        // Lane 3: West (D)
        for (int v = 0; v < lane_counts[3] && v < 15; v++) {
            int vx = center_x - 40 - (v+1)*spacing;
            draw_vehicle(renderer, vx, center_y - 5);
        }

        // Present the rendered frame
        SDL_RenderPresent(renderer);

        // Short delay for smoother updates and to pick up file changes
        SDL_Delay(150); // 150 ms
        light_state = !light_state;
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}