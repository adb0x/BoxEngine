#ifndef PHYSICS_H
#define PHYSICS_H

#include "box2d/box2d.h"
#include "HitResult.h"
#include "lua.hpp"
#include "LuaBridge.h"

class RaycastCallback : public b2RayCastCallback {
public:
    // For Raycast (closest hit only)
    HitResult closest_hit;
    bool hit_found = false;
    float closest_fraction = 1.0f;

    float ReportFixture(b2Fixture* fixture, const b2Vec2& point,
        const b2Vec2& normal, float fraction) override {
        Actor* actor = reinterpret_cast<Actor*>(fixture->GetUserData().pointer);

        // Ignore phantom fixtures (no actor pointer set)
        if (!actor) return -1.0f;

        // Track closest hit
        if (fraction < closest_fraction) {
            closest_fraction = fraction;
            closest_hit.actor = actor;
            closest_hit.point = point;
            closest_hit.normal = normal;
            closest_hit.is_trigger = fixture->IsSensor();
            hit_found = true;
        }

        // Return 1 to continue and find all fixtures
        return 1.0f;
    }
};

class RaycastAllCallback : public b2RayCastCallback {
public:
    std::vector<std::pair<float, HitResult>> hits; // fraction, result

    float ReportFixture(b2Fixture* fixture, const b2Vec2& point,
        const b2Vec2& normal, float fraction) override {
        Actor* actor = reinterpret_cast<Actor*>(fixture->GetUserData().pointer);

        // Ignore phantom fixtures
        if (!actor) return -1.0f;

        HitResult hit;
        hit.actor = actor;
        hit.point = point;
        hit.normal = normal;
        hit.is_trigger = fixture->IsSensor();

        hits.push_back({ fraction, hit });

        // Return 1 to continue collecting all hits
        return 1.0f;
    }
};

class Physics {
public:
    static luabridge::LuaRef Raycast(b2Vec2 pos, b2Vec2 dir, float dist);
    static luabridge::LuaRef RaycastAll(b2Vec2 pos, b2Vec2 dir, float dist);
};

#endif