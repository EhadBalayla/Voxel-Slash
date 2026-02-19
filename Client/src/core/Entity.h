#pragma once
#include "AABB.h"

class Entity {
public:
    glm::vec3 Position = glm::vec3(0.0f);
    float Rotation = 0.0f;
    
    AABB aabb;

    virtual void Update(float DeltaTime) = 0;

    glm::vec3 GetForwardVector();
    glm::vec3 GetRightVector();

protected:
    void MoveAndCollide(float DeltaTime);

    bool IsOnGround = false;
    float acceleration = 0.5f;
	float maxMovementSpeed = 5.2f;
	float friction = 0.8f;
	float gravity = 15.0f;
    glm::vec3 velocity = glm::vec3(0.0f);
};