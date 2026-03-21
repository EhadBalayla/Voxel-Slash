#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <vulkan/vulkan.h>
#include <fstream>

class ModInstance;
class TextureAsset;

enum class UIType {
    Image = 1,
    Button = 2,
    Text = 3,
    ProgressBar = 4,
};

class UIElement {
public:
    virtual ~UIElement() = default;

    virtual void Render(VkCommandBuffer cmd) = 0;
    virtual UIType GetType() const = 0;

    virtual void Save(std::ofstream& file) = 0;
    virtual void Load(std::ifstream& file, ModInstance* mod) = 0;
};

class UIImage : public UIElement {
public:
    void Render(VkCommandBuffer cmd) override;
    UIType GetType() const override;

    void Save(std::ofstream& file) override;
    void Load(std::ifstream& file, ModInstance* mod) override;

    TextureAsset* m_Asset = nullptr;
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
    
    void Render(VkCommandBuffer cmd, VkPipelineLayout layout, int ScrWidth, int ScrHeight);
};