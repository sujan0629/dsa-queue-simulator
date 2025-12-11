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

int main() {
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
    SDL_Color red = {255, 0, 0, 255};
    SDL_Color green = {0, 255, 0, 255};
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

        // Draw vehicles (placeholder)
        draw_vehicle(renderer, 60, 120);
        draw_vehicle(renderer, 210, 150);
        SDL_Delay(1000); // 1 second
        light_state = !light_state;
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}