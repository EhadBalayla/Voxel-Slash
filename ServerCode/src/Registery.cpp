#include "Registery.h"
#include <iostream>

Registery::Registery() {
    RegisterBlocks();
    RegisterEntities();
}

std::unordered_map<BlockType, BlockData>& Registery::GetBlockRegistery() {
    return BlockRegistery;
}
std::unordered_map<std::string, EntityData>& Registery::GetEntityRegistery() {
    return EntityRegistery;
}

void Registery::RegisterBlocks() {
    BlockRegistery[BlockType::Air] = {{0, 0, 0, 0, 0, 0}};
    BlockRegistery[BlockType::Stone] = {{3, 3, 3, 3, 3, 3}};
    BlockRegistery[BlockType::Grass] = {{0, 2, 1, 1, 1, 1}};
    BlockRegistery[BlockType::Dirt] = {{2, 2, 2, 2, 2, 2}};
    BlockRegistery[BlockType::Water] = {{255, 255, 255, 255, 255, 255}};
}

#include "Entities/PlayerEntity.h"
void Registery::RegisterEntities() {
    EntityRegistery["Player"] = {{{-0.3f, 0.0f, -0.3f}, {0.3f, 1.8f, 0.3f}}, PlayerTick, PlayerDataCreation};
}