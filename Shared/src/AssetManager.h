#include <unordered_set>
#include "AssetFormats/Asset.h"

class AssetManager {
public:
    template <typename T>
    T* LoadAsset(const char* path) {
        static_assert(std::is_base_of<Asset, T>::value, "T must be an Asset");
        T* asset = new T;
        asset->Load(path);
        assets.insert(asset);
        return asset;
    }
    void UnloadAsset(Asset* asset);
private:
    std::unordered_set<Asset*> assets;
};