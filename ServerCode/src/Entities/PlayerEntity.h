#pragma once
#include <glm/glm.hpp>

class Entity;

struct PlayerData {
    int ChunkCoordX = 0;
    int ChunkCoordY = 0;
    int ChunkCoordZ = 0;

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

/*void UpdateChunksAroundPlayer(Entity* player) {
    if(!GServer->m_ChunkManager.IsUpdatingChunks) {
        int CurrentCoordX = static_cast<int>(std::floor(player->Position.x / Chunk_Length));
        int CurrentCoordY = static_cast<int>(std::floor(player->Position.y / Chunk_Length));
        int CurrentCoordZ = static_cast<int>(std::floor(player->Position.z / Chunk_Length));

        PlayerData* Data = reinterpret_cast<PlayerData*>(player->ExtraData);

        if(CurrentCoordX != Data->ChunkCoordX || CurrentCoordY != Data->ChunkCoordY || CurrentCoordZ != Data->ChunkCoordZ) {
            Data->ChunkCoordX = CurrentCoordX;
            Data->ChunkCoordY = CurrentCoordY;
            Data->ChunkCoordZ = CurrentCoordZ;

            GServer->m_ChunkManager.UpdateChunks();
        }
    }
}*/


//actual entity functions
void PlayerTick(Entity* self);
void* PlayerDataCreation();