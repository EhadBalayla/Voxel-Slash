#include "Canvas.h"
#include <fstream>
#include <iostream>

#include "ModInstance.h"

#include <glm/gtc/matrix_transform.hpp>

void RenderCanvasNode(UINode* node, glm::mat4 parentTrans, VkCommandBuffer cmd, VkPipelineLayout layout, int ScrWidth, int ScrHeight) {
    float left = -node->Left * ScrWidth;
    float right = node->Right * ScrWidth;
    float bottom = node->Bottom * ScrHeight;
    float top = -node->Top * ScrHeight;

    glm::mat4 proj = glm::ortho(left, right, bottom, top, -1.0f, 1.0f);

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(node->Position.x, node->Position.y, 0.0f));
    model = glm::rotate(model, glm::radians(node->Rotation), glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::scale(model, glm::vec3(node->Size.x, node->Size.y, 1.0f));

    glm::mat4 overallTrans = parentTrans * model;
    glm::mat4 ProjTrans = proj * overallTrans;
    vkCmdPushConstants(cmd, layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &ProjTrans);
    vkCmdDraw(cmd, 6, 1, 0, 0);

    for(auto& n : node->m_Children) {
        RenderCanvasNode(n, overallTrans, cmd, layout, ScrWidth, ScrHeight);
    }
}



void WriteUINode(UINode* node, std::ofstream& file) {
    size_t nameSize = node->m_Name.size();
    file.write(reinterpret_cast<char*>(&nameSize), sizeof(size_t));
    file.write(node->m_Name.c_str(), nameSize);

    file.write(reinterpret_cast<char*>(&node->Position), sizeof(glm::vec2));
    file.write(reinterpret_cast<char*>(&node->Rotation), sizeof(float));
    file.write(reinterpret_cast<char*>(&node->Size), sizeof(glm::vec2));

    file.write(reinterpret_cast<char*>(&node->Left), sizeof(float));
    file.write(reinterpret_cast<char*>(&node->Right), sizeof(float));
    file.write(reinterpret_cast<char*>(&node->Bottom), sizeof(float));
    file.write(reinterpret_cast<char*>(&node->Top), sizeof(float));

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

    file.read(reinterpret_cast<char*>(&node->Left), sizeof(float));
    file.read(reinterpret_cast<char*>(&node->Right), sizeof(float));
    file.read(reinterpret_cast<char*>(&node->Bottom), sizeof(float));
    file.read(reinterpret_cast<char*>(&node->Top), sizeof(float));

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


void Canvas::Render(VkCommandBuffer cmd, VkPipelineLayout layout, int ScrWidth, int ScrHeight) {
    for(auto& n : nodes) {
        RenderCanvasNode(n, glm::mat4(1.0f), cmd, layout, ScrWidth, ScrHeight);
    }
}