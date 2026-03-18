#include "AssetManager.h"

#include "AssetFormats/SkeletalMeshAsset.h"

#include <filesystem>

AssetManager* GAssets = nullptr; 

AssetManager::AssetManager() {
    GAssets = this;
}

void AssetManager::LoadAsset(const char* path, AssetType type) {
    std::string name = std::filesystem::path(path).stem().string();
    switch (type) {
        case AssetType::SkeletalMeshAsset:
            assets[name] = CreateAsset<SkeletalMeshAsset>(path, name);
            break;
        case AssetType::StaticMeshAsset:
            //assets.insert(LoadAsset<StaticMeshAsset>(path));
            break;
    }
}

std::unordered_map<std::string, Asset*>& AssetManager::GetAllAssets() {
    return assets;
}