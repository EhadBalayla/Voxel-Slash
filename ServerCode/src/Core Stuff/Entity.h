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

    /*glm::vec3 GetForwardVector();
    glm::vec3 GetRightVector();
    
    void MoveAndCollide(float DeltaTime);
private:

    bool IsOnGround = false;
    float acceleration = 0.5f;
	float maxMovementSpeed = 5.2f;
	float friction = 0.8f;
	float gravity = 15.0f;
    glm::vec3 velocity = glm::vec3(0.0f);*/
};