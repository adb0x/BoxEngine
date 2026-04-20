#ifndef IMAGEDB_H
#define IMAGEDB_H

#include <string>
#include <vector>
#include <unordered_map>
#include "SDL2/SDL.h"
#include "SDL2_image/SDL_image.h"

enum class RenderType { SCENE = 0, UI = 1, PIXEL = 2 };

struct ImageDrawRequest {
    std::string image_name;
    float x;
    float y;
    int rotation_degrees;
    float scale_x;
    float scale_y;
    float pivot_x;
    float pivot_y;
    int r, g, b, a;
    int sorting_order;

    RenderType type;
    size_t call_order;
};

struct PixelDrawRequest {
    int x, y, r, g, b, a;
};

class ImageDB {
public:
    // Lua-facing API
    static void Draw(std::string image_name, float x, float y);
    static void DrawEx(std::string image_name, float x, float y, float rotation, float scale_x, float scale_y, float pivot_x, float pivot_y, float r, float g, float b, float a, float sorting_order);
    static void DrawUI(std::string image_name, float x, float y);
    static void DrawUIEx(std::string image_name, float x, float y, float r, float g, float b, float a, float sorting_order);
    static void DrawPixel(float x, float y, float r, float g, float b, float a);

    // Engine-facing API
    static void RenderAndClearAll(SDL_Renderer* renderer, float zoom_factor, float cam_x, float cam_y);
    static void Quit();

    //Particle system
    static void Init(SDL_Renderer* renderer);
    static void CreateDefaultParticleTextureWithName(const std::string& name);
private:
    static SDL_Renderer* sdl_renderer;

    static std::vector<ImageDrawRequest> image_draw_request_queue;
    static std::unordered_map<std::string, SDL_Texture*> texture_cache;
    static size_t request_count;

    static SDL_Texture* GetTexture(SDL_Renderer* renderer, const std::string& image_name);

    static std::vector<PixelDrawRequest> pixel_draw_request_queue;
};

#endif