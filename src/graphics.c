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

        // Draw lanes
        for (int i = 0; i < 4; i++) {
            draw_lane(renderer, 50 + i * 150, 100, LANE_WIDTH, LANE_HEIGHT);
        }

        // Read graphics state (lane vehicle counts) from file written by simulator
        int lane_counts[4] = {0,0,0,0};
        FILE* gs = fopen("data/graphics_state.txt", "r");
        if (gs) {
            for (int i = 0; i < 4; i++) {
                if (fscanf(gs, "%d", &lane_counts[i]) != 1) lane_counts[i] = 0;
            }
            fclose(gs);
        }

        // Draw vehicles according to lane_counts (simple vertical stacking)
        for (int lane = 0; lane < 4; lane++) {
            int count = lane_counts[lane];
            if (count > 20) count = 20; // cap visible vehicles
            int base_x = 50 + lane * 150 + 10;
            int base_y = 120; // top of lane
            int spacing = 18;
            for (int v = 0; v < count; v++) {
                int vy = base_y + v * spacing;
                draw_vehicle(renderer, base_x, vy);
            }
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