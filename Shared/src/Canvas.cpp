#include "Canvas.h"
#include <fstream>
#include <iostream>

#include "ModInstance.h"

void WriteUINode(UINode* node, std::ofstream& file) {
    size_t nameSize = node->m_Name.size();
    file.write(reinterpret_cast<char*>(&nameSize), sizeof(size_t));
    file.write(node->m_Name.c_str(), nameSize);

    file.write(reinterpret_cast<char*>(&node->Position), sizeof(glm::vec2));
    file.write(reinterpret_cast<char*>(&node->Rotation), sizeof(float));
    file.write(reinterpret_cast<char*>(&node->Size), sizeof(glm::vec2));

    size_t childsCount = node->m_Children.size();
    file.write(reinterpret_cast<char*>(&childsCount), sizeof(size_t));
    for(auto c : node->m_Children) {
        WriteUINode(c, file);
    }
}
void LoadUINode(UINode* node, std::ifstream& file, ModInstance* mod) {
    size_t nameSize;
    file.read(reinterpret_cast<char*>(&nameSize), sizeof(size_t));
    node->m_Name.resize(nameSize);
    file.read(reinterpret_cast<char*>(node->m_Name.data()), nameSize);

    file.read(reinterpret_cast<char*>(&node->Position), sizeof(glm::vec2));
    file.read(reinterpret_cast<char*>(&node->Rotation), sizeof(float));
    file.read(reinterpret_cast<char*>(&node->Size), sizeof(glm::vec2));

    size_t childsCount;
    file.read(reinterpret_cast<char*>(&childsCount), sizeof(size_t));
    for(size_t i = 0; i < childsCount; i++) {
        UINode* newNode = new UINode;
        newNode->m_Parent = node;
        node->m_Children.push_back(newNode);
        LoadUINode(newNode, file, mod);
    }
}

void Canvas::Save(const char* path) {
    std::ofstream file(path, std::ios::binary);
    if(!file.is_open()) {
        std::cout << "failed to create file for saving canvas" << std::endl;
    }

    size_t topLevelNodesCount = nodes.size();
    file.write(reinterpret_cast<char*>(&topLevelNodesCount), sizeof(size_t));
    for(auto c : nodes) {
        WriteUINode(c, file);
    }

    file.close();
}
void Canvas::Load(const char* path, ModInstance* mod) {
    std::ifstream file(path, std::ios::binary);
    if(!file.is_open()) {
        std::cout << "failed to open file for loading canvas" << std::endl;
    }

    size_t topLevelNodesCount = nodes.size();
    file.read(reinterpret_cast<char*>(&topLevelNodesCount), sizeof(size_t));
    for(size_t i = 0; i < topLevelNodesCount; i++) {
        UINode* c = new UINode;
        nodes.push_back(c);
        LoadUINode(c, file, mod);
    }

    file.close();
}