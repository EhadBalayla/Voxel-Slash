#pragma once
#include "AABB.h"

class Entity;
struct EntityData {
    AABB aabb;
    void(*Tick)(Entity* self);
    void* (*CreateExtraData)();
};

class EntityManager;
class Entity {
public:
    uint64_t ID = 0;
    glm::dvec3 Position = glm::dvec3(0.0f);
    float Rotation = 0.0f;
    EntityData Data;
    void* ExtraData = nullptr;

    glm::vec3 GetForwardVector();
    glm::vec3 GetRightVector();
};