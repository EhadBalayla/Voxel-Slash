#include "Prefab.h"
#include <fstream>
#include <iostream>

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

    size_t childsCount = node->m_Children.size();
    file.write(reinterpret_cast<char*>(&childsCount), sizeof(size_t));
    for(auto c : node->m_Children) {
        WritePrefabNode(c, file);
    }
}
void LoadPrefabNode(PrefabNode* node, std::ifstream& file) {
    size_t nameSize;
    file.read(reinterpret_cast<char*>(&nameSize), sizeof(size_t));
    node->m_Name.resize(nameSize);
    file.read(reinterpret_cast<char*>(node->m_Name.data()), nameSize);

    file.read(reinterpret_cast<char*>(&node->pos), sizeof(glm::vec3));
    file.read(reinterpret_cast<char*>(&node->rot), sizeof(glm::vec3));
    file.read(reinterpret_cast<char*>(&node->scale), sizeof(glm::vec3));

    size_t childsCount;
    file.read(reinterpret_cast<char*>(&childsCount), sizeof(size_t));
    for(int i = 0; i < childsCount; i++) {
        PrefabNode* newNode = new PrefabNode;
        newNode->m_Parent = node;
        node->m_Children.push_back(newNode);
        LoadPrefabNode(newNode, file);
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
void Prefab::Load(const char* path) {
    std::ifstream file(path, std::ios::binary);
    if(!file.is_open()) {
        std::cout << "failed to open file for loading prefab" << std::endl;
    }

    LoadPrefabNode(&m_RootNode, file);

    file.close();
}