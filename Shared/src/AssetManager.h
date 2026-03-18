#include <unordered_set>
#include "AssetFormats/Asset.h"

template <typename T>
T* CreateAsset(const char* path) {
    static_assert(std::is_base_of<Asset, T>::value, "T must be an Asset");
    T* asset = new T;
    asset->Load(path);
    return asset;
}

class AssetManager {
public:
    AssetManager();

    void LoadAsset(const char* path, AssetType type);
    void UnloadAsset(Asset* asset);

    std::unordered_set<Asset*>& GetAllAssets();
private:
    std::unordered_set<Asset*> assets;
};

extern AssetManager* GAssets;