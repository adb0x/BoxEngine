#ifndef RIGIDBODY_H
#define RIGIDBODY_H

#include "box2d/box2d.h"
#include <string>
#include "Actor.h"

class Rigidbody {
public:
    Actor* actor = nullptr;

    float x = 0.0f;
    float y = 0.0f;
    std::string body_type = "dynamic";
    bool precise = true;
    float gravity_scale = 1.0f;
    float density = 1.0f;
    float angular_friction = 0.3f;
    float rotation = 0.0f;

    // Collider properties
    bool has_collider = true;
    std::string collider_type = "box";
    float width = 1.0f;
    float height = 1.0f;
    float radius = 0.5f;
    float friction = 0.3f;
    float bounciness = 0.3f;

    // Trigger properties
    bool has_trigger = true;
    std::string trigger_type = "box";
    float trigger_width = 1.0f;
    float trigger_height = 1.0f;
    float trigger_radius = 0.5f;


    bool enabled = true;
    std::string key = "";
    std::string type = "Rigidbody";

    b2Body* body = nullptr;


    void OnStart();

    void OnDestroy();

    // Getters
    b2Vec2 GetPosition() const;
    float  GetRotation() const;
    b2Vec2 GetVelocity() const;
    float  GetAngularVelocity() const;
    float  GetGravityScale() const;
    b2Vec2 GetUpDirection() const;
    b2Vec2 GetRightDirection() const;

    // Setters / forces
    void AddForce(const b2Vec2& force);
    void SetVelocity(const b2Vec2& vel);
    void SetPosition(const b2Vec2& pos);
    void SetRotation(float degrees_clockwise);
    void SetAngularVelocity(float degrees_clockwise);
    void SetGravityScale(float scale);
    void SetUpDirection(b2Vec2 direction);
    void SetRightDirection(b2Vec2 direction);
};

#endif