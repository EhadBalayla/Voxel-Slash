#include "TextureAsset.h"

TextureAsset::TextureAsset() {
    header.type = AssetType::TextureAsset;
}
TextureAsset::~TextureAsset() {
    if(CreatedTexture) m_Texture.Delete();
}

void TextureAsset::Deserialize(std::ifstream& file) {
    file.read(reinterpret_cast<char*>(&MetaData), sizeof(TextureMetaData));

    unsigned char* pixelData = (unsigned char*)malloc(MetaData.DataSize);

    m_Texture.Create(pixelData, MetaData.Width, MetaData.Height);
    CreatedTexture = true;
    
    free(pixelData);
}
void TextureAsset::Serialize(std::ofstream& file) {
    file.write(reinterpret_cast<char*>(&MetaData), sizeof(TextureMetaData));
}