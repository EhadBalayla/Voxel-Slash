#pragma once
#include <unordered_map>
#include <string>
#include "Core Stuff/Block.h"
#include "Core Stuff/Entity.h"

class Registery {
public:
    Registery();

    std::unordered_map<BlockType, BlockData>& GetBlockRegistery();
    std::unordered_map<std::string, EntityData>& GetEntityRegistery();
private:
    std::unordered_map<BlockType, BlockData> BlockRegistery;
    std::unordered_map<std::string, EntityData> EntityRegistery;

    void RegisterBlocks();
    void RegisterEntities();
};