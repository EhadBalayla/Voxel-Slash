#pragma once
#include "Asset.h"
#include "../Texture.h"

struct TextureMetaData {
    uint16_t Width;
    uint16_t Height;
    uint8_t Channels;
    bool TextureFlipX = false;
    bool TextureFlipY = false;

    size_t DataOffset; //the offset of the pixel data
    size_t DataSize;
};

class TextureAsset : public Asset {
public:
    TextureAsset();
    ~TextureAsset();

    TextureMetaData MetaData;

    Texture& GetTexture();
protected:
    void Deserialize(std::ifstream& file) override;
    void Serialize(std::ofstream& file) override;
private:
    Texture m_Texture;
    bool CreatedTexture = false;
};