#pragma once
#include <cstdint>

enum class AssetType : uint8_t {
    SkeletalMeshAsset = 0,
};

struct AssetHeader {
    AssetType type;
};

class Asset {
public:
    AssetHeader header;

    virtual void Load() = 0;
    virtual void Save() = 0;
};