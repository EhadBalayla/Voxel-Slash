#include "Canvas.h"
#include <fstream>
#include <iostream>

#include "ModInstance.h"
#include "AssetFormats/TextureAsset.h"
#include "AssetFormats/FontAsset.h"
#include "Context.h"
#include "Window.h"
#include "Renderer.h"

#include <glm/gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>

void RenderCanvasNode(UINode* node, glm::mat4 parentTrans, VkCommandBuffer cmd, int ScrWidth, int ScrHeight) {
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

    if(node->m_Element) {
        node->m_Element->Render(cmd, ProjTrans);
    }

    for(auto& n : node->m_Children) {
        RenderCanvasNode(n, overallTrans, cmd, ScrWidth, ScrHeight);
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

    uint32_t type = 0;
    if(node->m_Element) {
        type = static_cast<uint32_t>(node->m_Element->GetType());
        file.write(reinterpret_cast<char*>(&type), sizeof(uint32_t));
        node->m_Element->Save(file);
    } else {
        file.write(reinterpret_cast<char*>(&type), sizeof(uint32_t));
    }

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

    uint32_t type = 0;
    file.read(reinterpret_cast<char*>(&type), sizeof(uint32_t));
    if(type > 0) {
        switch (static_cast<UIType>(type)) {
            case UIType::Image:
                node->m_Element = new UIImage;
            break;
            case UIType::Button:
                node->m_Element = new UIButton;
            break;
            case UIType::Text:
                node->m_Element = new UIText;
            break;
        }
        node->m_Element->Load(file, mod);
        node->m_Element->m_Node = node;
    }

    size_t childsCount;
    file.read(reinterpret_cast<char*>(&childsCount), sizeof(size_t));
    for(size_t i = 0; i < childsCount; i++) {
        UINode* newNode = new UINode;
        newNode->m_Parent = node;
        node->m_Children.push_back(newNode);
        LoadUINode(newNode, file, mod);
    }
}
void TickUINode(UINode* node) {
    if(node->m_Element && node->m_Element->GetType() == UIType::Button) {
        node->m_Element->Tick();
    }
    for(auto& n : node->m_Children) {
        TickUINode(n);
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

void Canvas::Tick() {
    for(auto& n : nodes) {
        TickUINode(n);
    }
}


void Canvas::Render(VkCommandBuffer cmd, int ScrWidth, int ScrHeight) {
    for(auto& n : nodes) {
        RenderCanvasNode(n, glm::mat4(1.0f), cmd, ScrWidth, ScrHeight);
    }
}


UIImage::UIImage() {
    VkDescriptorPoolSize poolSize{};
    poolSize.descriptorCount = GContext->MAX_FRAMES_IN_FLIGHT;
    poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.maxSets = GContext->MAX_FRAMES_IN_FLIGHT;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    
    if (vkCreateDescriptorPool(GContext->GetDevice(), &poolInfo, nullptr, &pool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool for descriptor sets of a UIImage element");
    }
    
    std::vector<VkDescriptorSetLayout> layouts(GContext->MAX_FRAMES_IN_FLIGHT, GContext->GetSingleTexLayout());
    VkDescriptorSetAllocateInfo setsInfo{};
    setsInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    setsInfo.descriptorPool = pool;
    setsInfo.descriptorSetCount = GContext->MAX_FRAMES_IN_FLIGHT;
    setsInfo.pSetLayouts = layouts.data();
    sets.resize(GContext->MAX_FRAMES_IN_FLIGHT);
    if (vkAllocateDescriptorSets(GContext->GetDevice(), &setsInfo, sets.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor setsof a UIImage element");
    }
}
void UIImage::Render(VkCommandBuffer cmd, glm::mat4 mtx) {
    if(m_Asset) {
        if (texturesPerSet[GContext->currentFrame] != &m_Asset->GetTexture()) {
            VkDescriptorImageInfo imgInfo{};
            imgInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imgInfo.imageView = m_Asset->GetTexture().GetImageView();
            imgInfo.sampler = GRenderer->GetSampler();

            VkWriteDescriptorSet write{};
            write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            write.descriptorCount = 1;
            write.pImageInfo = &imgInfo;
            write.dstArrayElement = 0;
            write.dstSet = sets[GContext->currentFrame];
            write.dstBinding = 0;

            vkUpdateDescriptorSets(GContext->GetDevice(), 1, &write, 0, nullptr);

            texturesPerSet[GContext->currentFrame] = &m_Asset->GetTexture();
        }

        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, *GContext->SingleImagePipeline);
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, GContext->GetSingleTexPPLayout(), 0, 1, &sets[GContext->currentFrame], 0, nullptr);
        vkCmdPushConstants(cmd, GContext->GetSingleTexPPLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &mtx);
        vkCmdDraw(cmd, 6, 1, 0, 0);
    }
}
UIType UIImage::GetType() const {
    return UIType::Image;
}
void UIImage::Save(std::ofstream& file) {
    size_t nameSize = 0;
    if(m_Asset) {
        nameSize = m_Asset->AssetName.size();
        file.write(reinterpret_cast<char*>(&nameSize), sizeof(size_t));
        file.write(reinterpret_cast<char*>(m_Asset->AssetName.data()), nameSize);
    }
    else file.write(reinterpret_cast<char*>(&nameSize), sizeof(size_t));
}
void UIImage::Load(std::ifstream& file, ModInstance* mod) {
    size_t nameSize = 0;
    file.read(reinterpret_cast<char*>(&nameSize), sizeof(size_t));
    if(nameSize > 0) {
        std::string assetName;
        assetName.resize(nameSize);
        file.read(reinterpret_cast<char*>(assetName.data()), nameSize);
        m_Asset = static_cast<TextureAsset*>(mod->GetAllAssets()[assetName]);
    }
}


UIButton::UIButton() {
    VkDescriptorPoolSize poolSize{};
    poolSize.descriptorCount = GContext->MAX_FRAMES_IN_FLIGHT;
    poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.maxSets = GContext->MAX_FRAMES_IN_FLIGHT;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    
    if (vkCreateDescriptorPool(GContext->GetDevice(), &poolInfo, nullptr, &pool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool for descriptor sets of a UIImage element");
    }
    
    std::vector<VkDescriptorSetLayout> layouts(GContext->MAX_FRAMES_IN_FLIGHT, GContext->GetSingleTexLayout());
    VkDescriptorSetAllocateInfo setsInfo{};
    setsInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    setsInfo.descriptorPool = pool;
    setsInfo.descriptorSetCount = GContext->MAX_FRAMES_IN_FLIGHT;
    setsInfo.pSetLayouts = layouts.data();
    sets.resize(GContext->MAX_FRAMES_IN_FLIGHT);
    if (vkAllocateDescriptorSets(GContext->GetDevice(), &setsInfo, sets.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor setsof a UIImage element");
    }
}
void UIButton::Render(VkCommandBuffer cmd, glm::mat4 mtx) {
    if(m_Asset) {
        if (texturesPerSet[GContext->currentFrame] != &m_Asset->GetTexture()) {
            VkDescriptorImageInfo imgInfo{};
            imgInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imgInfo.imageView = m_Asset->GetTexture().GetImageView();
            imgInfo.sampler = GRenderer->GetSampler();

            VkWriteDescriptorSet write{};
            write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            write.descriptorCount = 1;
            write.pImageInfo = &imgInfo;
            write.dstArrayElement = 0;
            write.dstSet = sets[GContext->currentFrame];
            write.dstBinding = 0;

            vkUpdateDescriptorSets(GContext->GetDevice(), 1, &write, 0, nullptr);

            texturesPerSet[GContext->currentFrame] = &m_Asset->GetTexture();
        }
    } else {
        //if(texturesPerSet[GContext->currentFrame] != nullptr) {
            VkDescriptorImageInfo imgInfo{};
            imgInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imgInfo.imageView = GContext->GetDummyTexture().GetImageView();
            imgInfo.sampler = GRenderer->GetSampler();

            VkWriteDescriptorSet write{};
            write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            write.descriptorCount = 1;
            write.pImageInfo = &imgInfo;
            write.dstArrayElement = 0;
            write.dstSet = sets[GContext->currentFrame];
            write.dstBinding = 0;

            vkUpdateDescriptorSets(GContext->GetDevice(), 1, &write, 0, nullptr);

            texturesPerSet[GContext->currentFrame] = nullptr;
        //}
    }
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, *GContext->SingleImagePipeline);
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, GContext->GetSingleTexPPLayout(), 0, 1, &sets[GContext->currentFrame], 0, nullptr);
    vkCmdPushConstants(cmd, GContext->GetSingleTexPPLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &mtx);
    vkCmdDraw(cmd, 6, 1, 0, 0);
}
void UIButton::Tick() {
    double xpos, ypos;
    int Width = GWindow->GetWindowWidth(), Height = GWindow->GetWindowHeight();
    glfwGetCursorPos(GWindow->GetGLFWwindow(), &xpos, &ypos);
    
    float CenterX = m_Node->Position.x + (m_Node->Left * Width);
    float CenterY = m_Node->Position.y + (m_Node->Top * Height);

    if(xpos >= CenterX - m_Node->Size.x / 2.0f && xpos <= CenterX + m_Node->Size.x / 2.0f &&
       ypos >= CenterY - m_Node->Size.y / 2.0f && ypos <= CenterY + m_Node->Size.y / 2.0f) {
        if(!IsHovering) {
            IsHovering = true;
            if(OnHovered) OnHovered();
        }
    }
    else {
        if(IsHovering) {
            IsHovering = false;
        }
    }

    if(IsHovering) {
        if(!IsClicking && glfwGetMouseButton(GWindow->GetGLFWwindow(), GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
            IsClicking = true;
            if(OnPress) OnPress();
        }
    }
    if((IsClicking && glfwGetMouseButton(GWindow->GetGLFWwindow(), GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE) || !IsHovering) {
        IsClicking = false;
    }
}
UIType UIButton::GetType() const {
    return UIType::Button;
}
void UIButton::Save(std::ofstream& file) {
    size_t nameSize = 0;
    if(m_Asset) {
        nameSize = m_Asset->AssetName.size();
        file.write(reinterpret_cast<char*>(&nameSize), sizeof(size_t));
        file.write(reinterpret_cast<char*>(m_Asset->AssetName.data()), nameSize);
    }
    else file.write(reinterpret_cast<char*>(&nameSize), sizeof(size_t));
}
void UIButton::Load(std::ifstream& file, ModInstance* mod) {
    size_t nameSize = 0;
    file.read(reinterpret_cast<char*>(&nameSize), sizeof(size_t));
    if(nameSize > 0) {
        std::string assetName;
        assetName.resize(nameSize);
        file.read(reinterpret_cast<char*>(assetName.data()), nameSize);
        m_Asset = static_cast<TextureAsset*>(mod->GetAllAssets()[assetName]);
    }
}



UIText::UIText() {
    VkDescriptorPoolSize poolSize{};
    poolSize.descriptorCount = GContext->MAX_FRAMES_IN_FLIGHT;
    poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.maxSets = GContext->MAX_FRAMES_IN_FLIGHT;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    
    if (vkCreateDescriptorPool(GContext->GetDevice(), &poolInfo, nullptr, &pool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool for descriptor sets of a UIImage element");
    }
    
    std::vector<VkDescriptorSetLayout> layouts(GContext->MAX_FRAMES_IN_FLIGHT, GContext->GetSingleTexLayout());
    VkDescriptorSetAllocateInfo setsInfo{};
    setsInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    setsInfo.descriptorPool = pool;
    setsInfo.descriptorSetCount = GContext->MAX_FRAMES_IN_FLIGHT;
    setsInfo.pSetLayouts = layouts.data();
    sets.resize(GContext->MAX_FRAMES_IN_FLIGHT);
    if (vkAllocateDescriptorSets(GContext->GetDevice(), &setsInfo, sets.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor setsof a UIImage element");
    }
}
void UIText::Render(VkCommandBuffer cmd, glm::mat4 mtx) {
    if(!m_Asset) return;

    if(texturesPerSet[GContext->currentFrame] != &m_Asset->GetFontAtlas()) {
        VkDescriptorImageInfo imgInfo{};
        imgInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imgInfo.imageView = m_Asset->GetFontAtlas().GetImageView();
        imgInfo.sampler = GRenderer->GetSampler();

        VkWriteDescriptorSet write{};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        write.descriptorCount = 1;
        write.pImageInfo = &imgInfo;
        write.dstArrayElement = 0;
        write.dstSet = sets[GContext->currentFrame];
        write.dstBinding = 0;

        vkUpdateDescriptorSets(GContext->GetDevice(), 1, &write, 0, nullptr);

        texturesPerSet[GContext->currentFrame] = &m_Asset->GetFontAtlas();
    }
    
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, *GContext->TextPipeline);
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, GContext->GetSingleTexPPLayout(), 0, 1, &sets[GContext->currentFrame], 0, nullptr);

    float cursorX = 0.0f;
    for(auto& c : text) {
        CharInfo info = m_Asset->GetCharacter(static_cast<int>(c));

		glm::vec3 advPos = glm::vec3(cursorX + info.Bearing.x, -info.Bearing.y, 0.0f);
		glm::vec3 relScale = glm::vec3(info.Size.x, info.Size.y, 0.0f);

		glm::mat4 offsetPos = glm::scale(glm::translate(glm::mat4(1.0f), advPos), relScale);
        glm::mat4 overall = mtx * offsetPos;

        vkCmdPushConstants(cmd, GContext->GetSingleTexPPLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &overall);
        vkCmdPushConstants(cmd, GContext->GetSingleTexPPLayout(), VK_SHADER_STAGE_VERTEX_BIT, sizeof(glm::mat4), sizeof(glm::vec2), &info.uvStart);
        vkCmdPushConstants(cmd, GContext->GetSingleTexPPLayout(), VK_SHADER_STAGE_VERTEX_BIT, sizeof(glm::mat4) + sizeof(glm::vec2), sizeof(glm::vec2), &info.uvOffset);
        vkCmdDraw(cmd, 6, 1, 0, 0);
				
		cursorX += (info.Advance >> 6);
    }
}
UIType UIText::GetType() const {
    return UIType::Text;
}
void UIText::Save(std::ofstream& file) {
    size_t nameSize = 0;
    if(m_Asset) {
        nameSize = m_Asset->AssetName.size();
        file.write(reinterpret_cast<char*>(&nameSize), sizeof(size_t));
        file.write(reinterpret_cast<char*>(m_Asset->AssetName.data()), nameSize);
    }
    else file.write(reinterpret_cast<char*>(&nameSize), sizeof(size_t));

    size_t textSize = text.size();
    file.write(reinterpret_cast<char*>(&textSize), sizeof(size_t));
    if(textSize > 0) file.write(reinterpret_cast<char*>(text.data()), textSize);
}
void UIText::Load(std::ifstream& file, ModInstance* mod) {
    size_t nameSize = 0;
    file.read(reinterpret_cast<char*>(&nameSize), sizeof(size_t));
    if(nameSize > 0) {
        std::string assetName;
        assetName.resize(nameSize);
        file.read(reinterpret_cast<char*>(assetName.data()), nameSize);
        m_Asset = static_cast<FontAsset*>(mod->GetAllAssets()[assetName]);
    }

    size_t textSize;
    file.read(reinterpret_cast<char*>(&textSize), sizeof(size_t));
    if(textSize > 0) {
        text.resize(textSize);
        file.read(reinterpret_cast<char*>(text.data()), textSize);
    }
}