#include "Prefab.h"
#include <fstream>
#include <iostream>

#include "AssetFormats/TransformAsset.h"
#include "ModInstance.h"

#include <glm/gtc/matrix_transform.hpp>

void RenderNode(PrefabNode* node, glm::mat4 parentTransform, VkCommandBuffer cmd, VkPipelineLayout ppLayout) {
	glm::mat4 pos = glm::translate(glm::mat4(1.0f), node->pos);

	float yaw   = glm::radians(node->rot.y);
    float pitch = glm::radians(node->rot.x);
    float roll  = glm::radians(node->rot.z);
    glm::mat4 rotX = glm::rotate(glm::mat4(1.0f), pitch, glm::vec3(1.0f, 0.0f, 0.0f));
    glm::mat4 rotY = glm::rotate(glm::mat4(1.0f), yaw,   glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 rotZ = glm::rotate(glm::mat4(1.0f), roll,  glm::vec3(0.0f, 0.0f, 1.0f));
    glm::mat4 rot = rotY * rotX * rotZ;

	glm::mat4 scale = glm::scale(glm::mat4(1.0f), node->scale);

	glm::mat4 overall = pos * rot * scale;
	overall = parentTransform * overall;

	if(node->m_Asset) {
        node->m_Asset->Render(cmd, ppLayout, overall);
    }

	for(auto c : node->m_Children) {
		RenderNode(c, overall, cmd, ppLayout);
	}
}

void AddNewPrefabNode(PrefabNode* parentNode, std::string newName) {
    PrefabNode* newNode = new PrefabNode;
    newNode->m_Name = newName;
    newNode->m_Parent = parentNode;
    parentNode->m_Children.push_back(newNode);
}
void RemovePrefabNode(PrefabNode* NodeToRemove) {
    if(!NodeToRemove->m_Parent) return;

    auto& childrenArray = NodeToRemove->m_Parent->m_Children;
    for(auto it = childrenArray.begin(); it != childrenArray.end();) {
        PrefabNode* p = *it;
        if(p == NodeToRemove) {
            delete p;
            it = childrenArray.erase(it);
        } else {
            it++;
        }
    }
}


void WritePrefabNode(PrefabNode* node, std::ofstream& file) {
    size_t nameSize = node->m_Name.size();
    file.write(reinterpret_cast<char*>(&nameSize), sizeof(size_t));
    file.write(node->m_Name.c_str(), nameSize);

    file.write(reinterpret_cast<char*>(&node->pos), sizeof(glm::vec3));
    file.write(reinterpret_cast<char*>(&node->rot), sizeof(glm::vec3));
    file.write(reinterpret_cast<char*>(&node->scale), sizeof(glm::vec3));

    size_t aNameSize = 0;
    if(!node->m_Asset) {
        file.write(reinterpret_cast<char*>(&aNameSize), sizeof(size_t));
    } else {
        size_t aNameSize = node->m_Asset->AssetName.size();
        file.write(reinterpret_cast<char*>(&aNameSize), sizeof(size_t));
        file.write(node->m_Asset->AssetName.c_str(), aNameSize);
    }

    size_t childsCount = node->m_Children.size();
    file.write(reinterpret_cast<char*>(&childsCount), sizeof(size_t));
    for(auto c : node->m_Children) {
        WritePrefabNode(c, file);
    }
}
void LoadPrefabNode(PrefabNode* node, std::ifstream& file, ModInstance* mod) {
    size_t nameSize;
    file.read(reinterpret_cast<char*>(&nameSize), sizeof(size_t));
    node->m_Name.resize(nameSize);
    file.read(reinterpret_cast<char*>(node->m_Name.data()), nameSize);

    file.read(reinterpret_cast<char*>(&node->pos), sizeof(glm::vec3));
    file.read(reinterpret_cast<char*>(&node->rot), sizeof(glm::vec3));
    file.read(reinterpret_cast<char*>(&node->scale), sizeof(glm::vec3));

    size_t aNameSize = 0;
    file.read(reinterpret_cast<char*>(&aNameSize), sizeof(size_t));
    if(aNameSize != 0) {
        std::string name;
        name.resize(aNameSize);
        file.read(name.data(), aNameSize);
        node->m_Asset = static_cast<TransformAsset*>(mod->GetAllAssets()[name]);
    }

    size_t childsCount;
    file.read(reinterpret_cast<char*>(&childsCount), sizeof(size_t));
    for(size_t i = 0; i < childsCount; i++) {
        PrefabNode* newNode = new PrefabNode;
        newNode->m_Parent = node;
        node->m_Children.push_back(newNode);
        LoadPrefabNode(newNode, file, mod);
    }
}

void Prefab::Save(const char* path) {
    std::ofstream file(path, std::ios::binary);
    if(!file.is_open()) {
        std::cout << "failed to create file for saving prefab" << std::endl;
    }

    WritePrefabNode(&m_RootNode, file);

    file.close();
}
void Prefab::Load(const char* path, ModInstance* mod) {
    std::ifstream file(path, std::ios::binary);
    if(!file.is_open()) {
        std::cout << "failed to open file for loading prefab" << std::endl;
    }

    LoadPrefabNode(&m_RootNode, file, mod);

    file.close();
}

void Prefab::Render(VkCommandBuffer cmd, VkPipelineLayout ppLayout, glm::mat4 Start) {
    RenderNode(&m_RootNode, Start, cmd, ppLayout);
}