#include "TextDB.h"
#include "Helper.h" // Include your Helper header
#include <iostream>

std::vector<TextDrawRequest> TextDB::draw_queue;
std::unordered_map<std::string, std::unordered_map<int, TTF_Font*>> TextDB::font_cache;

void TextDB::Init() {
    if (TTF_Init() == -1) {
        std::cout << "SDL_ttf could not initialize! TTF_Error: " << TTF_GetError() << std::endl;
    }
}

void TextDB::Lua_Draw(std::string str, float x, float y, std::string font_name, float size, float r, float g, float b, float a) {
    // Downcast floats to ints
    TextDrawRequest req;
    req.content = str;
    req.x = (int)x;
    req.y = (int)y;
    req.font_name = font_name;
    req.font_size = (int)size;
    req.color = { (Uint8)r, (Uint8)g, (Uint8)b, (Uint8)a };

    draw_queue.push_back(req);
}

void TextDB::RenderAll(SDL_Renderer* renderer) {
    for (const auto& req : draw_queue) {
        // 1. Font Caching Logic
        if (font_cache[req.font_name].find(req.font_size) == font_cache[req.font_name].end()) {
            std::string path = "resources/fonts/" + req.font_name + ".ttf";
            TTF_Font* font = TTF_OpenFont(path.c_str(), req.font_size);
            if (!font) continue;
            font_cache[req.font_name][req.font_size] = font;
        }

        TTF_Font* font = font_cache[req.font_name][req.font_size];

        // 2. Surface & Texture Creation
        SDL_Surface* surface = TTF_RenderText_Solid(font, req.content.c_str(), req.color);
        if (!surface) continue;

        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        if (texture) {

            SDL_FRect dstRect = { (float)req.x, (float)req.y, (float)surface->w, (float)surface->h };

            Helper::SDL_RenderCopyEx(-1, "UI_TEXT", renderer, texture, NULL, &dstRect, 0.0f, NULL, SDL_FLIP_NONE);

            SDL_DestroyTexture(texture);
        }

        SDL_FreeSurface(surface);
    }
    draw_queue.clear();
}

void TextDB::Quit() {
    for (auto& pair : font_cache) {
        for (auto& size_pair : pair.second) {
            TTF_CloseFont(size_pair.second);
        }
    }
    font_cache.clear();
    TTF_Quit();
}