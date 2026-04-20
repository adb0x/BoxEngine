#ifndef SCENEDB_H
#define SCENEDB_H

#include <vector>
#include <string>
#include <memory>
#include "Actor.h"
#include "lua.hpp"

class SceneDB {
public:
    // Change from shared_ptr to Actor*
    static bool LoadScene(lua_State* L, const std::string& scene_name, std::vector<Actor*>& out_actors);
};

#endif