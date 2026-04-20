#ifndef TEXTDB_H
#define TEXTDB_H

#include <string>
#include <vector>
#include <unordered_map>
#include "SDL2/SDL.h"
#include "ThirdParty/SDL2_ttf/SDL_ttf.h"

struct TextDrawRequest {
    std::string content;
    int x, y;
    std::string font_name;
    int font_size;
    SDL_Color color;
};

class TextDB {
public:
    static void Init();
    static void Lua_Draw(std::string str, float x, float y, std::string font_name, float size, float r, float g, float b, float a);
    static void RenderAll(SDL_Renderer* renderer);
    static void Quit();

private:
    static std::vector<TextDrawRequest> draw_queue;
    static std::unordered_map<std::string, std::unordered_map<int, TTF_Font*>> font_cache;
};

#endif