#include "Renderer.h"
#include "Helper.h"
#include <iostream>

Renderer::Renderer(const std::string& window_title, int width, int height,
    Uint8 r, Uint8 g, Uint8 b)
    : clear_color_r(r), clear_color_g(g), clear_color_b(b)
 {
    window = Helper::SDL_CreateWindow(
        window_title.c_str(),
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        width,
        height,
        SDL_WINDOW_SHOWN
    );

    if (!window) {
        std::cout << "Failed to create SDL window: " << SDL_GetError() << std::endl;
        exit(1);
    }

    renderer = Helper::SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    if (!renderer) {
        std::cout << "Failed to create SDL renderer: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        exit(1);
    }
}

void Renderer::Clear() {
    SDL_SetRenderDrawColor(renderer, clear_color_r, clear_color_g, clear_color_b, 255);
    SDL_RenderClear(renderer);
}

void Renderer::Present() {
    Helper::SDL_RenderPresent(renderer);
}

Renderer::~Renderer() {
    if (renderer) {
        SDL_DestroyRenderer(renderer);
    }
    if (window) {
        SDL_DestroyWindow(window);
    }
}

SDL_Window* Renderer::GetSDLWindow() const { //may be useful later
    return window;
}

SDL_Renderer* Renderer::GetSDLRenderer() const { //may be useful later
    return renderer;
}
