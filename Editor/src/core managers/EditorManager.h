#pragma once
#include <vulkan/vulkan.h>
#include "imgui.h"
#include <vector>
#include "Prefab.h"

class EditorManager {
public:
    void Init();
    void Render();
    void Finalize();
    void Terminate();

    Prefab& GetPrefab();
private:
    VkDescriptorPool editorPool;
    std::vector<ImTextureID> viewportBuffers;

    Prefab m_Prefab;
    PrefabNode* selectedNode = nullptr;
    void RenderPrefabNodes(PrefabNode* m_Node);
};