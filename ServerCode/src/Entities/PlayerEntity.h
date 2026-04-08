#pragma once
#include "../Core Stuff/Entity.h"
#include "../Server.h"
#include "../Core Stuff/Chunk.h"

struct PlayerData {
    int ChunkCoordX = 0;
    int ChunkCoordY = 0;
    int ChunkCoordZ = 0;
};

void UpdateChunksAroundPlayer(Entity* player) {
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
}


//actual entity functions
void PlayerTick(Entity* self) {
    UpdateChunksAroundPlayer(self);
    self->MoveAndCollide(1.0 / 20.0f);
}
void* PlayerDataCreation() {
    PlayerData* pData = (PlayerData*)malloc(sizeof(PlayerData)); 
    return pData;
}