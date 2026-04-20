#ifndef HITRESULT_H
#define HITRESULT_H

#include "box2d/box2d.h"

class Actor;

struct HitResult {
    Actor* actor = nullptr;
    b2Vec2 point;
    b2Vec2 normal;
    bool is_trigger = false;
};

#endif