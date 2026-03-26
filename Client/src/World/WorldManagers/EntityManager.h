#pragma once
#include <vector>

class Entity;
class Player;

class EntityManager {
public:
    void Render();
private:
    std::vector<Entity*> entities;
    Player* player = nullptr;
};