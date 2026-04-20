#include "Engine.h"
#include "EngineUtils.h"
#include "SceneDB.h"
#include "TemplateDB.h"
#include "ImageDB.h"
#include "Helper.h"
#include "TextDB.h"
#include "AudioDB.h"
#include "Input.h"
#include "ComponentDB.h"
#include "Camera.h"
#include "Scene.h"
#include "PhysicsWorld.h"
#include "EventBus.h"
#include "ParticleSystem.h"

#include <iostream>
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <chrono>
#include <unordered_set>
#include "SDL2/SDL_stdinc.h"
#include "SDL2/SDL.h"
#include <memory>
#include "lua.hpp"

//windows 
using namespace std;

Engine::Engine()
    : game_running(true)
{
    namespace fs = std::filesystem;

    if (!fs::exists("resources")) {
        cout << "error: resources/ missing";
        exit(0);
    }

    if (!fs::exists("resources/game.config")) {
        cout << "error: resources/game.config missing";
        exit(0);
    }

    // initialize Lua state
    L = luaL_newstate();
    if (L == nullptr) {
        cout << "error: failed to create lua state";
        exit(0);
    }
    luaL_openlibs(L);

    ComponentDB::Init(L);

    //load in templates here 
    TemplateDB::LoadTemplates();

    rapidjson::Document game_doc;
    EngineUtils::ReadJsonFile("resources/game.config", game_doc);

    std::string game_title = "";
    if (game_doc.HasMember("game_title") && game_doc["game_title"].IsString()) {
        game_title = game_doc["game_title"].GetString();
    }

    Uint8 clear_r = 255;
    Uint8 clear_g = 255;
    Uint8 clear_b = 255;


    if (fs::exists("resources/rendering.config")) {
        rapidjson::Document render_doc;
        EngineUtils::ReadJsonFile("resources/rendering.config", render_doc);

        if (render_doc.HasMember("x_resolution") && render_doc["x_resolution"].IsInt()) {
            window_width = render_doc["x_resolution"].GetInt();
        }

        if (render_doc.HasMember("y_resolution") && render_doc["y_resolution"].IsInt()) {
            window_height = render_doc["y_resolution"].GetInt();
        }

        if (render_doc.HasMember("clear_color_r") && render_doc["clear_color_r"].IsInt()) {
            clear_r = static_cast<Uint8>(render_doc["clear_color_r"].GetInt());
        }
        if (render_doc.HasMember("clear_color_g") && render_doc["clear_color_g"].IsInt()) {
            clear_g = static_cast<Uint8>(render_doc["clear_color_g"].GetInt());
        }
        if (render_doc.HasMember("clear_color_b") && render_doc["clear_color_b"].IsInt()) {
            clear_b = static_cast<Uint8>(render_doc["clear_color_b"].GetInt());
        }

        if (render_doc.HasMember("zoom_factor")) {
            Camera::SetZoom(render_doc["zoom_factor"].GetFloat());
        }
        if (render_doc.HasMember("camera_x")) {
            Camera::x = render_doc["camera_x"].GetFloat();
        }
        if (render_doc.HasMember("camera_y")) {
            Camera::y = render_doc["camera_y"].GetFloat();
        }
    }

    renderer = make_unique<Renderer>(game_title, window_width, window_height, clear_r, clear_g, clear_b);
    
    ImageDB::Init(renderer->GetSDLRenderer());

    TextDB::Init();
    AudioDB::Init();

    if (!game_doc.HasMember("initial_scene") || !game_doc["initial_scene"].IsString()) {
        cout << "error: initial_scene unspecified";
        exit(0);
    }

    string scene_name = game_doc["initial_scene"].GetString();

    SceneDB::LoadScene(L, scene_name, actors);
    Scene::SetCurrentSceneName(scene_name);

    ComponentDB::SetActorList(actors);

    SDL_GameControllerAddMapping(
        "03000000380700006652000000000000,"
        "Logitech G29 Driving Force Racing Wheel,"
        "x:b0,a:b1,b:b2,y:b3,"
        "leftshoulder:b4,rightshoulder:b5,"
        "lefttrigger:b6,righttrigger:b7,"
        "back:b8,start:b9,"
        "rightstick:b10,leftstick:b11,"
        "dpup:h0.1,dpright:h0.2,dpdown:h0.4,dpleft:h0.8,"
        "leftx:a0,lefty:a1,rightx:a2,righty:a3,"
        "platform:Windows"
    );

    SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER);
    Input::Init();
    //ComponentDB::RunOnStartQueue();
}

Engine::~Engine() {
    EventBus::Clear();

    if (L != nullptr) {
        ComponentDB::Clear();
        for (auto& actor : actors) {
            actor->components.clear();
        }
        lua_close(L);
        L = nullptr;
    }
}

void Engine::CallComponentFunction(const std::string& func_name) {
    bool is_update = (func_name == "OnUpdate");

    for (auto* actor : actors) {
        auto& comp_list = is_update
            ? actor->components_with_update
            : actor->components_with_late_update;

        for (auto& ref_ptr : comp_list) {
            luabridge::LuaRef& comp = *ref_ptr;

            if (!comp["enabled"].cast<bool>()) continue;
            if (comp["started"].isNil() || !comp["started"].cast<bool>()) continue;

            try {
                comp[func_name](comp);
            }
            catch (const luabridge::LuaException& e) {
                ComponentDB::ReportError(actor->GetName(), e);
            }
        }
    }
}

void Engine::GameLoop() {

    SDL_Event event;

    while (game_running) {

        if (Scene::IsSceneChangePending()) {
            string next_scene = Scene::GetPendingSceneName();

            EventBus::Clear();

            for (auto it = actors.begin(); it != actors.end(); ) {
                if (!(*it)->dont_destroy_on_load) {
                    (*it)->OnDestroy();

                    delete* it;
                    it = actors.erase(it);
                }
                else {
                    ++it;
                }
            }

            vector<Actor*> new_actors;
            SceneDB::LoadScene(L, next_scene, new_actors);
            actors.insert(actors.end(), new_actors.begin(), new_actors.end());

            Scene::SetCurrentSceneName(next_scene);
            Scene::ClearPendingScene();
        }

        // Spawn queued actors
        for (auto* a : actors_to_spawn) {
            actors.push_back(a);
        }
        actors_to_spawn.clear();

        Input::NewFrame();

        while (Helper::SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT)
                game_running = false;
            Input::ProcessEvent(event);
        }

        ComponentDB::RunOnStartQueue();

        // Rebuild caches for actors modified last frame
        for (auto* actor : actors) {
            if (actor->cache_dirty) {
                actor->RebuildFunctionCaches();
                actor->cache_dirty = false;
            }
        }

        CallComponentFunction("OnUpdate");
        CallComponentFunction("OnLateUpdate");

        EventBus::ProcessPendingSubscriptions();

        // Update all particle systems
        for (auto* ps : ParticleSystem::active_systems) {
            if (ps->enabled) ps->OnUpdate();
        }

        // Step physics � after all actor updates, before rendering
        if (PhysicsWorld::HasWorld()) {
            PhysicsWorld::Step();
        }

        renderer->Clear();

        ImageDB::RenderAndClearAll(
            renderer->GetSDLRenderer(),
            Camera::GetZoom(),
            Camera::GetPositionX(),
            Camera::GetPositionY()
        );
        TextDB::RenderAll(renderer->GetSDLRenderer());

        renderer->Present();

        // Destroy queued actors
        for (auto* actor_to_kill : actors_to_destroy) { 

            auto it = std::find(actors.begin(), actors.end(), actor_to_kill);
            if (it != actors.end())
                actors.erase(it);

            auto it_spawn = std::find(actors_to_spawn.begin(), actors_to_spawn.end(), actor_to_kill);
            if (it_spawn != actors_to_spawn.end())
                actors_to_spawn.erase(it_spawn);

            delete actor_to_kill;
        }
        actors_to_destroy.clear();
    }

    TextDB::Quit();
    AudioDB::Quit();
    ImageDB::Quit();
}