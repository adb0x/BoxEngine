#ifndef COMPONENTDB_H
#define COMPONENTDB_H

#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include "lua.hpp"
#include "LuaBridge.h"
#include "rapidjson/document.h"

// Forward declarations
class Actor;
class Rigidbody;
class ParticleSystem;

class ComponentDB {
public:
    // Initialize Lua integration
    static void Init(lua_State* L);

    // Get Lua state
    static lua_State* GetLuaState();

    // Load a component type
    static bool LoadComponentType(lua_State* L, const std::string& type);

    // Create an instance of a component type
    static luabridge::LuaRef CreateInstance(lua_State* L,
        const std::string& type,
        const std::string& key);

    // Queue OnStart after components created
    static void QueueOnStart(const luabridge::LuaRef& instance);
    static void RunOnStartQueue();

    // Inheritance
    static void EstablishInheritance(lua_State* L,
        const luabridge::LuaRef& instance_table,
        const luabridge::LuaRef& parent_table);

    // Logging functions
    static void CppLog(const std::string& message);
    static void CppLogError(const std::string& message);

    // Actor list helpers
    static void SetActorList(std::vector<Actor*>& actor_list);
    static std::vector<Actor*>& GetAllActors();

    // Clear variables
    static void Clear();

    // Error reporting
    static void ReportError(const std::string& actor_name, const luabridge::LuaException& e);

    static void InitializeActorFromTemplate(Actor* actor, const std::string& template_name);

    // Inject JSON properties into a Lua component table
    static void InjectProperties(luabridge::LuaRef& instance, const rapidjson::Value& comp_json);

    // Inject JSON properties directly into a C++ Rigidbody
    static void InjectRigidbodyProperties(Rigidbody* rb, const rapidjson::Value& comp_json);

    //Particle System JSON Injection 
    static void InjectParticleSystemProperties(ParticleSystem* ps, const rapidjson::Value& comp_json);

private:
    static lua_State* lua_state;

    // Mapping from type name to Lua table (component definitions)
    static std::unordered_map<std::string, luabridge::LuaRef> component_types;

    // Queue of OnStart functions
    static std::vector<luabridge::LuaRef> onstart_queue;

    // Pointer to current actor list
    static std::vector<Actor*>* actors;

    // LuaBridge static functions for Actor namespace
    static luabridge::LuaRef Lua_Find(std::string name);
    static luabridge::LuaRef Lua_FindAll(std::string name);
};

#endif