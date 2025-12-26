#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// --- CONFIGURATION ---
#define WINDOW_WIDTH 1000
#define WINDOW_HEIGHT 800

// STRICT GEOMETRY
// We use 3 lanes IN and 3 lanes OUT per road to prevent head-on collisions.
// However, visually they are grouped as one single road block.
#define LANE_WIDTH 25
#define LANES_PER_DIR 3
#define ROAD_HALF_WIDTH (LANES_PER_DIR * LANE_WIDTH) // 75px
#define ROAD_FULL_WIDTH (ROAD_HALF_WIDTH * 2)        // 150px
#define INTERSECTION_SIZE ROAD_FULL_WIDTH

#define VEHICLE_W 18
#define VEHICLE_L 30
#define SPEED 2.5f
#define TURN_SPEED 0.04f // Radians per frame

// SAFE DISTANCE (Braking logic)
#define STOP_DISTANCE 40
#define FOLLOW_DISTANCE 50

typedef enum { DIR_N, DIR_S, DIR_E, DIR_W } Direction;
typedef enum { LIGHT_RED, LIGHT_GREEN, LIGHT_YELLOW } LightState;
typedef enum { TURN_LEFT, TURN_STRAIGHT, TURN_RIGHT } TurnIntent;
typedef enum { STATE_SPAWN, STATE_DRIVE, STATE_BRAKE, STATE_TURN, STATE_EXIT } CarState;

// --- STRUCTURES ---

typedef struct {
    float x, y;
    float angle;      // Radians
    Direction dir;    // Origin direction
    int lane;         // 0=Left, 1=Center, 2=Right
    TurnIntent intent;
    CarState state;
    
    // Turning Math
    float pivot_x, pivot_y;
    float radius;
    float target_angle;
    float current_arc_angle;
    
    int active;
} Vehicle;

// --- GLOBALS ---
Vehicle vehicles[200];
int frame = 0;
LightState light_NS = LIGHT_GREEN;
LightState light_EW = LIGHT_RED;

// --- HELPER MATH ---

// Get the center coordinate of a specific lane relative to the road center
float get_lane_center(int lane_idx) {
    // lane_idx 0 is closest to center line, 2 is closest to curb
    return (lane_idx * LANE_WIDTH) + (LANE_WIDTH / 2.0f);
}

// Convert direction enum to angle
float dir_to_angle(Direction d) {
    switch(d) {
        case DIR_N: return -M_PI / 2.0f; // Up
        case DIR_S: return M_PI / 2.0f;  // Down
        case DIR_E: return 0.0f;         // Right
        case DIR_W: return M_PI;         // Left
    }
    return 0;
}

// Check collision with other cars
int is_blocked(int self_idx) {
    Vehicle* v = &vehicles[self_idx];
    
    for(int i=0; i<200; i++) {
        if(i == self_idx || !vehicles[i].active) continue;
        Vehicle* other = &vehicles[i];
        
        // Simple distance check
        float dx = v->x - other->x;
        float dy = v->y - other->y;
        float dist = sqrt(dx*dx + dy*dy);
        
        // Check if other car is IN FRONT relative to our angle
        // (Dot product or simple logic based on direction)
        if(dist < FOLLOW_DISTANCE) {
            float angle_to_other = atan2(other->y - v->y, other->x - v->x);
            float angle_diff = fabs(angle_to_other - v->angle);
            while(angle_diff > M_PI) angle_diff -= 2*M_PI;
            
            // If strictly in front (within 45 degrees)
            if(fabs(angle_diff) < 0.8f) return 1;
        }
    }
    return 0;
}

// --- LOGIC ---

void spawn_vehicle() {
    int idx = -1;
    for(int i=0; i<200; i++) { if(!vehicles[i].active) { idx = i; break; } }
    if(idx == -1) return;

    Vehicle* v = &vehicles[idx];
    v->active = 1;
    v->dir = rand() % 4;
    v->lane = rand() % 3; // 0, 1, 2
    v->state = STATE_DRIVE;
    
    // STRICT LANE RULES
    if(v->lane == 0) v->intent = TURN_LEFT;
    else if(v->lane == 1) v->intent = TURN_STRAIGHT;
    else v->intent = TURN_RIGHT;

    float cx = WINDOW_WIDTH / 2.0f;
    float cy = WINDOW_HEIGHT / 2.0f;
    float offset = get_lane_center(v->lane); // Offset from center line

    // Setup Start Position strictly in lane
    switch(v->dir) {
        case DIR_N: // Going North (Start Bottom)
            v->x = cx + offset; 
            v->y = WINDOW_HEIGHT + 50;
            v->angle = -M_PI/2;
            break;
        case DIR_S: // Going South (Start Top)
            v->x = cx - offset; 
            v->y = -50;
            v->angle = M_PI/2;
            break;
        case DIR_E: // Going East (Start Left)
            v->x = -50; 
            v->y = cy + offset;
            v->angle = 0;
            break;
        case DIR_W: // Going West (Start Right)
            v->x = WINDOW_WIDTH + 50; 
            v->y = cy - offset;
            v->angle = M_PI;
            break;
    }
}

void update_traffic_lights() {
    int cycle = frame % 1200; // 20 seconds cycle approx
    
    // Logic: 0-500 NS Green, 500-600 NS Yellow, 600-1100 EW Green, 1100-1200 EW Yellow
    if(cycle < 500) { light_NS = LIGHT_GREEN; light_EW = LIGHT_RED; }
    else if(cycle < 600) { light_NS = LIGHT_YELLOW; light_EW = LIGHT_RED; }
    else if(cycle < 1100) { light_NS = LIGHT_RED; light_EW = LIGHT_GREEN; }
    else { light_NS = LIGHT_RED; light_EW = LIGHT_YELLOW; }
}

void update_vehicles() {
    float cx = WINDOW_WIDTH / 2.0f;
    float cy = WINDOW_HEIGHT / 2.0f;
    float stop_boundary = INTERSECTION_SIZE / 2.0f;

    for(int i=0; i<200; i++) {
        Vehicle* v = &vehicles[i];
        if(!v->active) continue;

        // 1. BRAKING LOGIC (Traffic Lights)
        int approaching_light = 0;
        float dist_to_center = 0;

        // Calculate distance to intersection center
        if(v->dir == DIR_N) dist_to_center = v->y - cy;
        if(v->dir == DIR_S) dist_to_center = cy - v->y;
        if(v->dir == DIR_E) dist_to_center = cx - v->x;
        if(v->dir == DIR_W) dist_to_center = v->x - cx;

        // Check Light Color
        LightState my_light = (v->dir == DIR_N || v->dir == DIR_S) ? light_NS : light_EW;

        // If near stop line and light is not Green
        if(v->state != STATE_TURN && v->state != STATE_EXIT) {
            if(dist_to_center > 0 && dist_to_center < stop_boundary + STOP_DISTANCE) {
                if(my_light != LIGHT_GREEN) {
                    approaching_light = 1;
                }
            }
        }

        // 2. COLLISION CHECK
        int blocked = is_blocked(i);

        // 3. STATE UPDATES
        if(blocked || approaching_light) {
            v->state = STATE_BRAKE;
        } else if (v->state == STATE_BRAKE) {
            v->state = STATE_DRIVE;
        }

        // 4. INITIATE TURN (Mathematical Pivot Calculation)
        // Trigger turn when we hit the stop line/entrance of intersection
        if(v->state == STATE_DRIVE && dist_to_center <= stop_boundary + 5 && dist_to_center >= stop_boundary - 5) {
            if(v->intent != TURN_STRAIGHT) {
                v->state = STATE_TURN;
                
                float lane_offset = get_lane_center(v->lane);
                
                if(v->intent == TURN_RIGHT) {
                    // Pivot is the close corner. 
                    // Radius = Lane Offset from center line + Lane Width/2 (No, simpler: Distance from center to lane center)
                    // Radius = (Road_Half_Width) - Lane_Offset
                    // Actually, simpler math:
                    // If turning Right, Pivot is (Corner X, Corner Y)
                    
                    v->radius = ROAD_HALF_WIDTH - lane_offset; // Inner lanes have larger radius relative to corner? No.
                    // Lane 2 (Rightmost) has smallest radius.
                    // Lane 0 (Leftmost) has largest radius... wait, Lane 0 turns LEFT.
                    // Lane 2 turns RIGHT. Lane 2 is outer.
                    // Radius = 75 - (2*25 + 12.5) = 12.5 (very tight).
                    
                    // Let's calculate PIVOT coordinates
                    if(v->dir == DIR_N) { v->pivot_x = cx + ROAD_HALF_WIDTH; v->pivot_y = cy + ROAD_HALF_WIDTH; v->current_arc_angle = M_PI; }
                    if(v->dir == DIR_S) { v->pivot_x = cx - ROAD_HALF_WIDTH; v->pivot_y = cy - ROAD_HALF_WIDTH; v->current_arc_angle = 0; }
                    if(v->dir == DIR_E) { v->pivot_x = cx - ROAD_HALF_WIDTH; v->pivot_y = cy + ROAD_HALF_WIDTH; v->current_arc_angle = -M_PI/2; }
                    if(v->dir == DIR_W) { v->pivot_x = cx + ROAD_HALF_WIDTH; v->pivot_y = cy - ROAD_HALF_WIDTH; v->current_arc_angle = M_PI/2; }
                } 
                else if(v->intent == TURN_LEFT) {
                    // Pivot is the FAR corner
                    v->radius = ROAD_HALF_WIDTH + lane_offset;
                    
                    if(v->dir == DIR_N) { v->pivot_x = cx - ROAD_HALF_WIDTH; v->pivot_y = cy + ROAD_HALF_WIDTH; v->current_arc_angle = 0; }
                    if(v->dir == DIR_S) { v->pivot_x = cx + ROAD_HALF_WIDTH; v->pivot_y = cy - ROAD_HALF_WIDTH; v->current_arc_angle = M_PI; }
                    if(v->dir == DIR_E) { v->pivot_x = cx - ROAD_HALF_WIDTH; v->pivot_y = cy - ROAD_HALF_WIDTH; v->current_arc_angle = M_PI/2; }
                    if(v->dir == DIR_W) { v->pivot_x = cx + ROAD_HALF_WIDTH; v->pivot_y = cy + ROAD_HALF_WIDTH; v->current_arc_angle = 3*M_PI/2; }
                }
            } else {
                v->state = STATE_EXIT; // Drive straight through
            }
        }

        // 5. PHYSICS APPLICATION
        if(v->state == STATE_BRAKE) {
            // Do nothing, stopped
        } 
        else if(v->state == STATE_TURN) {
            // Move along ARC
            if(v->intent == TURN_RIGHT) {
                v->current_arc_angle -= TURN_SPEED; // Clockwise
                v->angle -= TURN_SPEED;
            } else {
                v->current_arc_angle += TURN_SPEED; // Counter-Clockwise
                v->angle += TURN_SPEED;
            }
            
            v->x = v->pivot_x + v->radius * cos(v->current_arc_angle);
            v->y = v->pivot_y + v->radius * sin(v->current_arc_angle);

            // Check if turn done (90 degrees moved)
            // Simplified check: if angle matches target
            // OR simply check if we have aligned with grid
            float abs_sin = fabs(sin(v->angle));
            float abs_cos = fabs(cos(v->angle));
            if( (abs_sin < 0.05 || abs_cos < 0.05) && (fabs(v->x - cx) > 10 || fabs(v->y - cy) > 10) ) {
                v->state = STATE_EXIT;
                // Snap to axis
                if(abs_sin < 0.05) v->angle = (cos(v->angle) > 0) ? 0 : M_PI;
                else v->angle = (sin(v->angle) > 0) ? M_PI/2 : -M_PI/2;
            }
        } 
        else {
            // Drive Straight (Vector)
            v->x += cos(v->angle) * SPEED;
            v->y += sin(v->angle) * SPEED;
        }

        // 6. DESPAWN
        if(v->x < -100 || v->x > WINDOW_WIDTH+100 || v->y < -100 || v->y > WINDOW_HEIGHT+100) {
            v->active = 0;
        }
    }
}

// --- RENDER ---

void draw_minimal_road(SDL_Renderer* ren) {
    // 4. Draw intersection center dot (very subtle)
    SDL_SetRenderDrawColor(ren, 180, 180, 200, 180);
    int dot_r = 7;
    int dot_cx = WINDOW_WIDTH / 2;
    int dot_cy = WINDOW_HEIGHT / 2;
    for(int w=0; w<dot_r*2; w++) {
        for(int h=0; h<dot_r*2; h++) {
            int dx = w - dot_r;
            int dy = h - dot_r;
            if(dx*dx + dy*dy <= dot_r*dot_r) {
                SDL_RenderDrawPoint(ren, dot_cx + dx, dot_cy + dy);
            }
        }
    }
    // 1. Draw Road Shoulder (Even Lighter Gray)
    SDL_SetRenderDrawColor(ren, 50, 50, 50, 255); // Shoulder color
    SDL_Rect v_shoulder = { (WINDOW_WIDTH - ROAD_FULL_WIDTH)/2 - 6, 0, ROAD_FULL_WIDTH + 12, WINDOW_HEIGHT };
    SDL_Rect h_shoulder = { 0, (WINDOW_HEIGHT - ROAD_FULL_WIDTH)/2 - 6, WINDOW_WIDTH, ROAD_FULL_WIDTH + 12 };
    SDL_RenderFillRect(ren, &v_shoulder);
    SDL_RenderFillRect(ren, &h_shoulder);

    // 2. Draw Road Base (Subtle Lighter Gray Asphalt)
    SDL_SetRenderDrawColor(ren, 60, 60, 60, 255); // Slightly lighter, subtle blue-gray
    SDL_Rect v_road = { (WINDOW_WIDTH - ROAD_FULL_WIDTH)/2, 0, ROAD_FULL_WIDTH, WINDOW_HEIGHT };
    SDL_Rect h_road = { 0, (WINDOW_HEIGHT - ROAD_FULL_WIDTH)/2, WINDOW_WIDTH, ROAD_FULL_WIDTH };
    SDL_RenderFillRect(ren, &v_road);
    SDL_RenderFillRect(ren, &h_road);

    // 2. Intersection Box (Slightly lighter to see overlapping)
    SDL_SetRenderDrawColor(ren, 80, 80, 80, 255);
    SDL_Rect intersection = { (WINDOW_WIDTH - ROAD_FULL_WIDTH)/2, (WINDOW_HEIGHT - ROAD_FULL_WIDTH)/2, ROAD_FULL_WIDTH, ROAD_FULL_WIDTH };
    SDL_RenderFillRect(ren, &intersection);

    // 3. Draw Minimal Crosswalks (Solid white stripes like old)
    SDL_SetRenderDrawColor(ren, 240, 240, 240, 255); // Lighter white
    int stripe_width = 8;
    int stripe_spacing = 12;
    int cx = WINDOW_WIDTH / 2;
    int cy = WINDOW_HEIGHT / 2;
    int inter_size = INTERSECTION_SIZE;
    // Top crosswalk (inside intersection)
    for (int i = cx - inter_size/2 + 10; i < cx + inter_size/2 - 10; i += stripe_spacing) {
        SDL_Rect stripe = {i, cy - inter_size/2 + 5, stripe_width, 15};
        SDL_RenderFillRect(ren, &stripe);
    }
    // Bottom crosswalk (inside intersection)
    for (int i = cx - inter_size/2 + 10; i < cx + inter_size/2 - 10; i += stripe_spacing) {
        SDL_Rect stripe = {i, cy + inter_size/2 - 20, stripe_width, 15};
        SDL_RenderFillRect(ren, &stripe);
    }
    // Left crosswalk (inside intersection)
    for (int i = cy - inter_size/2 + 10; i < cy + inter_size/2 - 10; i += stripe_spacing) {
        SDL_Rect stripe = {cx - inter_size/2 + 5, i, 15, stripe_width};
        SDL_RenderFillRect(ren, &stripe);
    }
    // Right crosswalk (inside intersection)
    for (int i = cy - inter_size/2 + 10; i < cy + inter_size/2 - 10; i += stripe_spacing) {
        SDL_Rect stripe = {cx + inter_size/2 - 20, i, 15, stripe_width};
        SDL_RenderFillRect(ren, &stripe);
    }

    // 3. Markings (Minimal White Lines)
    SDL_SetRenderDrawColor(ren, 200, 200, 200, 255);
    
    // cx and cy already declared above
    
    // Draw Center Lines (Solid)
    // Vertical
    SDL_RenderDrawLine(ren, cx, 0, cx, cy - ROAD_HALF_WIDTH);
    SDL_RenderDrawLine(ren, cx, cy + ROAD_HALF_WIDTH, cx, WINDOW_HEIGHT);
    // Horizontal
    SDL_RenderDrawLine(ren, 0, cy, cx - ROAD_HALF_WIDTH, cy);
    SDL_RenderDrawLine(ren, cx + ROAD_HALF_WIDTH, cy, WINDOW_WIDTH, cy);
    
    // Draw Lane Dividers (Dotted)
    // We have 3 lanes per side. So lines at offset 25 and 50.
    for(int i=1; i<3; i++) {
        int off = i * LANE_WIDTH;
        
        // Vertical Incoming/Outgoing
        for(int y=0; y<WINDOW_HEIGHT; y+=20) {
            if(y > cy - ROAD_HALF_WIDTH && y < cy + ROAD_HALF_WIDTH) continue; // Skip intersection
            SDL_RenderDrawLine(ren, cx - off, y, cx - off, y+10);
            SDL_RenderDrawLine(ren, cx + off, y, cx + off, y+10);
        }
        
        // Horizontal Incoming/Outgoing
        for(int x=0; x<WINDOW_WIDTH; x+=20) {
            if(x > cx - ROAD_HALF_WIDTH && x < cx + ROAD_HALF_WIDTH) continue;
            SDL_RenderDrawLine(ren, x, cy - off, x+10, cy - off);
            SDL_RenderDrawLine(ren, x, cy + off, x+10, cy + off);
        }
    }
}

void draw_traffic_lights(SDL_Renderer* ren) {
    // Positions for lights
    int cx = WINDOW_WIDTH / 2;
    int cy = WINDOW_HEIGHT / 2;

    // Helper to draw a traffic light
    void draw_traffic_light(int x, int y, LightState s) {
        // Black pole, taller and thicker
        SDL_Rect pole = {x-3, y-25, 6, 50};
        SDL_SetRenderDrawColor(ren, 50, 50, 50, 255); // Lighter dark gray
        SDL_RenderFillRect(ren, &pole);
        
        // Positions for the three lights
        int red_y = y - 15;
        int yellow_y = y;
        int green_y = y + 15;
        
        // Draw housings (dark gray squares)
        SDL_Rect housing_red = {x-8, red_y-8, 16, 16};
        SDL_RenderFillRect(ren, &housing_red);
        SDL_Rect housing_yellow = {x-8, yellow_y-8, 16, 16};
        SDL_RenderFillRect(ren, &housing_yellow);
        SDL_Rect housing_green = {x-8, green_y-8, 16, 16};
        SDL_RenderFillRect(ren, &housing_green);
        
        // Draw the active light
        int radius = 6;
        if(s == LIGHT_RED) {
            SDL_SetRenderDrawColor(ren, 255, 50, 50, 255);
            for(int dx = -radius; dx <= radius; dx++) {
                for(int dy = -radius; dy <= radius; dy++) {
                    if(dx*dx + dy*dy <= radius*radius) {
                        SDL_RenderDrawPoint(ren, x + dx, red_y + dy);
                    }
                }
            }
        } else if(s == LIGHT_YELLOW) {
            SDL_SetRenderDrawColor(ren, 255, 200, 0, 255);
            for(int dx = -radius; dx <= radius; dx++) {
                for(int dy = -radius; dy <= radius; dy++) {
                    if(dx*dx + dy*dy <= radius*radius) {
                        SDL_RenderDrawPoint(ren, x + dx, yellow_y + dy);
                    }
                }
            }
        } else if(s == LIGHT_GREEN) {
            SDL_SetRenderDrawColor(ren, 0, 255, 100, 255);
            for(int dx = -radius; dx <= radius; dx++) {
                for(int dy = -radius; dy <= radius; dy++) {
                    if(dx*dx + dy*dy <= radius*radius) {
                        SDL_RenderDrawPoint(ren, x + dx, green_y + dy);
                    }
                }
            }
        }
    }

    // Draw lights at corners, positioned outside the road
    int light_offset = 30;
    draw_traffic_light(cx - ROAD_FULL_WIDTH/2 - light_offset, cy - ROAD_FULL_WIDTH/2 - light_offset, light_NS); // Top Left (For Southbound)
    draw_traffic_light(cx + ROAD_FULL_WIDTH/2 + light_offset, cy + ROAD_FULL_WIDTH/2 + light_offset, light_NS); // Bottom Right (For Northbound)
    draw_traffic_light(cx - ROAD_FULL_WIDTH/2 - light_offset, cy + ROAD_FULL_WIDTH/2 + light_offset, light_EW); // Bottom Left (For Eastbound)
    draw_traffic_light(cx + ROAD_FULL_WIDTH/2 + light_offset, cy - ROAD_FULL_WIDTH/2 - light_offset, light_EW); // Top Right (For Westbound)
}

void draw_cars(SDL_Renderer* ren) {
    for(int i=0; i<200; i++) {
        Vehicle* v = &vehicles[i];
        if(!v->active) continue;

        // Color based on intent (Subtle UI)
        if(v->intent == TURN_LEFT) SDL_SetRenderDrawColor(ren, 100, 150, 255, 255); // Blue tint
        else if(v->intent == TURN_RIGHT) SDL_SetRenderDrawColor(ren, 255, 150, 100, 255); // Orange tint
        else SDL_SetRenderDrawColor(ren, 220, 220, 220, 255); // White

        // Manual Rotation Render
        float hw = VEHICLE_W / 2.0f;
        float hl = VEHICLE_L / 2.0f;
        
        float c = cos(v->angle);
        float s = sin(v->angle);
        
        // 4 Corners of the rectangle
        float p1x = -hl * c - -hw * s + v->x; float p1y = -hl * s + -hw * c + v->y;
        float p2x =  hl * c - -hw * s + v->x; float p2y =  hl * s + -hw * c + v->y;
        float p3x =  hl * c -  hw * s + v->x; float p3y =  hl * s +  hw * c + v->y;
        float p4x = -hl * c -  hw * s + v->x; float p4y = -hl * s +  hw * c + v->y;

        // Draw basic quad using lines
        SDL_RenderDrawLine(ren, p1x, p1y, p2x, p2y);
        SDL_RenderDrawLine(ren, p2x, p2y, p3x, p3y);
        SDL_RenderDrawLine(ren, p3x, p3y, p4x, p4y);
        SDL_RenderDrawLine(ren, p4x, p4y, p1x, p1y);
        
        // Fill (inefficient but works for minimal cars)
        SDL_Rect r = { (int)(v->x - 6), (int)(v->y - 6), 12, 12 };
        SDL_RenderFillRect(ren, &r);
    }
}

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;
    srand(time(NULL));
    if(SDL_Init(SDL_INIT_VIDEO) < 0) return 1;
    
    SDL_Window* win = SDL_CreateWindow("Minimal Traffic Sim", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    SDL_Renderer* ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    int running = 1;
    SDL_Event e;

    while(running) {
        while(SDL_PollEvent(&e)) { if(e.type == SDL_QUIT) running = 0; }

        // Logic
        if(frame % 80 == 0) spawn_vehicle(); // Reduced spawning frequency
        update_traffic_lights();
        update_vehicles();

        // Render
        SDL_SetRenderDrawColor(ren, 180, 180, 180, 255); // Gray background
        SDL_RenderClear(ren);

        draw_minimal_road(ren);
        draw_traffic_lights(ren);
        draw_cars(ren);

        SDL_RenderPresent(ren);
        frame++;
    }

    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}