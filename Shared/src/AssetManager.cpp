#include "AssetManager.h"

void AssetManager::UnloadAsset(Asset* asset) {
    assets.erase(asset);
    delete asset;
}