#include "ModInstance.h"

#include "AssetFormats/SkeletalMeshAsset.h"

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
                GetAllAssets()[name] = CreateAsset<SkeletalMeshAsset>(n.path().string().c_str(), name);
                break;
            case AssetType::StaticMeshAsset:
                //assets.insert(LoadAsset<StaticMeshAsset>(path));
                break;
        }
    }

    //load all prefabs
    for(auto& n : std::filesystem::directory_iterator(std::filesystem::path(std::string(ModPath) + "/Prefabs"))) {
        if(n.is_directory() || n.path().extension().string() != ".pfb") continue;

        Prefab pfb;
        pfb.Load(n.path().string().c_str(), this);
        std::string name = n.path().stem().string();
        GetAllPrefabs()[name] = pfb;
    }
}
ModInstance::~ModInstance() {

}

std::unordered_map<std::string, Asset*>& ModInstance::GetAllAssets() {
    return assets;
}
std::unordered_map<std::string, Prefab>& ModInstance::GetAllPrefabs() {
    return prefabs;
}