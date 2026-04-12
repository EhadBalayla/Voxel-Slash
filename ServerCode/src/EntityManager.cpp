#include "EntityManager.h"
#include "Server.h"
#include <iostream>

EntityManager::EntityManager() {
    std::cout << "Started the entity manager\n";
}
EntityManager::~EntityManager() {
    std::cout << "Ended the entity manager\n";
}

void EntityManager::TickEntities() {
    std::lock_guard<std::mutex> lock(enttArrMTX);

    for(auto& e : entities) {
        e.Data.Tick(&e);
    }
}

uint64_t EntityManager::SpawnEntity(std::string EntityID, glm::dvec3 Position, float Rotation) {
    //assuming the type always exists, cause i am too lazy for the moment to add simple check
    auto& registery = GServer->m_Registery.GetEntityRegistery();
    EntityData data = registery[EntityID];

    Entity entity;
    entity.ID = NextEntityID;
    entity.Position = Position;
    entity.Rotation = Rotation;
    entity.Data = data;

    entity.ExtraData = data.CreateExtraData();

    {
        std::lock_guard<std::mutex> lock(enttArrMTX);
        entities.push_back(entity);
    }
    
    uint64_t currentID = NextEntityID;
    NextEntityID++;

    idToIdx[currentID] = entities.size() - 1;
    std::cout << "Spawned entity of type " << EntityID << "\n";
    return currentID;
}
void EntityManager::DeleteEntity(uint64_t id) {
    std::lock_guard<std::mutex> lock(enttArrMTX);

    auto it = idToIdx.find(id);
    if(it == idToIdx.end()) return;

    size_t idx = it->second;

    free(entities[idx].ExtraData);

    if(idx != entities.size() - 1) {
        std::swap(entities[idx], entities.back());
        idToIdx[entities[idx].ID] = idx;
    }

    entities.pop_back();
    idToIdx.erase(it);
}
Entity EntityManager::GetEntity(uint64_t id) {
    std::lock_guard<std::mutex> lock(enttArrMTX);
    return entities[idToIdx[id]];
}
std::vector<Entity>& EntityManager::GetAllEntities() {
    return entities;
}
std::unordered_map<uint64_t, size_t>& EntityManager::GetIDToIDX() {
    return idToIdx;
}