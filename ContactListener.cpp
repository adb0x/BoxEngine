#include "ContactListener.h"
#include "Collision.h"
#include "Actor.h"
#include "ComponentDB.h"

void ContactListener::BeginContact(b2Contact* contact) {
    b2Fixture* fixture_a = contact->GetFixtureA();
    b2Fixture* fixture_b = contact->GetFixtureB();

    Actor* actor_a = reinterpret_cast<Actor*>(fixture_a->GetUserData().pointer);
    Actor* actor_b = reinterpret_cast<Actor*>(fixture_b->GetUserData().pointer);

    if (!actor_a || !actor_b) return;

    b2Vec2 relative_velocity = fixture_a->GetBody()->GetLinearVelocity()
        - fixture_b->GetBody()->GetLinearVelocity();

    bool a_is_trigger = fixture_a->IsSensor();
    bool b_is_trigger = fixture_b->IsSensor();

    if (a_is_trigger && b_is_trigger) {
        // Trigger-trigger: call OnTriggerEnter, point/normal are sentinels
        Collision col_a;
        col_a.other = actor_b;
        col_a.point = b2Vec2(-999.0f, -999.0f);
        col_a.relative_velocity = relative_velocity;
        col_a.normal = b2Vec2(-999.0f, -999.0f);

        Collision col_b;
        col_b.other = actor_a;
        col_b.point = b2Vec2(-999.0f, -999.0f);
        col_b.relative_velocity = relative_velocity;
        col_b.normal = b2Vec2(-999.0f, -999.0f);

        actor_a->OnCollision("OnTriggerEnter", col_a);
        actor_b->OnCollision("OnTriggerEnter", col_b);
    }
    else if (!a_is_trigger && !b_is_trigger) {
        // Collider-collider: call OnCollisionEnter with real manifold data
        b2WorldManifold world_manifold;
        contact->GetWorldManifold(&world_manifold);

        Collision col_a;
        col_a.other = actor_b;
        col_a.point = world_manifold.points[0];
        col_a.relative_velocity = relative_velocity;
        col_a.normal = world_manifold.normal;

        Collision col_b;
        col_b.other = actor_a;
        col_b.point = world_manifold.points[0];
        col_b.relative_velocity = relative_velocity;
        col_b.normal = world_manifold.normal;

        actor_a->OnCollision("OnCollisionEnter", col_a);
        actor_b->OnCollision("OnCollisionEnter", col_b);
    }
    // Mixed collider-trigger contacts are filtered out by categoryBits/maskBits
    // so this case should never occur, but we ignore it if it does
}

void ContactListener::EndContact(b2Contact* contact) {
    b2Fixture* fixture_a = contact->GetFixtureA();
    b2Fixture* fixture_b = contact->GetFixtureB();

    Actor* actor_a = reinterpret_cast<Actor*>(fixture_a->GetUserData().pointer);
    Actor* actor_b = reinterpret_cast<Actor*>(fixture_b->GetUserData().pointer);

    if (!actor_a || !actor_b) return;

    b2Vec2 relative_velocity = fixture_a->GetBody()->GetLinearVelocity()
        - fixture_b->GetBody()->GetLinearVelocity();

    bool a_is_trigger = fixture_a->IsSensor();
    bool b_is_trigger = fixture_b->IsSensor();

    if (a_is_trigger && b_is_trigger) {
        Collision col_a;
        col_a.other = actor_b;
        col_a.point = b2Vec2(-999.0f, -999.0f);
        col_a.relative_velocity = relative_velocity;
        col_a.normal = b2Vec2(-999.0f, -999.0f);

        Collision col_b;
        col_b.other = actor_a;
        col_b.point = b2Vec2(-999.0f, -999.0f);
        col_b.relative_velocity = relative_velocity;
        col_b.normal = b2Vec2(-999.0f, -999.0f);

        actor_a->OnCollision("OnTriggerExit", col_a);
        actor_b->OnCollision("OnTriggerExit", col_b);
    }
    else if (!a_is_trigger && !b_is_trigger) {
        Collision col_a;
        col_a.other = actor_b;
        col_a.point = b2Vec2(-999.0f, -999.0f);
        col_a.relative_velocity = relative_velocity;
        col_a.normal = b2Vec2(-999.0f, -999.0f);

        Collision col_b;
        col_b.other = actor_a;
        col_b.point = b2Vec2(-999.0f, -999.0f);
        col_b.relative_velocity = relative_velocity;
        col_b.normal = b2Vec2(-999.0f, -999.0f);

        actor_a->OnCollision("OnCollisionExit", col_a);
        actor_b->OnCollision("OnCollisionExit", col_b);
    }
}