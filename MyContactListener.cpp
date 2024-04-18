#include "headers/MyContactListener.h"
#include "headers/Actor.h"

void ColliderDetector::BeginContact(b2Contact* contact) {
    // Determine whether this contact involves a sensor (trigger)
    bool isSensorContact = contact->GetFixtureA()->IsSensor() || contact->GetFixtureB()->IsSensor();

    Actor* actorA = reinterpret_cast<Actor*>(contact->GetFixtureA()->GetUserData().pointer);
    Actor* actorB = reinterpret_cast<Actor*>(contact->GetFixtureB()->GetUserData().pointer);

    // Prepare the Collision object
    Collision collision;
    if (isSensorContact) {
        // Set sentinel values for sensor contacts
        collision.point = b2Vec2(-999.0f, -999.0f);
        collision.normal = b2Vec2(-999.0f, -999.0f);
    }
    else {
        b2WorldManifold worldManifold;
        contact->GetWorldManifold(&worldManifold);
        collision.point = worldManifold.points[0];
        collision.normal = worldManifold.normal;
    }
    collision.other = actorB;
    collision.relative_velocity = contact->GetFixtureA()->GetBody()->GetLinearVelocity() - contact->GetFixtureB()->GetBody()->GetLinearVelocity();


    if (actorA == nullptr || actorB == nullptr) {
        return;
    }
    // Check if both fixtures are sensors
    if (contact->GetFixtureA()->IsSensor() && contact->GetFixtureB()->IsSensor()) {
        // Call the Lua trigger exit function for both actors
        CallLuaFunction(actorA, "OnTriggerEnter", collision);
        collision.other = actorA;
        CallLuaFunction(actorB, "OnTriggerEnter", collision);
    }
    // Check if neither fixture is a sensor
    else if (!contact->GetFixtureA()->IsSensor() && !contact->GetFixtureB()->IsSensor()) {
        // Call the Lua collision exit function for both actors
        CallLuaFunction(actorA, "OnCollisionEnter", collision);
        collision.other = actorA;
        CallLuaFunction(actorB, "OnCollisionEnter", collision);
    }
}


void ColliderDetector::EndContact(b2Contact* contact) {
    bool isSensorContact = contact->GetFixtureA()->IsSensor() || contact->GetFixtureB()->IsSensor();

    Actor* actorA = reinterpret_cast<Actor*>(contact->GetFixtureA()->GetUserData().pointer);
    Actor* actorB = reinterpret_cast<Actor*>(contact->GetFixtureB()->GetUserData().pointer);

    // For EndContact, we set point and normal to sentinel values regardless of sensor or collider
    Collision collision;
    collision.other = actorB; // For actor A, the other actor is B
    collision.point = b2Vec2(-999.0f, -999.0f); // Sentinel value for invalid point
    collision.relative_velocity = contact->GetFixtureA()->GetBody()->GetLinearVelocity() - contact->GetFixtureB()->GetBody()->GetLinearVelocity();
    collision.normal = b2Vec2(-999.0f, -999.0f); // Sentinel value for invalid normal


    if (actorA == nullptr || actorB == nullptr) {
        return;
    }

    // Check if both fixtures are sensors
    if (contact->GetFixtureA()->IsSensor() && contact->GetFixtureB()->IsSensor()) {
        // Call the Lua trigger exit function for both actors
        CallLuaFunction(actorA, "OnTriggerExit", collision);
        collision.other = actorA;
        CallLuaFunction(actorB, "OnTriggerExit", collision);
    }
    // Check if neither fixture is a sensor
    else if (!contact->GetFixtureA()->IsSensor() && !contact->GetFixtureB()->IsSensor()) {
        // Call the Lua collision exit function for both actors
        CallLuaFunction(actorA, "OnCollisionExit", collision);
        collision.other = actorA;
        CallLuaFunction(actorB, "OnCollisionExit", collision);
    }
}


void ColliderDetector::CallLuaFunction(Actor* actor, const char* functionName, const Collision& collision) {
    if (!actor) return;

    // Retrieve the actor's Lua component and call the specified function
    luabridge::LuaRef components = actor->GetComponents("CollisionResponder", LuaHelper::L);
    for (int i = 1; !components[i].isNil(); ++i) {
        luabridge::LuaRef component = components[i];
        luabridge::LuaRef luaFunction = component[functionName];
        if (luaFunction.isFunction()) {
            try {
                luaFunction(component, collision);
            }
            catch (const luabridge::LuaException& e) {
                std::cerr << "LuaException caught: " << e.what() << std::endl;
            }
        }
    }
}