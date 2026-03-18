#include "AssetManager.h"

#include "AssetFormats/SkeletalMeshAsset.h"

AssetManager* GAssets = nullptr; 

AssetManager::AssetManager() {
    GAssets = this;
}

void AssetManager::LoadAsset(const char* path, AssetType type) {
    switch (type) {
        case AssetType::SkeletalMeshAsset: 
            assets.insert(CreateAsset<SkeletalMeshAsset>(path));
            break;
        case AssetType::StaticMeshAsset:
            //assets.insert(LoadAsset<StaticMeshAsset>(path));
            break;
    }
}
void AssetManager::UnloadAsset(Asset* asset) {
    assets.erase(asset);
    delete asset;
}

std::unordered_set<Asset*>& AssetManager::GetAllAssets() {
    return assets;
}