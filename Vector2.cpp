// Vector2.cpp
#include "headers/Vector2.h"

Vector2::Vector2(float x, float y) : vec(x, y) {}

float Vector2::getX() const { return vec.x; }
void Vector2::setX(float x) { vec.x = x; }
float Vector2::getY() const { return vec.y; }
void Vector2::setY(float y) { vec.y = y; }

void Vector2::Normalize() { vec.Normalize(); }
float Vector2::Length() const { return vec.Length(); }

float Vector2::Distance(const Vector2& a, const Vector2& b) {
    return b2Distance(a.vec, b.vec);
}

float Vector2::Dot(const Vector2& a, const Vector2& b) {
    return b2Dot(a.vec, b.vec);
}

Vector2 Vector2::operator+(const Vector2& other) const {
    return Vector2(vec.x + other.vec.x, vec.y + other.vec.y);
}

Vector2 Vector2::operator-(const Vector2& other) const {
    return Vector2(vec.x - other.vec.x, vec.y - other.vec.y);
}

Vector2 Vector2::operator*(float scalar) const {
    return Vector2(vec.x * scalar, vec.y * scalar);
}


Vector2 Vector2::Normalized() {
    float length = Length();
    if (length > 0.0f) {
        return Vector2(vec.x / length, vec.y / length);
    }
    return *this;
}

void Vector2::Vector2ToLua(lua_State* L) {
    luabridge::getGlobalNamespace(L)
        .beginClass<Vector2>("Vector2")
        .addConstructor<void(*)(float, float)>()
        .addProperty("x", &Vector2::getX, &Vector2::setX)
        .addProperty("y", &Vector2::getY, &Vector2::setY)
        .addFunction("Normalize", &Vector2::Normalize)
        .addFunction("Length", &Vector2::Length)
        .addStaticFunction("Distance", &Vector2::Distance)
        .addStaticFunction("Dot", static_cast<float(*)(const Vector2&, const Vector2&)>(&Dot))
        .addFunction("__add", &Vector2::operator+)
        .addFunction("__sub", &Vector2::operator-)
        .addFunction("__mul", &Vector2::operator*)
        .endClass();
}
