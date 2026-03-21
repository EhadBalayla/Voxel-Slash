#pragma once
#include <cstdint>
#include <fstream>

enum class AssetType : uint8_t {
    SkeletalMeshAsset = 0,
    StaticMeshAsset = 1,
    TextureAsset = 2,
    
};

struct AssetHeader {
    AssetType type;
};

//sexy

class Asset {
public:
    AssetHeader header;
    std::string AssetName;

    void Load(const char* path);
    void Save(const char* path);
protected:
    virtual void Deserialize(std::ifstream& path) = 0;
    virtual void Serialize(std::ofstream& path) = 0;
};