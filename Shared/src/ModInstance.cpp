#include "ModInstance.h"

#include "AssetFormats/Asset.h"
#include "Prefab.h"
#include "Canvas.h"

#include "AssetFormats/SkeletalMeshAsset.h"
#include "AssetFormats/TextureAsset.h"

#include <filesystem>

template <typename T>
T* CreateAsset(const char* path, std::string name) {
    static_assert(std::is_base_of<Asset, T>::value, "T must be an Asset");
    T* asset = new T;
    asset->AssetName = name;
    asset->Load(path);
    return asset;
}

ModInstance::ModInstance(const char* ModPath) {
    //load all non voxel assets
    for(auto& n : std::filesystem::directory_iterator(std::filesystem::path(std::string(ModPath) + "/NonVoxelAssets"))) {
        if(n.is_directory() || n.path().extension().string() != ".vsa") continue;

        AssetType type;
        std::ifstream f(n.path().c_str(), std::ios::binary);
        f.read(reinterpret_cast<char*>(&type), sizeof(AssetType));
        f.close();
        std::string name = n.path().stem().string();
        switch (type) {
            case AssetType::SkeletalMeshAsset:
                assets[name] = CreateAsset<SkeletalMeshAsset>(n.path().string().c_str(), name);
                break;
            case AssetType::StaticMeshAsset:
                //assets.insert(LoadAsset<StaticMeshAsset>(path));
                break;
            case AssetType::TextureAsset:
                assets[name] = CreateAsset<TextureAsset>(n.path().string().c_str(), name);
                break;
        }
    }

    //load all prefabs
    for(auto& n : std::filesystem::directory_iterator(std::filesystem::path(std::string(ModPath) + "/Prefabs"))) {
        if(n.is_directory() || n.path().extension().string() != ".pfb") continue;

        Prefab* pfb = new Prefab;
        pfb->Load(n.path().string().c_str(), this);
        std::string name = n.path().stem().string();
        GetAllPrefabs()[name] = pfb;
    }

    //load all canvases
    for(auto& n : std::filesystem::directory_iterator(std::filesystem::path(std::string(ModPath) + "/Canvases"))) {
        if(n.is_directory() || n.path().extension().string() != ".cvs") continue;

        Canvas* cvs = new Canvas;
        cvs->Load(n.path().string().c_str(), this);
        std::string name = n.path().stem().string();
        GetAllCanvases()[name] = cvs;
    }
}
ModInstance::~ModInstance() {

}

std::unordered_map<std::string, Asset*>& ModInstance::GetAllAssets() {
    return assets;
}
std::unordered_map<std::string, Prefab*>& ModInstance::GetAllPrefabs() {
    return prefabs;
}
std::unordered_map<std::string, Canvas*>& ModInstance::GetAllCanvases() {
    return canvases;
}