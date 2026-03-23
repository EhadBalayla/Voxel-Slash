#include "FontAsset.h"

FontAsset::FontAsset() {
    header.type = AssetType::FontAsset;
}
FontAsset::~FontAsset() {
    if(m_Characters) free(m_Characters);
    if(CreatedTexture) m_Texture.Delete();
}

Texture& FontAsset::GetFontAtlas() {
    return m_Texture;
}

CharInfo FontAsset::GetCharacter(int idx) {
    if(idx >= MetaData.CharCount) return m_Characters[0];
    
    return m_Characters[idx];
}

void FontAsset::Deserialize(std::ifstream& file) {
    file.read(reinterpret_cast<char*>(&MetaData), sizeof(FontMetaData));

    if (MetaData.CharCount > 0) {
		m_Characters = (CharInfo*)malloc(sizeof(CharInfo) * MetaData.CharCount);
		file.read(reinterpret_cast<char*>(m_Characters), sizeof(CharInfo) * MetaData.CharCount);
	}

    unsigned char* pixelBuffer = (unsigned char*)malloc(MetaData.AtlasBufferSize);
    file.read(reinterpret_cast<char*>(pixelBuffer), MetaData.AtlasBufferSize);

    const int ATLAS_SIZE = 1024;
    m_Texture.Create(pixelBuffer, ATLAS_SIZE, ATLAS_SIZE);

    free(pixelBuffer);
}
void FontAsset::Serialize(std::ofstream& file) {
    file.write(reinterpret_cast<char*>(&MetaData), sizeof(FontMetaData));
}