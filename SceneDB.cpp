#include "SceneDB.h"
#include "EngineUtils.h"
#include "ComponentDB.h"
#include "TemplateDB.h"
#include "Rigidbody.h"

#include <filesystem>
#include <iostream>
#include <rapidjson/document.h>
#include <algorithm>

using namespace std;
namespace fs = std::filesystem;


bool SceneDB::LoadScene(lua_State* L, const string& scene_name, vector<Actor*>& out_actors) {
    string path = "resources/scenes/" + scene_name + ".scene";

    if (!fs::exists(path)) {
        cout << "error: scene " << scene_name << " missing\n";
        exit(0);
    }

    rapidjson::Document scene_doc;
    EngineUtils::ReadJsonFile(path, scene_doc);

    if (!scene_doc.HasMember("actors") || !scene_doc["actors"].IsArray()) {
        return false;
    }

    out_actors.clear();
    int next_id = 1;

    for (auto& actor_json : scene_doc["actors"].GetArray()) {

        string name = "";
        string template_name = "";

        if (actor_json.HasMember("name") && actor_json["name"].IsString()) {
            name = actor_json["name"].GetString();
        }

        if (actor_json.HasMember("template") && actor_json["template"].IsString()) {
            template_name = actor_json["template"].GetString();
        }

        Actor* actor = new Actor(next_id++, name, template_name);

        // STEP 1: Load template components
        if (template_name != "" && TemplateDB::HasTemplate(template_name)) {
            const rapidjson::Document& template_doc = TemplateDB::GetTemplate(template_name);

            if (template_doc.HasMember("components") && template_doc["components"].IsObject()) {
                vector<string> keys;
                for (auto it = template_doc["components"].MemberBegin();
                    it != template_doc["components"].MemberEnd(); ++it) {
                    keys.push_back(it->name.GetString());
                }
                sort(keys.begin(), keys.end());

                for (const string& key : keys) {
                    const auto& comp_json = template_doc["components"][key.c_str()];
                    if (!comp_json.HasMember("type") || !comp_json["type"].IsString()) continue;

                    string type = comp_json["type"].GetString();
                    luabridge::LuaRef instance = ComponentDB::CreateInstance(L, type, key);

                    if (instance.isTable()) {
                        instance["enabled"] = true;
                        ComponentDB::InjectProperties(instance, comp_json);
                    }
                    else {
                        Rigidbody* rb = nullptr;
                        try { rb = instance.cast<Rigidbody*>(); }
                        catch (...) {}
                        if (rb) ComponentDB::InjectRigidbodyProperties(rb, comp_json);

                        ParticleSystem* ps = nullptr;
                        try { ps = instance.cast<ParticleSystem*>(); }
                        catch (...) {}
                        if (ps) ComponentDB::InjectParticleSystemProperties(ps, comp_json);
                    }
                    auto ref_ptr = std::make_shared<luabridge::LuaRef>(instance);
                    actor->InjectConvenienceReferences(ref_ptr);
                    actor->components.emplace(key, ref_ptr);
                }
            }
        }

        // STEP 2: Apply scene components / overrides
        if (actor_json.HasMember("components") && actor_json["components"].IsObject()) {
            vector<string> keys;
            for (auto it = actor_json["components"].MemberBegin();
                it != actor_json["components"].MemberEnd(); ++it) {
                keys.push_back(it->name.GetString());
            }
            sort(keys.begin(), keys.end());

            for (const string& key : keys) {
                const auto& comp_json = actor_json["components"][key.c_str()];
                auto existing = actor->components.find(key);

                if (existing != actor->components.end()) {
                    // Override existing component
                    luabridge::LuaRef& instance = *(existing->second);
                    if (instance.isTable()) {
                        ComponentDB::InjectProperties(instance, comp_json);
                    }
                    else {
                        Rigidbody* rb = nullptr;
                        try { rb = instance.cast<Rigidbody*>(); }
                        catch (...) {}
                        if (rb) ComponentDB::InjectRigidbodyProperties(rb, comp_json);

                        ParticleSystem* ps = nullptr;
                        try { ps = instance.cast<ParticleSystem*>(); }
                        catch (...) {}
                        if (ps) ComponentDB::InjectParticleSystemProperties(ps, comp_json);
                    }
                }
                else {
                    // New component defined in scene
                    if (!comp_json.HasMember("type") || !comp_json["type"].IsString()) continue;

                    string type = comp_json["type"].GetString();
                    luabridge::LuaRef instance = ComponentDB::CreateInstance(L, type, key);

                    if (instance.isTable()) {
                        instance["enabled"] = true;
                        ComponentDB::InjectProperties(instance, comp_json);
                    }
                    else {
                        Rigidbody* rb = nullptr;
                        try { rb = instance.cast<Rigidbody*>(); }
                        catch (...) {}
                        if (rb) ComponentDB::InjectRigidbodyProperties(rb, comp_json);

                        ParticleSystem* ps = nullptr;
                        try { ps = instance.cast<ParticleSystem*>(); }
                        catch (...) {}
                        if (ps) ComponentDB::InjectParticleSystemProperties(ps, comp_json);
                    }

                    auto ref_ptr = std::make_shared<luabridge::LuaRef>(instance);
                    actor->InjectConvenienceReferences(ref_ptr);
                    actor->components.emplace(key, ref_ptr);
                }
            }
        }

        actor->RebuildFunctionCaches();

        // Queue OnStart for all components (sorted by key)
        for (auto& [key, ref_ptr] : actor->components) {
            ComponentDB::QueueOnStart(*ref_ptr);
        }

        out_actors.push_back(actor);
    }

    return true;
}