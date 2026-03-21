#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <string>
#include "imgui.h"

#include "Prefab.h"
#include "Importer.h"

#include "AssetFormats/Asset.h"

class Canvas;
class UINode;

struct AssetRef {
    std::string name;
    AssetType type;
};

class EditorManager {
public:
    void Init();
    void Render();
    void Finalize();
    void Terminate();

    Prefab& GetPrefab();
    Canvas* GetCanvas() const;
private:
    //for the chosen data folder
    bool IsDataFolderChosen = false;
    std::string DataFolder = "none";

    //for editor references of available assets and prefabs
    std::vector<AssetRef> AssetList;
    void RefreshAssetList();

    //editor Vulkan stuff
    VkDescriptorPool editorPool;
    std::vector<ImTextureID> viewportBuffers;

    //prefab stuff
    Prefab m_Prefab;
    PrefabNode* selectedNode = nullptr;
    void RenderPrefabNodes(PrefabNode* m_Node);

    //canvas stuff
    Canvas* m_Canvas = nullptr;
    UINode* selectedUIElement = nullptr;
    void RenderUINodes(UINode* m_Node);

    //importer
    Importer m_Importer;
};