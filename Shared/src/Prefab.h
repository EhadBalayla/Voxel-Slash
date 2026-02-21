#pragma once
#include <vector>
#include <string>
#include <glm/glm.hpp>

struct PrefabNode {
    PrefabNode* m_Parent = nullptr;
    std::vector<PrefabNode*> m_Children;

    std::string m_Name = "Node";

    //relative to parent unless its root in which case its the pivot
    glm::vec3 pos = glm::vec3(0.0f);
    glm::vec3 rot = glm::vec3(0.0f); //yaw/pitch/roll
    glm::vec3 scale = glm::vec3(1.0f);
};
void AddNewPrefabNode(PrefabNode* parentNode, std::string newName);
void RemovePrefabNode(PrefabNode* NodeToRemove);

class Prefab {
public:
    void Save();

    PrefabNode m_RootNode;
};