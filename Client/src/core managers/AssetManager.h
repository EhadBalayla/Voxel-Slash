#pragma once
#include <unordered_map>
#include <string>

class Asset;

class AssetManager {
public:

private:
    std::unordered_map<std::string, Asset*> assetCache;
};