#ifndef SCENE_H
#define SCENE_H

#include <string>
#include "Actor.h"

class Scene {
public:
    static void Load(const std::string& scene_name);
    static std::string GetCurrent();
    static void DontDestroy(Actor* actor);

    // Engine-only helpers
    static bool IsSceneChangePending();
    static std::string GetPendingSceneName();
    static void ClearPendingScene();
    static void SetCurrentSceneName(const std::string& name);

private:
    static std::string current_scene_name;
    static std::string pending_scene_name;
    static bool scene_change_requested;
};

#endif