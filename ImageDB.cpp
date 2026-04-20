#include "ImageDB.h"
#include "Helper.h"
#include <algorithm>
#include <cmath>
#include <glm/glm.hpp>

std::vector<ImageDrawRequest> ImageDB::image_draw_request_queue;
std::unordered_map<std::string, SDL_Texture*> ImageDB::texture_cache;
size_t ImageDB::request_count = 0;

std::vector<PixelDrawRequest> ImageDB::pixel_draw_request_queue;

SDL_Renderer* ImageDB::sdl_renderer = nullptr;

SDL_Texture* ImageDB::GetTexture(SDL_Renderer* renderer, const std::string& image_name) {
    if (texture_cache.find(image_name) == texture_cache.end()) {
        std::string path = "resources/images/" + image_name + ".png";
        texture_cache[image_name] = IMG_LoadTexture(renderer, path.c_str());
    }
    return texture_cache[image_name];
}

void ImageDB::Draw(std::string image_name, float x, float y) {
    DrawEx(image_name, x, y, 0, 1, 1, 0.5f, 0.5f, 255, 255, 255, 255, 0);
}

void ImageDB::DrawEx(std::string image_name, float x, float y, float rotation, float scale_x, float scale_y, float pivot_x, float pivot_y, float r, float g, float b, float a, float sorting_order) {
    image_draw_request_queue.push_back({ std::move(image_name), x, y, (int)rotation, scale_x, scale_y, pivot_x, pivot_y, (int)r, (int)g, (int)b, (int)a, (int)sorting_order, RenderType::SCENE, request_count++ });
}

void ImageDB::DrawUI(std::string image_name, float x, float y) {
    DrawUIEx(image_name, x, y, 255, 255, 255, 255, 0);
}

void ImageDB::DrawUIEx(std::string image_name, float x, float y, float r, float g, float b, float a, float sorting_order) {
    // UI usually defaults to top-left (0,0) pivot
    image_draw_request_queue.push_back({ image_name, x, y, 0, 1.0f, 1.0f, 0.0f, 0.0f, (int)r, (int)g, (int)b, (int)a, (int)sorting_order, RenderType::UI, request_count++ });
}

void ImageDB::DrawPixel(float x, float y, float r, float g, float b, float a) {
    pixel_draw_request_queue.push_back({ (int)x, (int)y, (int)r, (int)g, (int)b, (int)a });
}

void ImageDB::RenderAndClearAll(SDL_Renderer* renderer, float zoom_factor, float cam_x, float cam_y) {
    // std::sort with call_order tiebreaker gives a total order — no need for stable_sort
    std::sort(image_draw_request_queue.begin(), image_draw_request_queue.end(),
        [](const ImageDrawRequest& a, const ImageDrawRequest& b) {
            if (a.type != b.type) return (int)a.type < (int)b.type;
            if (a.sorting_order != b.sorting_order) return a.sorting_order < b.sorting_order;
            return a.call_order < b.call_order;
        });

    int window_w, window_h;
    SDL_GetRendererOutputSize(renderer, &window_w, &window_h);
    const int pixels_per_unit = 100;

    for (auto& req : image_draw_request_queue) {
        if (req.type == RenderType::PIXEL) {
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(renderer, req.r, req.g, req.b, req.a);
            SDL_RenderDrawPoint(renderer, (int)req.x, (int)req.y);
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
            continue;
        }

        // Apply staff-recommended scale handling
        if (req.type == RenderType::SCENE)
            SDL_RenderSetScale(renderer, zoom_factor, zoom_factor);
        else
            SDL_RenderSetScale(renderer, 1.0f, 1.0f);

        SDL_Texture* tex = GetTexture(renderer, req.image_name);
        if (!tex) continue;

        float base_w, base_h;
        Helper::SDL_QueryTexture(tex, &base_w, &base_h);

        SDL_RendererFlip flip = SDL_FLIP_NONE;
        if (req.scale_x < 0) flip = (SDL_RendererFlip)(flip | SDL_FLIP_HORIZONTAL);
        if (req.scale_y < 0) flip = (SDL_RendererFlip)(flip | SDL_FLIP_VERTICAL);

        float final_w = base_w * glm::abs(req.scale_x);
        float final_h = base_h * glm::abs(req.scale_y);

        SDL_FPoint pivot_pt = { req.pivot_x * final_w, req.pivot_y * final_h };

        SDL_FRect dst;
        dst.w = final_w;
        dst.h = final_h;

        if (req.type == RenderType::SCENE) {
            float world_rel_x = (req.x - cam_x) * pixels_per_unit;
            float world_rel_y = (req.y - cam_y) * pixels_per_unit;

            // Center offset logic matching staff expectations
            dst.x = world_rel_x + (window_w * 0.5f) * (1.0f / zoom_factor) - pivot_pt.x;
            dst.y = world_rel_y + (window_h * 0.5f) * (1.0f / zoom_factor) - pivot_pt.y;
        }
        else {
            dst.x = req.x - pivot_pt.x;
            dst.y = req.y - pivot_pt.y;
        }

        SDL_SetTextureColorMod(tex, req.r, req.g, req.b);
        SDL_SetTextureAlphaMod(tex, req.a);

        // Pass actor_id -1 and "DUMMY" to satisfy RenderLogger
        Helper::SDL_RenderCopyEx(-1, "DUMMY", renderer, tex, NULL, &dst, (float)req.rotation_degrees, &pivot_pt, flip);

        SDL_SetTextureColorMod(tex, 255, 255, 255);
        SDL_SetTextureAlphaMod(tex, 255);
    }

    //render pixels separately 
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    for (auto& req : pixel_draw_request_queue) {
        SDL_SetRenderDrawColor(renderer, req.r, req.g, req.b, req.a);
        SDL_RenderDrawPoint(renderer, req.x, req.y);
    }
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
    pixel_draw_request_queue.clear();

    // Reset for next frame/UI
    SDL_RenderSetScale(renderer, 1.0f, 1.0f);
    image_draw_request_queue.clear();
    request_count = 0;
}

void ImageDB::Quit() {
    for (auto const& [name, tex] : texture_cache) SDL_DestroyTexture(tex);
    texture_cache.clear();
}

void ImageDB::Init(SDL_Renderer* renderer) {
    sdl_renderer = renderer;
    image_draw_request_queue.reserve(16384);
}

void ImageDB::CreateDefaultParticleTextureWithName(const std::string& name) {
    if (texture_cache.find(name) != texture_cache.end()) return;
    SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormat(0, 8, 8, 32, SDL_PIXELFORMAT_RGBA8888);
    Uint32 white = SDL_MapRGBA(surface->format, 255, 255, 255, 255);
    SDL_FillRect(surface, NULL, white);
    SDL_Texture* tex = SDL_CreateTextureFromSurface(sdl_renderer, surface);
    SDL_FreeSurface(surface);
    texture_cache[name] = tex;
}