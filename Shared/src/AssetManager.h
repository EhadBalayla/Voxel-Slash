#include <unordered_map>
#include <string>
#include "AssetFormats/Asset.h"

template <typename T>
T* CreateAsset(const char* path, std::string name) {
    static_assert(std::is_base_of<Asset, T>::value, "T must be an Asset");
    T* asset = new T;
    asset->AssetName = name;
    asset->Load(path);
    return asset;
}

class AssetManager {
public:
    AssetManager();

    void LoadAsset(const char* path, AssetType type);

    std::unordered_map<std::string, Asset*>& GetAllAssets();
private:
    std::unordered_map<std::string, Asset*> assets;
};

extern AssetManager* GAssets;