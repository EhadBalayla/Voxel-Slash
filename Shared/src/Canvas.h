#include <vector>
#include <string>
#include <glm/glm.hpp>

class ModInstance;

struct UINode {
    UINode* m_Parent = nullptr;
    std::vector<UINode*> m_Children;

    std::string m_Name = "Empty Element";

    //transform for the UINode
    glm::vec2 Position = glm::vec2(0.0f);
    float Rotation = 0.0f;
    glm::vec2 Size = glm::vec2(1.0f);
};
void AddNewUINode(UINode* parentNode, std::string newName);

class Canvas {
public:
    void Save(const char* path);
    void Load(const char* path, ModInstance* mod);

    std::vector<UINode*> nodes; //top level nodes
};