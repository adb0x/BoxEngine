#include "Scene.h"

std::string Scene::current_scene_name = "";
std::string Scene::pending_scene_name = "";
bool Scene::scene_change_requested = false;

void Scene::Load(const std::string& scene_name) {
    pending_scene_name = scene_name;
    scene_change_requested = true;
}

std::string Scene::GetCurrent() {
    return current_scene_name;
}

void Scene::DontDestroy(Actor* actor) {
    if (actor) actor->dont_destroy_on_load = true;
}

bool Scene::IsSceneChangePending(){
    return scene_change_requested;
}

std::string Scene::GetPendingSceneName(){
    return pending_scene_name;
}

void Scene::ClearPendingScene(){
    scene_change_requested = false;
}

void Scene::SetCurrentSceneName(const std::string& name){
    current_scene_name = name;
}