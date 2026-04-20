#ifndef ENGINE_H
#define ENGINE_H

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include "Actor.h"
#include "Renderer.h"
#include "glm/glm.hpp"

// Include Lua
#include "lua.hpp"


class Engine {
public:
    Engine();
    ~Engine();

    void GameLoop();

    // Expose Lua state for components
    lua_State* L = nullptr;

    static inline std::vector<Actor*> actors; // Main list
    static inline std::vector<Actor*> actors_to_spawn; // Queue for Instantiate
    static inline std::vector<Actor*> actors_to_destroy; // Queue for Destroy

private:

    // Lifecycle helper
    void CallComponentFunction(const std::string& function_name);

    bool game_running;

    //int cam_width = 13;
    //int cam_height = 9;
    int window_width = 640;
    int window_height = 360;


    std::unique_ptr<Renderer> renderer;

};

#endif