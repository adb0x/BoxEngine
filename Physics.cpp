#include "Physics.h"
#include "PhysicsWorld.h"
#include "ComponentDB.h"
#include <algorithm>

luabridge::LuaRef Physics::Raycast(b2Vec2 pos, b2Vec2 dir, float dist) {
    lua_State* L = ComponentDB::GetLuaState();

    // Return nil if no world, dist <= 0, or no direction
    if (!PhysicsWorld::HasWorld() || dist <= 0.0f)
        return luabridge::LuaRef(L);

    dir.Normalize();
    b2Vec2 end = pos + dist * dir;

    RaycastCallback callback;
    PhysicsWorld::GetWorld()->RayCast(&callback, pos, end);

    if (!callback.hit_found)
        return luabridge::LuaRef(L);

    return luabridge::LuaRef(L, callback.closest_hit);
}

luabridge::LuaRef Physics::RaycastAll(b2Vec2 pos, b2Vec2 dir, float dist) {
    lua_State* L = ComponentDB::GetLuaState();

    if (!PhysicsWorld::HasWorld() || dist <= 0.0f) {
        // Return empty table
        return luabridge::newTable(L);
    }

    dir.Normalize();
    b2Vec2 end = pos + dist * dir;

    RaycastAllCallback callback;
    PhysicsWorld::GetWorld()->RayCast(&callback, pos, end);

    // Sort by fraction (nearest to furthest)
    std::sort(callback.hits.begin(), callback.hits.end(),
        [](const auto& a, const auto& b) { return a.first < b.first; });

    luabridge::LuaRef table = luabridge::newTable(L);
    int index = 1;
    for (auto& [fraction, hit] : callback.hits) {
        table[index++] = hit;
    }

    return table;
}