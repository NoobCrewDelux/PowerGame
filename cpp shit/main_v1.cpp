#include <SDL2/SDL.h>
#include <iostream>
#include <random>
#include <vector>
#include <atomic>

// Window dimensions
const int WIDTH = 1920, HEIGHT = 1080;
const int GAMEFPS = 60;

// Grid setup
const int GRID_WIDTH = 20, GRID_HEIGHT = 20;
const int BASE_TILE_SIZE = 60;
float zoom = 1.0;
const float MIN_ZOOM = 0.5, MAX_ZOOM = 2.0;

// Colors
SDL_Color BACKGROUND_COLOR = {135, 206, 235, 255};
SDL_Color WATER_COLOR = {0, 0, 255, 255};  // Blue for water (lakes)
SDL_Color GRASS_COLOR = {17, 124, 19, 255};  // Solid green for grass
SDL_Color GRID_COLOR = {0, 0, 0, 255};  // Black for outlines

// Camera setup
int camera_x = 0, camera_y = 0;
bool dragging = false;
SDL_Point last_mouse_pos = {0, 0};

// Lake setup (1% chance of turning a tile blue)
std::vector<std::vector<bool>> lake_map(GRID_WIDTH, std::vector<bool>(GRID_HEIGHT, false));

void generate_lakes() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 99);  // For 1% chance (0-99)

    for (int x = 0; x < GRID_WIDTH; ++x) {
        for (int y = 0; y < GRID_HEIGHT; ++y) {
            if (dis(gen) < 1) {  // 1% chance
                lake_map[x][y] = true;  // Set the tile to be a lake (blue)
            }
        }
    }
}

SDL_Point grid_to_iso(int x, int y) {
    int tile_size = int(BASE_TILE_SIZE * zoom);
    int iso_x = (x - y) * tile_size / 2 + WIDTH / 2 + camera_x;
    int iso_y = (x + y) * tile_size / 4 + 100 + camera_y;
    return {iso_x, iso_y};
}

void draw_grid(SDL_Renderer* renderer) {
    for (int x = 0; x < GRID_WIDTH; ++x) {
        for (int y = 0; y < GRID_HEIGHT; ++y) {
            SDL_Point iso_pos = grid_to_iso(x, y);
            int tile_size = int(BASE_TILE_SIZE * zoom);

            bool is_water = lake_map[x][y];  // Check if this tile should be water (lake)

            // Draw tile based on whether it's water or grass
            if (is_water) {
                SDL_Rect rect = {iso_pos.x - tile_size / 2, iso_pos.y - tile_size / 2, tile_size, tile_size};
                SDL_SetRenderDrawColor(renderer, WATER_COLOR.r, WATER_COLOR.g, WATER_COLOR.b, WATER_COLOR.a);
                SDL_RenderFillRect(renderer, &rect);
            } else {
                // Draw grass tile (solid green with black outline)
                SDL_Rect rect = {iso_pos.x - tile_size / 2, iso_pos.y - tile_size / 2, tile_size, tile_size};
                SDL_SetRenderDrawColor(renderer, GRASS_COLOR.r, GRASS_COLOR.g, GRASS_COLOR.b, GRASS_COLOR.a);
                SDL_RenderFillRect(renderer, &rect);

                // Draw black outline
                SDL_SetRenderDrawColor(renderer, GRID_COLOR.r, GRID_COLOR.g, GRID_COLOR.b, GRID_COLOR.a);
                SDL_RenderDrawRect(renderer, &rect);
            }
        }
    }
}

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0) {
        std::cerr << "SDL2 could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return -1;
    }

    SDL_Window* window = SDL_CreateWindow("2.5D Plane with Random Lakes", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WIDTH, HEIGHT, SDL_WINDOW_FULLSCREEN | SDL_WINDOW_SHOWN);
    if (!window) {
        std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        return -1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        std::cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        return -1;
    }

    // Generate random lakes
    generate_lakes();

    SDL_Event e;
    bool quit = false;

    while (!quit) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
            }
            else if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_MIDDLE) {
                dragging = true;
                last_mouse_pos.x = e.button.x;
                last_mouse_pos.y = e.button.y;
            }
            else if (e.type == SDL_MOUSEBUTTONUP && e.button.button == SDL_BUTTON_MIDDLE) {
                dragging = false;
            }
            else if (e.type == SDL_MOUSEWHEEL) {
                float zoom_change = e.wheel.y * 0.1f;
                float new_zoom = zoom + zoom_change;
                zoom = std::max(MIN_ZOOM, std::min(MAX_ZOOM, new_zoom));

                // Adjust camera position based on zoom
                SDL_Point mouse_pos = {e.motion.x, e.motion.y};
                float zoom_factor = zoom / (zoom - zoom_change);
                camera_x -= (mouse_pos.x - WIDTH / 2) * (zoom_factor - 1);
                camera_y -= (mouse_pos.y - HEIGHT / 2) * (zoom_factor - 1);
            }
        }

        SDL_SetRenderDrawColor(renderer, BACKGROUND_COLOR.r, BACKGROUND_COLOR.g, BACKGROUND_COLOR.b, BACKGROUND_COLOR.a);
        SDL_RenderClear(renderer);

        draw_grid(renderer);

        SDL_RenderPresent(renderer);

        SDL_Delay(1000 / GAMEFPS);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
