#pragma once
#include <vector>
#include <unordered_map>
#include <string>
#include <glm/glm.hpp>
#include <vulkan/vulkan.h>
#include <fstream>

class ModInstance;
class TextureAsset;
class FontAsset;
class Texture;
struct UINode;

enum class UIType {
    Image = 1,
    Button = 2,
    Text = 3,
    ProgressBar = 4,
};

class UIElement {
public:
    UINode* m_Node = nullptr;

    virtual ~UIElement() = default;

    virtual void Render(VkCommandBuffer cmd, VkSampler smp, glm::mat4 mtx) = 0;
    virtual void Tick() {}
    virtual UIType GetType() const = 0;

    virtual void Save(std::ofstream& file) = 0;
    virtual void Load(std::ifstream& file, ModInstance* mod) = 0;
};

class UIImage : public UIElement {
public:
    UIImage();

    void Render(VkCommandBuffer cmd, VkSampler smp, glm::mat4 mtx) override;
    UIType GetType() const override;

    void Save(std::ofstream& file) override;
    void Load(std::ifstream& file, ModInstance* mod) override;

    TextureAsset* m_Asset = nullptr;
private:
    VkDescriptorPool pool;
    std::vector<VkDescriptorSet> sets; //for all the textures
    Texture* texturesPerSet[3] = { nullptr }; //basically referencing which texture each descriptor set holds, so we will know to update if needed
};

class UIButton : public UIElement {
public:
    UIButton();

    void Render(VkCommandBuffer cmd, VkSampler smp, glm::mat4 mtx) override;
    void Tick() override;
    UIType GetType() const override;

    void Save(std::ofstream& file) override;
    void Load(std::ifstream& file, ModInstance* mod) override;
    TextureAsset* m_Asset = nullptr;

    //callbacks
    void(*OnHovered)() = nullptr;
    void(*OnPress)() = nullptr;
private:
    VkDescriptorPool pool;
    std::vector<VkDescriptorSet> sets; //for all the textures
    Texture* texturesPerSet[3] = { nullptr }; //basically referencing which texture each descriptor set holds, so we will know to update if needed

    bool IsHovering = false;
    bool IsClicking = false;
};

class UIText : public UIElement {
public:
    UIText();

    void Render(VkCommandBuffer cmd, VkSampler smp, glm::mat4 mtx);
    UIType GetType() const override;

    void Save(std::ofstream& file) override;
    void Load(std::ifstream& file, ModInstance* mod) override;

    FontAsset* m_Asset = nullptr;
    std::string text = "Sexy Text";
private:
    VkDescriptorPool pool;
    std::vector<VkDescriptorSet> sets; //for all the textures
    Texture* texturesPerSet[3] = { nullptr }; //basically referencing which texture each descriptor set holds, so we will know to update if needed
};

struct UINode {
    UINode* m_Parent = nullptr;
    std::vector<UINode*> m_Children;

    std::string m_Name = "Empty Element";

    //transform for the UINode
    glm::vec2 Position = glm::vec2(0.0f);
    float Rotation = 0.0f;
    glm::vec2 Size = glm::vec2(1.0f);

    //anchors for the UINode
    float Left = 0.0f, Right = 1.0f, Bottom = 1.0f, Top = 0.0f;

    //contents of the node
    UIElement* m_Element = nullptr;
};
void AddNewUINode(UINode* parentNode, std::string newName);
void SetUIElement(UINode* node, UIType type);

class Canvas {
public:
    void Save(const char* path);
    void Load(const char* path, ModInstance* mod);
    
    std::vector<UINode*> nodes; //top level nodes
    
    void Tick();
    void Render(VkCommandBuffer cmd, VkSampler smp, int ScrWidth, int ScrHeight);
private:
    std::unordered_map<std::string, UINode*> NameNodeMap;
};