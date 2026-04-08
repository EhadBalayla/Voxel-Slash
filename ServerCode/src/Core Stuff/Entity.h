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
    glm::dvec3 Position = glm::dvec3(0.0f); //remember to later on change it into doubles to fix distance precision loss
    float Rotation = 0.0f;
    EntityData Data;
    void* ExtraData = nullptr;

    glm::vec3 GetForwardVector();
    glm::vec3 GetRightVector();
    
    void MoveAndCollide(float DeltaTime);
protected:

    bool IsOnGround = false;
    float acceleration = 0.5f;
	float maxMovementSpeed = 5.2f;
	float friction = 0.8f;
	float gravity = 15.0f;
    glm::vec3 velocity = glm::vec3(0.0f);
};