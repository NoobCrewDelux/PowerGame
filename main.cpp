#include <iostream>
#include <fstream>
#include <SDL2/SDL_ttf.h>
#include "./src/include/SDL2/SDL_image.h"
#include <SDL2/SDL.h>
#include <stdexcept>
#include <string>
#include <algorithm>

// Screen dimensions
const int SCREEN_WIDTH = 1920;
const int SCREEN_HEIGHT = 1080;

// Grid dimensions
const int GRID_WIDTH = 50;
const int GRID_HEIGHT = 50;
const int TILE_SIZE = 32; // Initial tile size (zoom will modify this)

// Camera position & zoom variables
float zoom = 1.0;
const float MIN_ZOOM = 0.5, MAX_ZOOM = 5.0;
float cameraX = 0;
float cameraY = 0;

float tile_spread = 2; // 2 normal. 1 = 1 air block separation, 0.5 = 2 air block separation, 4 = 2 block block density

// Middle mouse button drag variables
bool dragging = false;
int lastMouseX = 0, lastMouseY = 0;

// Desired FPS (frames per second)
const int FPS = 60;
const int FRAME_DELAY = 1000 / FPS;  // Time per frame in milliseconds

// Function to check if a file exists
bool fileExists(const std::string& filename) {
    std::ifstream f(filename.c_str());
    return f.good();
}

// Function to load an image surface
SDL_Surface* loadSurface(const std::string& path) {
    // Check if the file exists first
    if (!fileExists(path)) {
        std::cerr << "Texture file does not exist: " << path << std::endl;
        return nullptr;
    }

    // Load the image as an SDL_Surface
    SDL_Surface* surface = IMG_Load(path.c_str());
    if (surface == nullptr) {
        std::cerr << "Failed to load texture from path: " << path << "\n";
        std::cerr << "SDL_image error: " << IMG_GetError() << std::endl;
    } else {
        std::cout << "Image loaded successfully as SDL_Surface." << std::endl;
    }

    return surface;
}

void drawIsometricGrid(SDL_Renderer* renderer, SDL_Texture* tileTexture, float zoom) {
    if (renderer == nullptr) {
        throw std::runtime_error("Renderer is null");
    }
    if (tileTexture == nullptr) {
        throw std::runtime_error("Tile texture is null");
    }

    // Calculate the scaled tile size based on zoom
    int scaledTileSize = TILE_SIZE * zoom;

    for (int row = 0; row < GRID_HEIGHT; ++row) {
        for (int col = 0; col < GRID_WIDTH; ++col) {
            // Calculate screen position for isometric grid
            int screenX = (col - row) * scaledTileSize / tile_spread + SCREEN_WIDTH / 2 + cameraX;
            int screenY = (col + row) * scaledTileSize / (tile_spread * 2) + cameraY;

            // Define where to draw the tile
            SDL_Rect destRect = { screenX, screenY, scaledTileSize, scaledTileSize };

            // Render the texture directly
            if (SDL_RenderCopy(renderer, tileTexture, nullptr, &destRect) != 0) {
                throw std::runtime_error("Error rendering texture: " + std::string(SDL_GetError()));
            }
        }
    }
}

int main(int argc, char* argv[]) {
    try {
        std::cout << "Initializing SDL..." << std::endl;
        if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
            throw std::runtime_error("Failed to initialize SDL: " + std::string(SDL_GetError()));
        }

        std::cout << "Initializing SDL_image..." << std::endl;
        if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
            throw std::runtime_error("Failed to initialize SDL_image: " + std::string(IMG_GetError()));
        }

        std::cout << "Initializing SDL_ttf..." << std::endl;
        if (TTF_Init() == -1) {
            throw std::runtime_error("Failed to initialize SDL_ttf: " + std::string(TTF_GetError()));
        }

        std::cout << "Creating window..." << std::endl;
        SDL_Window* window = SDL_CreateWindow("Custom Isometric Grid Render", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
        if (!window) {
            throw std::runtime_error("Failed to create window: " + std::string(SDL_GetError()));
        }

        std::cout << "Creating renderer..." << std::endl;
        SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
        if (!renderer) {
            SDL_DestroyWindow(window);
            throw std::runtime_error("Failed to create accelerated renderer: " + std::string(SDL_GetError()));
        }

        std::cout << "Loading font..." << std::endl;
        TTF_Font* font = TTF_OpenFont("./resources/fonts/arial.ttf", 24);
        if (!font) {
            SDL_DestroyRenderer(renderer);
            SDL_DestroyWindow(window);
            throw std::runtime_error("Failed to load font: " + std::string(TTF_GetError()));
        }

        std::cout << "Loading texture..." << std::endl;
        SDL_Texture* tileTexture = IMG_LoadTexture(renderer, "C:/development/grass.png");
        if (!tileTexture) {
            TTF_CloseFont(font);
            SDL_DestroyRenderer(renderer);
            SDL_DestroyWindow(window);
            throw std::runtime_error("Failed to load texture: " + std::string(IMG_GetError()));
        }

        std::cout << "Texture loaded and ready!" << std::endl;

        bool running = true;
        SDL_Event event;
        Uint32 frameStart;
        int frameTime;
        int frame_count = 0;

        char fps_text[20];

        while (running) {
            frameStart = SDL_GetTicks();

            std::cout << "Entering event loop..." << std::endl;

            while (SDL_PollEvent(&event)) {
                switch (event.type) {
                    case SDL_QUIT:
                        running = false;
                        break;

                    case SDL_KEYDOWN:
                        if (event.key.keysym.sym == SDLK_ESCAPE) {
                            running = false;
                        }
                        break;

                    case SDL_MOUSEBUTTONDOWN:
                        if (event.button.button == SDL_BUTTON_MIDDLE) {
                            dragging = true;
                            lastMouseX = event.button.x;
                            lastMouseY = event.button.y;
                        }
                        break;

                    case SDL_MOUSEBUTTONUP:
                        if (event.button.button == SDL_BUTTON_MIDDLE) {
                            dragging = false;
                        }
                        break;

                    case SDL_MOUSEMOTION:
                        if (dragging) {
                            int deltaX = event.motion.x - lastMouseX;
                            int deltaY = event.motion.y - lastMouseY;
                            cameraX += deltaX;
                            cameraY += deltaY;
                            lastMouseX = event.motion.x;
                            lastMouseY = event.motion.y;
                        }
                        break;

                    case SDL_MOUSEWHEEL:
                        float zoom_change = event.wheel.y * 0.05f;
                        float new_zoom = zoom + zoom_change;
                        zoom = std::max(MIN_ZOOM, std::min(MAX_ZOOM, new_zoom));
                        break;
                }
            }

            SDL_SetRenderDrawColor(renderer, 135, 206, 235, 255);
            SDL_RenderClear(renderer);

            drawIsometricGrid(renderer, tileTexture, zoom);

            frame_count++;
            float fps = 1000.0f / (SDL_GetTicks() - frameStart + 1);
            sprintf(fps_text, "FPS: %.2f", fps);

            SDL_Surface* text_surface = TTF_RenderText_Solid(font, fps_text, {255, 255, 255, 255});
            if (text_surface) {
                SDL_Texture* text_texture = SDL_CreateTextureFromSurface(renderer, text_surface);
                if (text_texture) {
                    SDL_Rect dest_rect = {SCREEN_WIDTH - text_surface->w - 10, 10, text_surface->w, text_surface->h};
                    SDL_RenderCopy(renderer, text_texture, NULL, &dest_rect);
                    SDL_DestroyTexture(text_texture);
                }
                SDL_FreeSurface(text_surface);
            }

            SDL_RenderPresent(renderer);

            frameTime = SDL_GetTicks() - frameStart;
            if (frameTime < FRAME_DELAY) {
                SDL_Delay(FRAME_DELAY - frameTime);
            }
        }

        SDL_DestroyTexture(tileTexture);
        TTF_CloseFont(font);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        IMG_Quit();
        SDL_Quit();

        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }
}
