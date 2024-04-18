// Physics.cpp

#include "headers/RayCasting.h"

FirstHitRayCastCallback::FirstHitRayCastCallback()
    : hitResult(nullptr, b2Vec2_zero, b2Vec2_zero, 0.0f, false) {}

float FirstHitRayCastCallback::ReportFixture(b2Fixture* fixture, const b2Vec2& point, const b2Vec2& normal, float fraction) {
    Actor* actor = reinterpret_cast<Actor*>(fixture->GetUserData().pointer);
    if (actor == nullptr) {
        return -1.0f; // Skip fixtures with no associated actor
    }
    if (fraction == 0.0f) {
        return -1.0f;
    }
    // Include sensors in the results but mark them as triggers
    bool isSensor = fixture->IsSensor();
    hitResult = HitResult(actor, point, normal, fraction, isSensor);
    hit = true;

    // If it's a sensor, continue the raycast to find non-sensor hits
    // Otherwise, stop the raycast at this hit
    return isSensor ? 1.0f : fraction;
}


float AllHitsRayCastCallback::ReportFixture(b2Fixture* fixture, const b2Vec2& point, const b2Vec2& normal, float fraction) {
    // Retrieve the actor from the user data of the fixture
    Actor* actor = reinterpret_cast<Actor*>(fixture->GetUserData().pointer);

    // If there's no actor associated with the fixture, skip it
    if (actor == nullptr) {
        return -1.0f;
    }
    if (fraction == 0.0f) {
        return -1.0f;
    }

    // Include the hit in the results, whether it's a sensor or not
    bool isSensor = fixture->IsSensor();
    hits.emplace_back(actor, point, normal, fraction, isSensor);

    // Continue the raycast to report all hits
    return isSensor ? 1.0f : fraction;
}

b2Vec2 toB2Vec2(const Vector2& vector) {
    return b2Vec2(vector.getX(), vector.getY());
}

luabridge::LuaRef Physics::Raycast(Vector2 pos, Vector2 dir, float dist) {
    FirstHitRayCastCallback callback;
    Vector2 endPos = pos + dir * dist;
    RigidBody::world->RayCast(&callback, toB2Vec2(pos), toB2Vec2(endPos));

    if (callback.hit) {
        return luabridge::LuaRef(LuaHelper::L, callback.hitResult);
    }
    return luabridge::LuaRef(LuaHelper::L); // Return nil if nothing was hit
}

luabridge::LuaRef Physics::RaycastAll(Vector2 pos, Vector2 dir, float dist) {
    AllHitsRayCastCallback callback;
    Vector2 endPos = pos + dir * dist;
    RigidBody::world->RayCast(&callback, toB2Vec2(pos), toB2Vec2(endPos));

    // Sort the hits by the fraction (which indicates the distance along the ray)
    std::sort(callback.hits.begin(), callback.hits.end(),
        [](const HitResult& a, const HitResult& b) {
            return a.fraction < b.fraction;
        });


    luabridge::LuaRef hitsTable = luabridge::newTable(LuaHelper::L);
    for (size_t i = 0; i < callback.hits.size(); ++i) {
        hitsTable[i + 1] = luabridge::LuaRef(LuaHelper::L, callback.hits[i]);
    }
    return hitsTable; // Return a table of hits
}