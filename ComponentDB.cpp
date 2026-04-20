#include "ComponentDB.h"
#include "Actor.h"
#include <iostream>
#include <filesystem>
#include "Engine.h"
#include "Application.h"
#include "Input.h"  
#include "TemplateDB.h"
#include "TextDB.h"
#include "AudioDB.h"
#include "ImageDB.h"
#include "Camera.h"
#include "Scene.h"
#include "box2d/box2d.h"
#include "Rigidbody.h"
#include "PhysicsWorld.h"
#include "HitResult.h"
#include "Physics.h"
#include "EventBus.h"
#include "ParticleSystem.h"

using namespace std;
namespace fs = std::filesystem;

lua_State* ComponentDB::lua_state = nullptr;

// --- Static members ---
unordered_map<string, luabridge::LuaRef> ComponentDB::component_types;
vector<luabridge::LuaRef> ComponentDB::onstart_queue;
vector<Actor*>* ComponentDB::actors = nullptr;

// --- LuaBridge Init ---
void ComponentDB::Init(lua_State* L) {
    lua_state = L;

    luabridge::getGlobalNamespace(L)
        .beginNamespace("Debug")
        .addFunction("Log", ComponentDB::CppLog)
        .addFunction("LogError", ComponentDB::CppLogError)
        .endNamespace();

    luabridge::getGlobalNamespace(L)
        .beginClass<Actor>("Actor")
        .addFunction("GetName", &Actor::GetName)
        .addFunction("GetID", &Actor::GetID)
        .addFunction("GetComponentByKey", &Actor::GetComponentByKey)
        .addFunction("GetComponent", &Actor::GetComponent)
        .addFunction("GetComponents", &Actor::GetComponents)
        .addFunction("AddComponent", &Actor::AddComponent)
        .addFunction("RemoveComponent", &Actor::RemoveComponent)
        .endClass();

    luabridge::getGlobalNamespace(L)
        .beginNamespace("Actor")
        .addFunction("Find", ComponentDB::Lua_Find)
        .addFunction("FindAll", ComponentDB::Lua_FindAll)
        .addFunction("Instantiate", &Actor::Instantiate)
        .addFunction("Destroy", &Actor::Destroy)
        .endNamespace();

    luabridge::getGlobalNamespace(L)
        .beginNamespace("Application")
        .addFunction("Quit", &Application::Quit)
        .addFunction("Sleep", &Application::Sleep)
        .addFunction("GetFrame", &Application::GetFrame)
        .addFunction("OpenURL", &Application::OpenURL)
        .endNamespace();

    luabridge::getGlobalNamespace(L)
        .beginClass<glm::vec2>("vec2")
        .addProperty("x", &glm::vec2::x)
        .addProperty("y", &glm::vec2::y)
        .endClass();

    luabridge::getGlobalNamespace(L)
        .beginNamespace("Input")
        .addFunction("GetKey", &Input::GetKey)
        .addFunction("GetKeyDown", &Input::GetKeyDown)
        .addFunction("GetKeyUp", &Input::GetKeyUp)
        .addFunction("GetMousePosition", &Input::GetMousePosition)
        .addFunction("GetMouseButton", &Input::GetMouseButton)
        .addFunction("GetMouseButtonDown", &Input::GetMouseButtonDown)
        .addFunction("GetMouseButtonUp", &Input::GetMouseButtonUp)
        .addFunction("GetMouseScrollDelta", &Input::GetMouseScrollDelta)
        .addFunction("HideCursor", &Input::HideCursor)
        .addFunction("ShowCursor", &Input::ShowCursor)

       // --- Controller ---
       
        .addFunction("GetControllerButton", &Input::GetControllerButton)
        .addFunction("GetControllerButtonDown", &Input::GetControllerButtonDown)
        .addFunction("GetControllerButtonUp", &Input::GetControllerButtonUp)
        .addFunction("GetControllerAxis", &Input::GetControllerAxis)
        .addFunction("GetControllerCount", &Input::GetControllerCount)
        .addFunction("RumbleController", &Input::RumbleController)

        .addFunction("GetJoystickAxis", &Input::GetJoystickAxis)
        .addFunction("GetJoystickButton", &Input::GetJoystickButton)
        .addFunction("GetJoystickAxisCount", &Input::GetJoystickAxisCount)
        .addFunction("GetJoystickButtonCount", &Input::GetJoystickButtonCount)

        .endNamespace();

    luabridge::getGlobalNamespace(L)
        .beginNamespace("Text")
        .addFunction("Draw", &TextDB::Lua_Draw)
        .endNamespace();

    luabridge::getGlobalNamespace(L)
        .beginNamespace("Audio")
        .addFunction("Play", &AudioDB::Play)
        .addFunction("Halt", &AudioDB::Halt)
        .addFunction("SetVolume", &AudioDB::SetVolume)
        .endNamespace();

    luabridge::getGlobalNamespace(L)
        .beginNamespace("Image")
        .addFunction("Draw", &ImageDB::Draw)
        .addFunction("DrawEx", &ImageDB::DrawEx)
        .addFunction("DrawUI", &ImageDB::DrawUI)
        .addFunction("DrawUIEx", &ImageDB::DrawUIEx)
        .addFunction("DrawPixel", &ImageDB::DrawPixel)
        .endNamespace();

    luabridge::getGlobalNamespace(L)
        .beginNamespace("Camera")
        .addFunction("SetPosition", &Camera::SetPosition)
        .addFunction("GetPositionX", &Camera::GetPositionX)
        .addFunction("GetPositionY", &Camera::GetPositionY)
        .addFunction("SetZoom", &Camera::SetZoom)
        .addFunction("GetZoom", &Camera::GetZoom)
        .endNamespace();

    luabridge::getGlobalNamespace(L)
        .beginNamespace("Scene")
        .addFunction("Load", &Scene::Load)
        .addFunction("GetCurrent", &Scene::GetCurrent)
        .addFunction("DontDestroy", &Scene::DontDestroy)
        .endNamespace();

    luabridge::getGlobalNamespace(L)
        .beginClass<b2Vec2>("Vector2")
        .addConstructor<void(*)(float, float)>()
        .addProperty("x", &b2Vec2::x)
        .addProperty("y", &b2Vec2::y)
        .addFunction("Normalize", &b2Vec2::Normalize)
        .addFunction("Length", &b2Vec2::Length)
        .addFunction("__add", &b2Vec2::operator_add)
        .addFunction("__sub", &b2Vec2::operator_sub)
        .addFunction("__mul", &b2Vec2::operator_mul)
        .addStaticFunction("Distance", static_cast<float(*)(const b2Vec2&, const b2Vec2&)>(&b2Distance))
        .addStaticFunction("Dot", static_cast<float(*)(const b2Vec2&, const b2Vec2&)>(&b2Dot))
        .endClass();

    // Register the Rigidbody C++ class with LuaBridge
    luabridge::getGlobalNamespace(L)
        .beginClass<Rigidbody>("Rigidbody")
        .addProperty("x", &Rigidbody::x)
        .addProperty("y", &Rigidbody::y)
        .addProperty("body_type", &Rigidbody::body_type)
        .addProperty("precise", &Rigidbody::precise)
        .addProperty("gravity_scale", &Rigidbody::gravity_scale)
        .addProperty("density", &Rigidbody::density)
        .addProperty("angular_friction", &Rigidbody::angular_friction)
        .addProperty("rotation", &Rigidbody::rotation)
        .addProperty("has_collider", &Rigidbody::has_collider)
        .addProperty("collider_type", &Rigidbody::collider_type)
        .addProperty("width", &Rigidbody::width)
        .addProperty("height", &Rigidbody::height)
        .addProperty("radius", &Rigidbody::radius)
        .addProperty("friction", &Rigidbody::friction)
        .addProperty("bounciness", &Rigidbody::bounciness)
        .addProperty("has_trigger", &Rigidbody::has_trigger)
        .addProperty("trigger_type", &Rigidbody::trigger_type)
        .addProperty("trigger_width", &Rigidbody::trigger_width)
        .addProperty("trigger_height", &Rigidbody::trigger_height)
        .addProperty("trigger_radius", &Rigidbody::trigger_radius)
        .addProperty("enabled", &Rigidbody::enabled)
        .addProperty("key", &Rigidbody::key)
        .addProperty("type", &Rigidbody::type)
        .addFunction("GetPosition", &Rigidbody::GetPosition)
        .addFunction("GetRotation", &Rigidbody::GetRotation)
        .addFunction("GetVelocity", &Rigidbody::GetVelocity)
        .addFunction("GetAngularVelocity", &Rigidbody::GetAngularVelocity)
        .addFunction("GetGravityScale", &Rigidbody::GetGravityScale)
        .addFunction("GetUpDirection", &Rigidbody::GetUpDirection)
        .addFunction("GetRightDirection", &Rigidbody::GetRightDirection)
        .addFunction("AddForce", &Rigidbody::AddForce)
        .addFunction("SetVelocity", &Rigidbody::SetVelocity)
        .addFunction("SetPosition", &Rigidbody::SetPosition)
        .addFunction("SetRotation", &Rigidbody::SetRotation)
        .addFunction("SetAngularVelocity", &Rigidbody::SetAngularVelocity)
        .addFunction("SetGravityScale", &Rigidbody::SetGravityScale)
        .addFunction("SetUpDirection", &Rigidbody::SetUpDirection)
        .addFunction("SetRightDirection", &Rigidbody::SetRightDirection)
        .addFunction("OnDestroy", &Rigidbody::OnDestroy)
        .endClass();

    luabridge::getGlobalNamespace(L)
        .beginClass<Collision>("Collision")
        .addProperty("other", &Collision::other)
        .addProperty("point", &Collision::point)
        .addProperty("relative_velocity", &Collision::relative_velocity)
        .addProperty("normal", &Collision::normal)
        .endClass();

    luabridge::getGlobalNamespace(L)
        .beginClass<HitResult>("HitResult")
        .addProperty("actor", &HitResult::actor)
        .addProperty("point", &HitResult::point)
        .addProperty("normal", &HitResult::normal)
        .addProperty("is_trigger", &HitResult::is_trigger)
        .endClass();

    luabridge::getGlobalNamespace(L)
        .beginNamespace("Physics")
        .addFunction("Raycast", &Physics::Raycast)
        .addFunction("RaycastAll", &Physics::RaycastAll)
        .endNamespace();

    luabridge::getGlobalNamespace(L)
        .beginNamespace("Event")
        .addFunction("Publish", &EventBus::Publish)
        .addFunction("Subscribe", &EventBus::Subscribe)
        .addFunction("Unsubscribe", &EventBus::Unsubscribe)
        .endNamespace();

    luabridge::getGlobalNamespace(L)
        .beginClass<ParticleSystem>("ParticleSystem")
        .addProperty("enabled", &ParticleSystem::enabled)
        .addProperty("key", &ParticleSystem::key)
        .addProperty("type", &ParticleSystem::type)
        .addProperty("x", &ParticleSystem::x)
        .addProperty("y", &ParticleSystem::y)
        .addProperty("frames_between_bursts", &ParticleSystem::frames_between_bursts)
        .addProperty("burst_quantity", &ParticleSystem::burst_quantity)
        .addProperty("start_scale_min", &ParticleSystem::start_scale_min)
        .addProperty("start_scale_max", &ParticleSystem::start_scale_max)
        .addProperty("rotation_min", &ParticleSystem::rotation_min)
        .addProperty("rotation_max", &ParticleSystem::rotation_max)
        .addProperty("rotation_speed_min", &ParticleSystem::rotation_speed_min)
        .addProperty("rotation_speed_max", &ParticleSystem::rotation_speed_max)
        .addProperty("start_speed_min", &ParticleSystem::start_speed_min)
        .addProperty("start_speed_max", &ParticleSystem::start_speed_max)
        .addProperty("gravity_scale_x", &ParticleSystem::gravity_scale_x)
        .addProperty("gravity_scale_y", &ParticleSystem::gravity_scale_y)
        .addProperty("drag_factor", &ParticleSystem::drag_factor)
        .addProperty("angular_drag_factor", &ParticleSystem::angular_drag_factor)
        .addProperty("start_color_r", &ParticleSystem::start_color_r)
        .addProperty("start_color_g", &ParticleSystem::start_color_g)
        .addProperty("start_color_b", &ParticleSystem::start_color_b)
        .addProperty("start_color_a", &ParticleSystem::start_color_a)
        .addProperty("emit_radius_min", &ParticleSystem::emit_radius_min)
        .addProperty("emit_radius_max", &ParticleSystem::emit_radius_max)
        .addProperty("emit_angle_min", &ParticleSystem::emit_angle_min)
        .addProperty("emit_angle_max", &ParticleSystem::emit_angle_max)
        .addProperty("duration_frames", &ParticleSystem::duration_frames)
        .addProperty("image", &ParticleSystem::image)
        .addProperty("sorting_order", &ParticleSystem::sorting_order)
        .addProperty("emission_allowed", &ParticleSystem::emission_allowed)
        .addFunction("Stop", &ParticleSystem::Stop)
        .addFunction("Play", &ParticleSystem::Play)
        .addFunction("Burst", &ParticleSystem::Burst)
        .endClass();
}

lua_State* ComponentDB::GetLuaState() {
    return lua_state;
}

bool ComponentDB::LoadComponentType(lua_State* L, const string& type) {
    // Rigidbody is a built-in C++ component, not a Lua script
    if (type == "Rigidbody") return true;

    if (component_types.find(type) != component_types.end()) return true;

    string path = "resources/component_types/" + type + ".lua";
    if (!fs::exists(path)) {
        cout << "error: failed to locate component " << type;
        exit(0);
    }

    if (luaL_dofile(L, path.c_str()) != LUA_OK) {
        cout << "problem with lua file " << type;
        exit(0);
    }

    luabridge::LuaRef parent_table = luabridge::getGlobal(L, type.c_str());
    if (!parent_table.isTable()) {
        cout << "error: " << type << " is not a Lua table";
        exit(0);
    }

    component_types.emplace(type, parent_table);
    return true;
}

luabridge::LuaRef ComponentDB::CreateInstance(lua_State* L, const string& type, const string& key) {
    // Special path: Rigidbody is a C++ component, wrap it directly
    if (type == "Rigidbody") {
        Rigidbody* rb = new Rigidbody();
        rb->key = key;
        rb->type = "Rigidbody";
        rb->enabled = true;
        return luabridge::LuaRef(L, rb);
    }

    if (type == "ParticleSystem") {
        ParticleSystem* ps = new ParticleSystem();
        ps->key = key;
        ps->type = "ParticleSystem";
        ps->enabled = true;
        return luabridge::LuaRef(L, ps);
    }

    LoadComponentType(L, type);

    luabridge::LuaRef parent_table = component_types.at(type);
    luabridge::LuaRef instance_table = luabridge::newTable(L);

    EstablishInheritance(L, instance_table, parent_table);
    instance_table["key"] = key;
    instance_table["type"] = type;

    return instance_table;
}

void ComponentDB::EstablishInheritance(lua_State* L, const luabridge::LuaRef& instance_table, const luabridge::LuaRef& parent_table) {
    luabridge::LuaRef mt = luabridge::newTable(L);
    mt["__index"] = parent_table;

    instance_table.push(L);
    mt.push(L);
    lua_setmetatable(L, -2);
    lua_pop(L, 1);
}

void ComponentDB::QueueOnStart(const luabridge::LuaRef& instance) {
    if (!instance.isNil()) {
        onstart_queue.push_back(instance);
    }
}

void ComponentDB::RunOnStartQueue() {
    std::vector<luabridge::LuaRef> processing_queue;
    processing_queue.swap(onstart_queue);

    for (auto& ref : processing_queue) {
        if (ref.isNil()) continue;

        // --- C++ Rigidbody path ---
        // isUserdata() is true for C++ objects pushed via LuaBridge.
        // This check must come BEFORE any ref["..."] access.

        if (ref.isUserdata()) {
            Rigidbody* rb = nullptr;
            try { rb = ref.cast<Rigidbody*>(); }
            catch (...) {}
            if (rb && rb->enabled) { rb->OnStart(); continue; }

            ParticleSystem* ps = nullptr;
            try { ps = ref.cast<ParticleSystem*>(); }
            catch (...) {}
            if (ps && ps->enabled) { ps->OnStart(); continue; }

            continue;
        }

        // --- Lua component path (safe: ref is a table) ---
        luabridge::LuaRef enabled_ref = ref["enabled"];
        if (enabled_ref.isNil() || !enabled_ref.cast<bool>()) continue;

        luabridge::LuaRef started_ref = ref["started"];
        if (!started_ref.isNil() && started_ref.cast<bool>()) continue;

        ref["started"] = true;
        try {
            luabridge::LuaRef on_start = ref["OnStart"];
            if (!on_start.isNil() && on_start.isFunction())
                on_start(ref);
        }
        catch (const luabridge::LuaException& e) {
            std::string actor_name = "Unknown Actor";
            luabridge::LuaRef actor_ref = ref["actor"];
            if (!actor_ref.isNil()) {
                Actor* actor_ptr = actor_ref.cast<Actor*>();
                if (actor_ptr) actor_name = actor_ptr->GetName();
            }
            ComponentDB::ReportError(actor_name, e);
        }
    }
}

void ComponentDB::CppLog(const string& message) { cout << message << endl; }
void ComponentDB::CppLogError(const string& message) { cout << message << endl; }

void ComponentDB::SetActorList(vector<Actor*>& actor_list) {
    actors = &actor_list;
}

vector<Actor*>& ComponentDB::GetAllActors() {
    if (!actors) {
        static vector<Actor*> empty_list;
        return empty_list;
    }
    return *actors;
}

void ComponentDB::Clear() {
    onstart_queue.clear();
    component_types.clear();
    actors = nullptr;
}

luabridge::LuaRef ComponentDB::Lua_Find(std::string name) {
    lua_State* L = ComponentDB::lua_state;

    if (actors) {
        for (auto* actor_ptr : *actors) {
            if (actor_ptr->GetName() == name && !actor_ptr->marked_for_destruction)
                return luabridge::LuaRef(L, actor_ptr);
        }
    }

    for (auto* actor_ptr : Engine::actors_to_spawn) {
        if (actor_ptr->GetName() == name && !actor_ptr->marked_for_destruction)
            return luabridge::LuaRef(L, actor_ptr);
    }

    return luabridge::LuaRef(L);
}

luabridge::LuaRef ComponentDB::Lua_FindAll(std::string name) {
    lua_State* L = ComponentDB::lua_state;
    luabridge::LuaRef table = luabridge::newTable(L);
    int index = 1;

    if (actors) {
        for (auto* actor_ptr : *actors) {
            if (actor_ptr->GetName() == name && !actor_ptr->marked_for_destruction)
                table[index++] = actor_ptr;
        }
    }

    for (auto* actor_ptr : Engine::actors_to_spawn) {
        if (actor_ptr->GetName() == name && !actor_ptr->marked_for_destruction)
            table[index++] = actor_ptr;
    }

    return table;
}

void ComponentDB::ReportError(const std::string& actor_name, const luabridge::LuaException& e) {
    std::string error_message = e.what();
    std::replace(error_message.begin(), error_message.end(), '\\', '/');
    std::cout << "\033[31m" << actor_name << " : " << error_message << "\033[0m" << std::endl;
}

void ComponentDB::InitializeActorFromTemplate(Actor* actor, const std::string& template_name) {
    if (template_name == "" || !TemplateDB::HasTemplate(template_name)) return;

    const rapidjson::Document& template_doc = TemplateDB::GetTemplate(template_name);
    if (!template_doc.HasMember("components") || !template_doc["components"].IsObject()) return;

    if (template_doc.HasMember("name") && template_doc["name"].IsString()) {
        actor->actor_name = template_doc["name"].GetString();
    }

    vector<string> keys;
    for (auto it = template_doc["components"].MemberBegin(); it != template_doc["components"].MemberEnd(); ++it) {
        keys.push_back(it->name.GetString());
    }
    sort(keys.begin(), keys.end());

    for (const string& key : keys) {
        const auto& comp_json = template_doc["components"][key.c_str()];
        if (!comp_json.HasMember("type") || !comp_json["type"].IsString()) continue;

        string type = comp_json["type"].GetString();
        luabridge::LuaRef instance = CreateInstance(lua_state, type, key);

        // Only set enabled/inject properties on Lua tables, not C++ objects
        if (instance.isTable()) {
            instance["enabled"] = true;
            InjectProperties(instance, comp_json);
        }
        else {
            // 1. Get the type string from the JSON to avoid "guessing" with casts
            string type = comp_json["type"].GetString();


            if (type == "Rigidbody") {
                Rigidbody* rb = nullptr;
                try {
                    rb = instance.cast<Rigidbody*>();
                }
                catch (...) {
                }

                if (rb) {
                    InjectRigidbodyProperties(rb, comp_json);
                }
            }
            else if (type == "ParticleSystem") {
                ParticleSystem* ps = nullptr;
                try {
                    ps = instance.cast<ParticleSystem*>();
                }
                catch (...) {
                }

                if (ps) {
                    InjectParticleSystemProperties(ps, comp_json);
                }
            }
        }

        auto ref_ptr = std::make_shared<luabridge::LuaRef>(instance);
        actor->InjectConvenienceReferences(ref_ptr);
        actor->components.emplace(key, ref_ptr);
        QueueOnStart(instance);
    }

    actor->RebuildFunctionCaches();
}

void ComponentDB::InjectProperties(luabridge::LuaRef& instance, const rapidjson::Value& json) {
    for (auto it = json.MemberBegin(); it != json.MemberEnd(); ++it) {
        std::string prop = it->name.GetString();
        if (prop == "type") continue;

        const auto& val = it->value;
        if (val.IsInt())         instance[prop] = val.GetInt();
        else if (val.IsFloat())  instance[prop] = val.GetFloat();
        else if (val.IsBool())   instance[prop] = val.GetBool();
        else if (val.IsString()) instance[prop] = val.GetString();
    }
}

void ComponentDB::InjectRigidbodyProperties(Rigidbody* rb, const rapidjson::Value& json) {
    for (auto it = json.MemberBegin(); it != json.MemberEnd(); ++it) {
        std::string prop = it->name.GetString();
        if (prop == "type") continue;

        const auto& val = it->value;
        if      (prop == "x"               && val.IsNumber())  rb->x               = val.GetFloat();
        else if (prop == "y"               && val.IsNumber())  rb->y               = val.GetFloat();
        else if (prop == "body_type"       && val.IsString())  rb->body_type       = val.GetString();
        else if (prop == "precise"         && val.IsBool())    rb->precise         = val.GetBool();
        else if (prop == "gravity_scale"   && val.IsNumber())  rb->gravity_scale   = val.GetFloat();
        else if (prop == "density"         && val.IsNumber())  rb->density         = val.GetFloat();
        else if (prop == "angular_friction"&& val.IsNumber())  rb->angular_friction= val.GetFloat();
        else if (prop == "rotation"        && val.IsNumber())  rb->rotation        = val.GetFloat();
        else if (prop == "has_collider"    && val.IsBool())    rb->has_collider    = val.GetBool();
        else if (prop == "collider_type"   && val.IsString())  rb->collider_type   = val.GetString();
        else if (prop == "width"           && val.IsNumber())  rb->width           = val.GetFloat();
        else if (prop == "height"          && val.IsNumber())  rb->height          = val.GetFloat();
        else if (prop == "radius"          && val.IsNumber())  rb->radius          = val.GetFloat();
        else if (prop == "friction"        && val.IsNumber())  rb->friction        = val.GetFloat();
        else if (prop == "bounciness"      && val.IsNumber())  rb->bounciness      = val.GetFloat();
        else if (prop == "has_trigger"     && val.IsBool())    rb->has_trigger     = val.GetBool();
        else if (prop == "trigger_type"    && val.IsString())  rb->trigger_type    = val.GetString();
        else if (prop == "trigger_width"   && val.IsNumber())  rb->trigger_width   = val.GetFloat();
        else if (prop == "trigger_height"  && val.IsNumber())  rb->trigger_height  = val.GetFloat();
        else if (prop == "trigger_radius"  && val.IsNumber())  rb->trigger_radius  = val.GetFloat();
        else if (prop == "enabled"         && val.IsBool())    rb->enabled         = val.GetBool();
    }
}

void ComponentDB::InjectParticleSystemProperties(ParticleSystem* ps, const rapidjson::Value& json) {
    for (auto it = json.MemberBegin(); it != json.MemberEnd(); ++it) {
        std::string prop = it->name.GetString();
        if (prop == "type") continue;

        const auto& val = it->value;
        if      (prop == "x"                     && val.IsNumber()) ps->x                     = val.GetFloat();
        else if (prop == "y"                     && val.IsNumber()) ps->y                     = val.GetFloat();
        else if (prop == "frames_between_bursts" && val.IsInt())    ps->frames_between_bursts = val.GetInt();
        else if (prop == "burst_quantity"        && val.IsInt())    ps->burst_quantity        = val.GetInt();
        else if (prop == "start_scale_min"       && val.IsNumber()) ps->start_scale_min       = val.GetFloat();
        else if (prop == "start_scale_max"       && val.IsNumber()) ps->start_scale_max       = val.GetFloat();
        else if (prop == "rotation_min"          && val.IsNumber()) ps->rotation_min          = val.GetFloat();
        else if (prop == "rotation_max"          && val.IsNumber()) ps->rotation_max          = val.GetFloat();
        else if (prop == "start_color_r"         && val.IsInt())    ps->start_color_r         = val.GetInt();
        else if (prop == "start_color_g"         && val.IsInt())    ps->start_color_g         = val.GetInt();
        else if (prop == "start_color_b"         && val.IsInt())    ps->start_color_b         = val.GetInt();
        else if (prop == "start_color_a"         && val.IsInt())    ps->start_color_a         = val.GetInt();
        else if (prop == "emit_radius_min"       && val.IsNumber()) ps->emit_radius_min       = val.GetFloat();
        else if (prop == "emit_radius_max"       && val.IsNumber()) ps->emit_radius_max       = val.GetFloat();
        else if (prop == "emit_angle_min"        && val.IsNumber()) ps->emit_angle_min        = val.GetFloat();
        else if (prop == "emit_angle_max"        && val.IsNumber()) ps->emit_angle_max        = val.GetFloat();
        else if (prop == "image"                 && val.IsString()) ps->image                 = val.GetString();
        else if (prop == "sorting_order"         && val.IsInt())    ps->sorting_order         = val.GetInt();
        else if (prop == "enabled"               && val.IsBool())   ps->enabled               = val.GetBool();
        else if (prop == "duration_frames"       && val.IsInt())    ps->duration_frames       = val.GetInt();
        else if (prop == "start_speed_min"       && val.IsNumber()) ps->start_speed_min       = val.GetFloat();
        else if (prop == "start_speed_max"       && val.IsNumber()) ps->start_speed_max       = val.GetFloat();
        else if (prop == "rotation_speed_min"    && val.IsNumber()) ps->rotation_speed_min    = val.GetFloat();
        else if (prop == "rotation_speed_max"    && val.IsNumber()) ps->rotation_speed_max    = val.GetFloat();
        else if (prop == "gravity_scale_x"       && val.IsNumber()) ps->gravity_scale_x       = val.GetFloat();
        else if (prop == "gravity_scale_y"       && val.IsNumber()) ps->gravity_scale_y       = val.GetFloat();
        else if (prop == "drag_factor"           && val.IsNumber()) ps->drag_factor           = val.GetFloat();
        else if (prop == "angular_drag_factor"   && val.IsNumber()) ps->angular_drag_factor   = val.GetFloat();
        else if (prop == "end_scale"             && val.IsNumber()) ps->end_scale             = val.GetFloat();
        else if (prop == "end_color_r"           && val.IsInt())    ps->end_color_r           = val.GetInt();
        else if (prop == "end_color_g"           && val.IsInt())    ps->end_color_g           = val.GetInt();
        else if (prop == "end_color_b"           && val.IsInt())    ps->end_color_b           = val.GetInt();
        else if (prop == "end_color_a"           && val.IsInt())    ps->end_color_a           = val.GetInt();
    }
}