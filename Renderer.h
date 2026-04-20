#ifndef RENDERER_H
#define RENDERER_H

#include "SDL2/SDL.h"
#include <string>

class Renderer {
public:
    Renderer(const std::string& window_title, int width, int height, 
        Uint8 clear_color_r, Uint8 clear_color_g, Uint8 clear_color_b);
    ~Renderer();

    SDL_Window* GetSDLWindow() const;
    SDL_Renderer* GetSDLRenderer() const;
    void Clear();
    void Present();

private:
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    Uint8 clear_color_r;
    Uint8 clear_color_g;
    Uint8 clear_color_b;
};

#endif
