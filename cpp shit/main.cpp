#include <SDL2/SDL.h>
#include <iostream>

//camera constats


// Constants for isometric grid
const int TILE_WIDTH = 50;
const int TILE_HEIGHT = 25;
const int PRISM_HEIGHT = 12;  // Integer value (since int type doesn't support decimals)
const int ROWS = 50;
const int COLS = 50;
const int SCREEN_WIDTH = 1920;
const int SCREEN_HEIGHT = 1080;

// Colors
const SDL_Color TILE_TOP_COLOR = {100, 200, 100, 255}; // Light green
const SDL_Color TILE_SIDE_COLOR = {80, 150, 80, 255};  // Darker green
const SDL_Color TILE_FRONT_COLOR = {60, 120, 60, 255}; // Even darker green

void drawPrism(SDL_Renderer* renderer, int x, int y) {
    // Top Face Points
    SDL_Point topFace[5] = {
        {x, y - TILE_HEIGHT / 2},                   // Top
        {x + TILE_WIDTH / 2, y},                    // Right
        {x, y + TILE_HEIGHT / 2},                   // Bottom
        {x - TILE_WIDTH / 2, y},                    // Left
        {x, y - TILE_HEIGHT / 2}                    // Back to Top
    };

    // Side Face Points
    SDL_Point sideFace[5] = {
        {x + TILE_WIDTH / 2, y},                    // Top Right
        {x + TILE_WIDTH / 2, y + PRISM_HEIGHT},     // Bottom Right
        {x, y + TILE_HEIGHT / 2 + PRISM_HEIGHT},    // Bottom Center
        {x, y + TILE_HEIGHT / 2},                   // Top Center
        {x + TILE_WIDTH / 2, y}                     // Back to Top Right
    };

    // Front Face Points
    SDL_Point frontFace[5] = {
        {x, y + TILE_HEIGHT / 2},                   // Top Center
        {x, y + TILE_HEIGHT / 2 + PRISM_HEIGHT},    // Bottom Center
        {x - TILE_WIDTH / 2, y + PRISM_HEIGHT},     // Bottom Left
        {x - TILE_WIDTH / 2, y},                    // Top Left
        {x, y + TILE_HEIGHT / 2}                    // Back to Top Center
    };

    // Draw Top Face
    SDL_SetRenderDrawColor(renderer, TILE_TOP_COLOR.r, TILE_TOP_COLOR.g, TILE_TOP_COLOR.b, TILE_TOP_COLOR.a);
    SDL_RenderDrawLines(renderer, topFace, 5);

    // Draw Side Face
    SDL_SetRenderDrawColor(renderer, TILE_SIDE_COLOR.r, TILE_SIDE_COLOR.g, TILE_SIDE_COLOR.b, TILE_SIDE_COLOR.a);
    SDL_RenderDrawLines(renderer, sideFace, 5);

    // Draw Front Face
    SDL_SetRenderDrawColor(renderer, TILE_FRONT_COLOR.r, TILE_FRONT_COLOR.g, TILE_FRONT_COLOR.b, TILE_FRONT_COLOR.a);
    SDL_RenderDrawLines(renderer, frontFace, 5);
}

int main(int argc, char* argv[]) {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Isometric Prisms", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }


    bool running = true;
    SDL_Event e;

    while (running) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) running = false;
            if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) running = false; // Exit on Esc key
            
            if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) {
            running = false; // Exit on Esc key
            }

        }
        

        // Clear screen
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // White background
        SDL_RenderClear(renderer);
        // Draw grid
        for (int row = 0; row < ROWS; ++row) {
            for (int col = 0; col < COLS; ++col) {
                int x = (col - row) * (TILE_WIDTH / 2) + SCREEN_WIDTH / 2;
                int y = (col + row) * (TILE_HEIGHT / 2) ;
                drawPrism(renderer, x, y);
            }
        }

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
