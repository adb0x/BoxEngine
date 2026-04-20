#include "Rigidbody.h"
#include "PhysicsWorld.h"
#include "glm/glm.hpp"

static const float DEG_TO_RAD = b2_pi / 180.0f;
static const float RAD_TO_DEG = 180.0f / b2_pi;

void Rigidbody::OnStart() {
    PhysicsWorld::Init();

    b2BodyDef body_def;

    if (body_type == "dynamic")         body_def.type = b2_dynamicBody;
    else if (body_type == "static")     body_def.type = b2_staticBody;
    else if (body_type == "kinematic")  body_def.type = b2_kinematicBody;
    else                                body_def.type = b2_dynamicBody;

    body_def.position.Set(x, y);
    body_def.angle = (rotation * DEG_TO_RAD);
    body_def.bullet = precise;
    body_def.gravityScale = gravity_scale;
    body_def.angularDamping = angular_friction;

    body = PhysicsWorld::GetWorld()->CreateBody(&body_def);

    // Collider first, then trigger
    if (has_collider) {
        b2FixtureDef fixture_def;
        fixture_def.isSensor = false;
        fixture_def.density = density;
        fixture_def.friction = friction;
        fixture_def.restitution = bounciness;

        // Category 0x0001 = collider, only collide with other colliders
        fixture_def.filter.categoryBits = 0x0001;
        fixture_def.filter.maskBits = 0x0001;

        if (collider_type == "box") {
            b2PolygonShape shape;
            shape.SetAsBox(width * 0.5f, height * 0.5f);
            fixture_def.shape = &shape;
            b2Fixture* f = body->CreateFixture(&fixture_def);
            f->GetUserData().pointer = reinterpret_cast<uintptr_t>(actor);
        }
        else if (collider_type == "circle") {
            b2CircleShape shape;
            shape.m_radius = radius;
            fixture_def.shape = &shape;
            b2Fixture* f = body->CreateFixture(&fixture_def);
            f->GetUserData().pointer = reinterpret_cast<uintptr_t>(actor);
        }
    }

    if (has_trigger) {
        b2FixtureDef trigger_fixture_def;
        trigger_fixture_def.isSensor = true;
        trigger_fixture_def.density = density;

        // Category 0x0002 = trigger, only collide with other triggers
        trigger_fixture_def.filter.categoryBits = 0x0002;
        trigger_fixture_def.filter.maskBits = 0x0002;

        if (trigger_type == "box") {
            b2PolygonShape shape;
            shape.SetAsBox(trigger_width * 0.5f, trigger_height * 0.5f);
            trigger_fixture_def.shape = &shape;
            b2Fixture* f = body->CreateFixture(&trigger_fixture_def);
            f->GetUserData().pointer = reinterpret_cast<uintptr_t>(actor);
        }
        else if (trigger_type == "circle") {
            b2CircleShape shape;
            shape.m_radius = trigger_radius;
            trigger_fixture_def.shape = &shape;
            b2Fixture* f = body->CreateFixture(&trigger_fixture_def);
            f->GetUserData().pointer = reinterpret_cast<uintptr_t>(actor);
        }
    }

    if (!has_collider && !has_trigger) {
        // Phantom sensor
        b2PolygonShape phantom_shape;
        phantom_shape.SetAsBox(width * 0.5f, height * 0.5f);

        b2FixtureDef phantom_fixture_def;
        phantom_fixture_def.shape = &phantom_shape;
        phantom_fixture_def.density = density;

        phantom_fixture_def.isSensor = true;
        
        body->CreateFixture(&phantom_fixture_def);
    }
}


void Rigidbody::OnDestroy() {
    if (body && PhysicsWorld::HasWorld()) {
        PhysicsWorld::GetWorld()->DestroyBody(body);
        body = nullptr;
    }
}

// --- Getters ---

b2Vec2 Rigidbody::GetPosition() const {
    if (body == nullptr)
        return b2Vec2(x, y);
    return body->GetPosition();
}

float Rigidbody::GetRotation() const {
    if (body) return (body->GetAngle() * RAD_TO_DEG);
    return rotation;
}

b2Vec2 Rigidbody::GetVelocity() const {
    if (body) return body->GetLinearVelocity();
    return b2Vec2(0.0f, 0.0f);
}

float Rigidbody::GetAngularVelocity() const {
    if (body) return body->GetAngularVelocity() * RAD_TO_DEG;
    return 0.0f;
}

float Rigidbody::GetGravityScale() const {
    if (body) return body->GetGravityScale();
    return gravity_scale;
}

b2Vec2 Rigidbody::GetUpDirection() const {
    if (!body) return b2Vec2(0.0f, -1.0f);
    float angle = body->GetAngle();
    b2Vec2 result = b2Vec2(glm::sin(angle), -glm::cos(angle));
    result.Normalize();
    return result;
}

b2Vec2 Rigidbody::GetRightDirection() const {
    if (!body) return b2Vec2(1.0f, 0.0f);
    float angle = body->GetAngle();
    b2Vec2 result = b2Vec2(glm::cos(angle), glm::sin(angle));
    result.Normalize();
    return result;
}

// --- Setters / Forces ---

void Rigidbody::AddForce(const b2Vec2& force) {
    if (body) body->ApplyForceToCenter(force, true);
}

void Rigidbody::SetVelocity(const b2Vec2& vel) {
    if (body) body->SetLinearVelocity(vel);
}

void Rigidbody::SetPosition(const b2Vec2& pos) {
    if (body == nullptr) {
        x = pos.x;
        y = pos.y;
    }
    else {
        body->SetTransform(pos, body->GetAngle());
    }
}

void Rigidbody::SetRotation(float degrees_clockwise) {
    if (body) body->SetTransform(body->GetPosition(), degrees_clockwise * DEG_TO_RAD);
}

void Rigidbody::SetAngularVelocity(float degrees_clockwise) {
    if (body) body->SetAngularVelocity(degrees_clockwise * DEG_TO_RAD);
}

void Rigidbody::SetGravityScale(float scale) {
    if (body) body->SetGravityScale(scale);
}

void Rigidbody::SetUpDirection(b2Vec2 direction) {
    direction.Normalize();
    float new_angle_radians = glm::atan(direction.x, -direction.y);
    if (body) body->SetTransform(body->GetPosition(), new_angle_radians);
}

void Rigidbody::SetRightDirection(b2Vec2 direction) {
    direction.Normalize();
    float new_angle_radians = glm::atan(direction.x, -direction.y) - b2_pi / 2.0f;
    if (body) body->SetTransform(body->GetPosition(), new_angle_radians);
}