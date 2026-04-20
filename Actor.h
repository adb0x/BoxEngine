#ifndef ACTOR_H
#define ACTOR_H

#include <string>
#include <map>
#include <vector>
#include <memory>
#include <unordered_map>
#include "lua.hpp"
#include "LuaBridge.h"
#include "ComponentDB.h"
#include "Collision.h"

class Actor : public std::enable_shared_from_this<Actor> {
public:
    int actor_id = -1;
    std::string actor_name = "";
    std::string template_name = "";

    bool marked_for_destruction = false;
    static inline int runtime_component_counter = 0;

    bool dont_destroy_on_load = false;

    bool cache_dirty = false;

    // Keyed map for lookups (GetComponentByKey, overrides, etc.)
    std::map<std::string, std::shared_ptr<luabridge::LuaRef>> components;

    // *** OPTIMIZATION: Pre-cached lists for fast per-frame iteration ***
    // Populated once after all components are loaded / when components are added or removed.
    // Avoids copying the map and avoids per-frame isNil() / function existence checks.
    std::vector<std::shared_ptr<luabridge::LuaRef>> components_with_update;
    std::vector<std::shared_ptr<luabridge::LuaRef>> components_with_late_update;

    Actor() = default;

    Actor(int id,
        const std::string& name,
        const std::string& template_str)
        : actor_id(id),
        actor_name(name),
        template_name(template_str)
    {
    }

    // Rebuild the fast-iteration vectors from the components map.
    // Call this after all components on an actor have been loaded/modified.
    void RebuildFunctionCaches();

    // --- Flexible Referencing ---
    std::string GetName() const { return actor_name; }
    int GetID() const { return actor_id; }

    // Get a component by key (or nil if not found)
    luabridge::LuaRef GetComponentByKey(const std::string& key) const;

    // Get first component by type (or nil)
    luabridge::LuaRef GetComponent(const std::string& type) const;

    // Get all components of a type as Lua table
    luabridge::LuaRef GetComponents(const std::string& type) const;

    // Inject actor reference into component
    void InjectConvenienceReferences(std::shared_ptr<luabridge::LuaRef> component_ref);

    // Component API
    luabridge::LuaRef AddComponent(std::string type_name);
    void RemoveComponent(luabridge::LuaRef comp_ref);

    // Static Actor API
    static luabridge::LuaRef Instantiate(std::string template_name);
    static void Destroy(Actor* actor_ptr);

    //collision
    void OnCollision(const std::string& func_name, const Collision& col);

    //destroy 
    void OnDestroy();
};

#endif