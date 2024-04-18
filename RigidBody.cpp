#include "headers/RigidBody.h"

b2World* RigidBody::world;

RigidBody::RigidBody() {
   
}

RigidBody::RigidBody(const RigidBody& other)
    : type(other.type), key(other.key), actor(nullptr), // actor may need special handling
    enabled(other.enabled), onStartOver(other.onStartOver),
    x(other.x), y(other.y), body_type(other.body_type),
    collider_type(other.collider_type), width(other.width), height(other.height),
    radius(other.radius), friction(other.friction), bounciness(other.bounciness),
    trigger_type(other.trigger_type), trigger_width(other.trigger_width),
    trigger_height(other.trigger_height), trigger_radius(other.trigger_radius),
    precise(other.precise), gravity_scale(other.gravity_scale),
    density(other.density), angular_friction(other.angular_friction),
    rotation(other.rotation), has_collider(other.has_collider), has_trigger(other.has_trigger),
    m_body(nullptr) { 


    // If there are other pointers or resources that need deep copying, handle them here
}

void RigidBody::InitializeWorld() {

    if (!world) {
        b2Vec2 gravity(0.0f, 9.8f); // Earth's gravity in m/s^2 downwards
        world = new b2World(gravity);
     
        ColliderDetector* myListener = new ColliderDetector();
        world->SetContactListener(myListener);

    }
}

void RigidBody::Step() {
    if (world)
    {
        world->Step(1.0f / 60.0f, 8, 3);
    }
}

void RigidBody::OnStart() {
    // Create the body definition based on stored properties
    b2BodyDef bodyDef;
    bodyDef.position.Set(x, y);
    bodyDef.angle = rotation * b2_pi / 180.0f; // Convert degrees to radians

    // Set body type
    if (body_type == "dynamic") bodyDef.type = b2_dynamicBody;
    else if (body_type == "static") bodyDef.type = b2_staticBody;
    else if (body_type == "kinematic") bodyDef.type = b2_kinematicBody;

    // Other properties
    bodyDef.bullet = precise;
    bodyDef.angularDamping = angular_friction;
    bodyDef.gravityScale = gravity_scale;

    // Create the body in the Box2D world
    m_body = world->CreateBody(&bodyDef);

    // Define a box shape for our body
    b2PolygonShape boxShape;
    boxShape.SetAsBox(0.5f, 0.5f); 


    /* phantom sensor to make bodies move if neither collider nor trigger is present */
    if (!has_collider && !has_trigger)
    {
        b2PolygonShape phantom_shape;
        phantom_shape.SetAsBox(width * 0.5f, height * 0.5f);

        b2FixtureDef phantom_fixture_def;
        phantom_fixture_def.shape = &phantom_shape;
        phantom_fixture_def.density = density;

        // Because it is a sensor (with no callback even), no collisions will ever occur
        phantom_fixture_def.isSensor = true;

        //phantom_fixture_def.userData.pointer = reinterpret_cast<uintptr_t>(actor);

        m_body->CreateFixture(&phantom_fixture_def);

    }
    if (has_collider) {
        if (collider_type == "box") {
            b2PolygonShape boxShape;
            boxShape.SetAsBox(width / 2.0f, height / 2.0f); // Half-width and half-height

            b2FixtureDef fixtureDef;
            fixtureDef.shape = &boxShape;
            fixtureDef.density = density;
            fixtureDef.friction = friction;
            fixtureDef.restitution = bounciness;
            fixtureDef.isSensor = false;
            fixtureDef.userData.pointer = reinterpret_cast<uintptr_t>(actor);
            m_body->CreateFixture(&fixtureDef);
        }
        else if (collider_type == "circle") {
            b2CircleShape circleShape;
            circleShape.m_radius = radius;

            b2FixtureDef fixtureDef;
            fixtureDef.shape = &circleShape;
            fixtureDef.density = density;
            fixtureDef.friction = friction;
            fixtureDef.restitution = bounciness;
            fixtureDef.isSensor = false;
            fixtureDef.userData.pointer = reinterpret_cast<uintptr_t>(actor);
            m_body->CreateFixture(&fixtureDef);
        }
    }
    if (has_trigger) {
        if (trigger_type == "box") {
            b2PolygonShape triggerShape;
            triggerShape.SetAsBox(trigger_width / 2.0f, trigger_height / 2.0f); 

            b2FixtureDef triggerFixtureDef;
            triggerFixtureDef.shape = &triggerShape;
            triggerFixtureDef.isSensor = true; // Make it a sensor
            triggerFixtureDef.density = density;
            triggerFixtureDef.friction = friction;
            triggerFixtureDef.restitution = bounciness;
            triggerFixtureDef.userData.pointer = reinterpret_cast<uintptr_t>(actor); 

            m_body->CreateFixture(&triggerFixtureDef);
        }
        else if (trigger_type == "circle") {
            // Create a circle shape for the trigger volume
            b2CircleShape triggerShape;
            triggerShape.m_radius = trigger_radius;

            b2FixtureDef triggerFixtureDef;
            triggerFixtureDef.shape = &triggerShape;
            triggerFixtureDef.isSensor = true; // Make it a sensor
            triggerFixtureDef.density = density;
            triggerFixtureDef.friction = friction;
            triggerFixtureDef.restitution = bounciness;
            triggerFixtureDef.userData.pointer = reinterpret_cast<uintptr_t>(actor); 

            m_body->CreateFixture(&triggerFixtureDef); 
        }
    }


}

float RigidBody::convert_rad_to_deg(float rad)
{
    return rad * (180.0f / b2_pi);
}

float RigidBody::convert_deg_to_rad(float deg)
{
    return deg * (b2_pi / 180.0f);
}


b2Vec2 RigidBody::toB2Vec2(const Vector2& vector) const {
    return b2Vec2(vector.getX(), vector.getY());
}

Vector2 RigidBody::GetPosition() {
    if (m_body != nullptr) { // Check if the physics body has been created
        b2Vec2 pos = m_body->GetPosition();
        return Vector2{ pos.x, pos.y };
    }
    return Vector2(x, y); // Return stored position if m_body is not available
}


float RigidBody::GetRotation() {
    if (m_body != nullptr) {
        return convert_rad_to_deg(m_body->GetAngle());
    }
    return rotation; // Return stored rotation if m_body is not available
}


void RigidBody::AddForce(const Vector2& force) {
    if (m_body != nullptr) {
        m_body->ApplyForceToCenter(toB2Vec2(force), true);
    }
}

void RigidBody::SetVelocity(const Vector2& velocity) {
    if (m_body != nullptr) {
        m_body->SetLinearVelocity(toB2Vec2(velocity));
    }
}


void RigidBody::SetPosition(const Vector2& position) {
    if (m_body != nullptr) {
        m_body->SetTransform(toB2Vec2(position), m_body->GetAngle());
    }
    else {
        x = position.getX(); // Store the position if m_body is not available
        y = position.getY();
    }
}

void RigidBody::SetRotation(float degreesClockwise) {
    if (m_body != nullptr) {
        m_body->SetTransform(m_body->GetPosition(), convert_deg_to_rad(degreesClockwise));
    }
    else {
        rotation = degreesClockwise; // Store the rotation if m_body is not available
    }
}


void RigidBody::SetAngularVelocity(float degreesClockwise) {
    if (m_body != nullptr) {
        m_body->SetAngularVelocity(convert_deg_to_rad(degreesClockwise));
    }
}

void RigidBody::SetGravityScale(float scale) {
    if (m_body != nullptr) {
        m_body->SetGravityScale(scale);
    }
}

void RigidBody::SetUpDirection(const Vector2& direction) {
    if (m_body != nullptr) {
        Vector2 normalizedDirection = direction;
        normalizedDirection.Normalize();
        m_body->SetTransform(m_body->GetPosition(), glm::atan(normalizedDirection.getX(), -normalizedDirection.getY()));
    }
}

void RigidBody::SetRightDirection(const Vector2& direction) {
    if (m_body != nullptr) {
        Vector2 normalizedDirection = direction;
        normalizedDirection.Normalize();
        m_body->SetTransform(m_body->GetPosition(), (glm::atan(normalizedDirection.getX(), -normalizedDirection.getY()) - (b2_pi / 2.0)));
    }
}

Vector2 RigidBody::GetVelocity() const {
    if (m_body != nullptr) {
        b2Vec2 velocity = m_body->GetLinearVelocity();
        return Vector2(velocity.x, velocity.y);
    }
}

float RigidBody::GetAngularVelocity() const {
    return (m_body->GetAngularVelocity()) * (180.0f / b2_pi);
}

float RigidBody::GetGravityScale() const {
    return m_body->GetGravityScale();
}

Vector2 RigidBody::GetUpDirection() {
    float angle = m_body->GetAngle();
    return Vector2( glm::sin(angle), -glm::cos(angle)).Normalized();
}


Vector2 RigidBody::GetRightDirection() {
    float angle = m_body->GetAngle();
    return Vector2(glm::cos(angle), glm::sin(angle)).Normalized();
}
