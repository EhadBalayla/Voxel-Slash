#include "TextureAsset.h"

TextureAsset::TextureAsset() {
    header.type = AssetType::TextureAsset;
}
TextureAsset::~TextureAsset() {
    if(CreatedTexture) m_Texture.Delete();
}

Texture& TextureAsset::GetTexture() {
    return m_Texture;
}

void TextureAsset::Deserialize(std::ifstream& file) {
    file.read(reinterpret_cast<char*>(&MetaData), sizeof(TextureMetaData));

    unsigned char* pixelData = (unsigned char*)malloc(MetaData.DataSize);

    file.seekg(MetaData.DataOffset, std::ios::beg);
    file.read(reinterpret_cast<char*>(pixelData), MetaData.DataSize);

    m_Texture.Create(pixelData, static_cast<int>(MetaData.Width), static_cast<int>(MetaData.Height));
    CreatedTexture = true;
    
    free(pixelData);
}
void TextureAsset::Serialize(std::ofstream& file) {
    file.write(reinterpret_cast<char*>(&MetaData), sizeof(TextureMetaData));
}