#pragma once
#include <vector>
#include <mutex>
#include <unordered_map>
#include "Core Stuff/Entity.h"

class EntityManager {
public:
    EntityManager();
    ~EntityManager();

    void TickEntities();

    uint64_t SpawnEntity(std::string EntityType); //spawns an entity based on type and returns the ID for the spawned entity
    void DeleteEntity(uint64_t id);
private:
    uint64_t NextEntityID = 0; //a counter for entity ids

    std::mutex enttArrMTX;
    std::vector<Entity> entities;
    std::unordered_map<uint64_t, size_t> idToIdx;
};