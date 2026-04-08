#include "EntityManager.h"
#include "Server.h"
#include <iostream>

EntityManager::EntityManager() {
    std::cout << "Started the entity manager" << std::endl;
}
EntityManager::~EntityManager() {
    std::cout << "Ended the entity manager" << std::endl;
}

void EntityManager::TickEntities() {
    //std::lock_guard<std::mutex> lock(enttArrMTX);

    /*for(auto& e : entities) {
        e.Data.Tick(&e);
    }*/
}

uint64_t EntityManager::SpawnEntity(std::string EntityID) {
    //assuming the type always exists, cause i am too lazy for the moment to add simple check
    auto& registery = GServer->m_Registery.GetEntityRegistery();
    EntityData data = registery[EntityID];

    Entity entity;
    entity.Data = data;

    entity.ExtraData = data.CreateExtraData();

    {
        std::lock_guard<std::mutex> lock(enttArrMTX);
        entities.push_back(entity);
    }
    
    uint64_t currentID = NextEntityID;
    NextEntityID++;

    idToIdx[currentID] = entities.size() - 1;
    std::cout << "Spawned entity of type " << EntityID << std::endl;
    return currentID;
}
void EntityManager::DeleteEntity(uint64_t id) {
    std::lock_guard<std::mutex> lock(enttArrMTX);

    auto it = idToIdx.find(id);
    if(it == idToIdx.end()) return;

    size_t idx = it->second;

    free(entities[idx].ExtraData);

    // swap with last element to maintain vector integrity
    if(idx != entities.size() - 1) {
        std::swap(entities[idx], entities.back());
        //idToIdx[entities[idx].ID] = idx; // update map
    }

    entities.pop_back();
    idToIdx.erase(it);
}