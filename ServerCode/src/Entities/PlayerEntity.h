#pragma once
#include <glm/glm.hpp>

class Entity;

struct PlayerData {
    int64_t ChunkCoordX = 0;
    int64_t ChunkCoordY = 0;
    int64_t ChunkCoordZ = 0;

    bool IsOnGround = false;
    float acceleration = 0.5f;
	float maxMovementSpeed = 5.2f;
	float friction = 0.8f;
	float gravity = 15.0f;
    glm::vec3 velocity = glm::vec3(0.0f);

    bool IsForward = false; //for when moving forward
    bool IsBackward = false; //for when moving backward
    bool IsLeft = false; //for when moving left
    bool IsRight = false; //for when moving right
	bool IsJump = false; //for when the player is jumping
};

void MoveAndCollide(Entity* self, float DeltaTime);
void UpdateChunksAroundPlayer(Entity* player);


//actual entity functions
void PlayerTick(Entity* self);
void* PlayerDataCreation();