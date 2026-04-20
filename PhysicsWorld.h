#ifndef PHYSICSWORLD_H
#define PHYSICSWORLD_H

#include "box2d/box2d.h"
#include "ContactListener.h"

class PhysicsWorld {
public:
    static void Init() {
        if (world == nullptr) {
            world = new b2World(b2Vec2(0.0f, 9.8f));
            world->SetContactListener(&contact_listener);
        }
    }

    static b2World* GetWorld() { return world; }

    static void Step() {
        if (world)
            world->Step(1.0f / 60.0f, 8, 3);
    }

    static bool HasWorld() { return world != nullptr; }

private:
    static inline b2World* world = nullptr;
    static inline ContactListener contact_listener;
};

#endif