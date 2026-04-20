#include "Actor.h"
#include "ComponentDB.h"
#include "Engine.h"
#include <algorithm>
#include "Rigidbody.h"
#include "ParticleSystem.h"

using namespace std;

void Actor::RebuildFunctionCaches() {
    components_with_update.clear();
    components_with_late_update.clear();

    // components is a std::map so iteration is already in sorted (key) order.
    for (auto& [key, ref_ptr] : components) {
        luabridge::LuaRef& comp = *ref_ptr;

        luabridge::LuaRef on_update = comp["OnUpdate"];
        if (!on_update.isNil() && on_update.isFunction()) {
            components_with_update.push_back(ref_ptr);
        }

        luabridge::LuaRef on_late = comp["OnLateUpdate"];
        if (!on_late.isNil() && on_late.isFunction()) {
            components_with_late_update.push_back(ref_ptr);
        }
    }
}

luabridge::LuaRef Actor::GetComponentByKey(const std::string& key) const
{
    auto it = components.find(key);

    if (it != components.end())
        return *(it->second);

    return luabridge::LuaRef(ComponentDB::GetLuaState()); // nil
}

luabridge::LuaRef Actor::GetComponent(const std::string& type) const {
    for (auto const& [key, ref] : components) {
        if ((*ref)["type"].isString() && (*ref)["type"].cast<std::string>() == type) {
            return *ref;
        }
    }
    return luabridge::LuaRef(ComponentDB::GetLuaState());
}

luabridge::LuaRef Actor::GetComponents(const std::string& type) const
{
    lua_State* L = ComponentDB::GetLuaState();
    luabridge::LuaRef table = luabridge::newTable(L);

    int index = 1;

    // map is already sorted by key
    for (const auto& [key, ref] : components) {
        if ((*ref)["type"].isString() &&
            (*ref)["type"].cast<std::string>() == type) {
            table[index++] = *ref;
        }
    }

    return table;
}

void Actor::InjectConvenienceReferences(std::shared_ptr<luabridge::LuaRef> component_ref) {
    luabridge::LuaRef& comp = *component_ref;

    if (comp.isTable()) {
        comp["actor"] = this;
    }
    else if (comp.isUserdata()) {
        luabridge::LuaRef type_ref = comp["type"];
        if (type_ref.isString()) {
            std::string type_name = type_ref.cast<std::string>();

            if (type_name == "Rigidbody") {
                Rigidbody* rb = comp.cast<Rigidbody*>();
                if (rb) rb->actor = this;
            }
            else if (type_name == "ParticleSystem") {
                ParticleSystem* ps = comp.cast<ParticleSystem*>();
                if (ps) ps->actor = this;
            }
        }
    }
}

luabridge::LuaRef Actor::AddComponent(std::string type_name) {
    std::string key = "r" + std::to_string(runtime_component_counter++);
    luabridge::LuaRef instance = ComponentDB::CreateInstance(ComponentDB::GetLuaState(), type_name, key);

    if (instance.isTable()) {
        instance["enabled"] = true;
        instance["actor"] = this;
    }
    else if (instance.isUserdata()) {
        // Handle C++ Rigidbody
        try {
            Rigidbody* rb = instance.cast<Rigidbody*>();
            if (rb) rb->actor = this;
        }
        catch (...) {}

        // Handle C++ ParticleSystem
        try {
            ParticleSystem* ps = instance.cast<ParticleSystem*>();
            if (ps) ps->actor = this;
        }
        catch (...) {}
    }

    auto ref_ptr = std::make_shared<luabridge::LuaRef>(instance);
    components[key] = ref_ptr;
    cache_dirty = true;

    ComponentDB::QueueOnStart(instance);
    return instance;
}

void Actor::RemoveComponent(luabridge::LuaRef comp_ref) {
    if (comp_ref.isNil()) return;

    std::string key = "";
    if (comp_ref.isUserdata()) {
        for (auto it = components.begin(); it != components.end(); ++it) {
            if (*(it->second) == comp_ref) {
                key = it->first;
                break;
            }
        }
    }
    else {
        key = comp_ref["key"].cast<std::string>();
    }

    if (key.empty()) return;

    // Call OnDestroy before removing
    if (comp_ref.isUserdata()) {
        try {
            Rigidbody* rb = comp_ref.cast<Rigidbody*>();
            if (rb) rb->OnDestroy();
        }
        catch (...) {}

        try {
            ParticleSystem* ps = comp_ref.cast<ParticleSystem*>();
            if (ps) ps->OnDestroy();
        }
        catch (...) {}
    }
    else {
        luabridge::LuaRef fn = comp_ref["OnDestroy"];
        if (!fn.isNil() && fn.isFunction()) {
            try { fn(comp_ref); }
            catch (const luabridge::LuaException& e) {
                ComponentDB::ReportError(actor_name, e);
            }
        }
    }

    components.erase(key);
    cache_dirty = true;
}

luabridge::LuaRef Actor::Instantiate(std::string template_name) {
    Actor* new_actor = new Actor();
    ComponentDB::InitializeActorFromTemplate(new_actor, template_name);
    Engine::actors_to_spawn.push_back(new_actor);
    return luabridge::LuaRef(ComponentDB::GetLuaState(), new_actor);
}

void Actor::Destroy(Actor* actor_ptr) {
    if (!actor_ptr || actor_ptr->marked_for_destruction) return;

    actor_ptr->marked_for_destruction = true;

    for (auto const& [key, comp_ptr] : actor_ptr->components) {
        if ((*comp_ptr).isTable()) {
            (*comp_ptr)["enabled"] = false;
        }
    }

    //call destroy immediately
    actor_ptr->OnDestroy();

    Engine::actors_to_destroy.push_back(actor_ptr);
}

void Actor::OnCollision(const std::string& func_name, const Collision& col) {
    for (auto& [key, ref_ptr] : components) {
        luabridge::LuaRef& comp = *ref_ptr;

        if (!comp.isTable()) continue;
        if (comp["enabled"].isNil() || !comp["enabled"].cast<bool>()) continue;

        luabridge::LuaRef fn = comp[func_name];
        if (fn.isNil() || !fn.isFunction()) continue;

        try {
            fn(comp, col);
        }
        catch (const luabridge::LuaException& e) {
            ComponentDB::ReportError(actor_name, e);
        }
    }
}
void Actor::OnDestroy() {
    for (auto& [key, ref_ptr] : components) {
        luabridge::LuaRef& comp = *ref_ptr;

        if (comp.isUserdata()) {
            bool handled = false;

            // Check type string first to avoid triggering internal LuaBridge cast errors
            luabridge::LuaRef type_ref = comp["type"];
            if (type_ref.isString()) {
                std::string type_name = type_ref.cast<std::string>();
                if (type_name == "Rigidbody") {
                    Rigidbody* rb = comp.cast<Rigidbody*>();
                    if (rb) { rb->OnDestroy(); handled = true; }
                }
                else if (type_name == "ParticleSystem") {
                    ParticleSystem* ps = comp.cast<ParticleSystem*>();
                    if (ps) { ps->OnDestroy(); handled = true; }
                }
            }
            if (handled) continue;
        }

        // Lua component path
        luabridge::LuaRef fn = comp["OnDestroy"];
        if (fn.isNil() || !fn.isFunction()) continue;

        try {
            fn(comp);
        }
        catch (const luabridge::LuaException& e) {
            ComponentDB::ReportError(actor_name, e);
        }
    }
}