#pragma once
#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

class ModInstance;
class TransformAsset;

struct PrefabNode {
    PrefabNode* m_Parent = nullptr;
    std::vector<PrefabNode*> m_Children;

    std::string m_Name = "Node";

    //relative to parent unless its root in which case its the pivot
    glm::vec3 pos = glm::vec3(0.0f);
    glm::vec3 rot = glm::vec3(0.0f); //yaw/pitch/roll
    glm::vec3 scale = glm::vec3(1.0f);

    //the asset that is loaded within the prefab
    TransformAsset* m_Asset = nullptr;
};

void AddNewPrefabNode(PrefabNode* parentNode, std::string newName);
void RemovePrefabNode(PrefabNode* NodeToRemove);

class Prefab {
public:
    void Save(const char* path);
    void Load(const char* path, ModInstance* mod);

    void Render(VkCommandBuffer cmd, VkPipelineLayout ppLayout, glm::mat4 Start);

    PrefabNode m_RootNode;
};