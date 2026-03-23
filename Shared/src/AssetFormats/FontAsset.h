#pragma once
#include "Asset.h"
#include "../Texture.h"
#include <glm/glm.hpp>

struct CharInfo {
	uint32_t idx;
	unsigned int Advance;
	glm::ivec2 Bearing;
	glm::ivec2 Size;
};

struct FontMetaData {
    size_t DataOffset;
    size_t CharCount;
    size_t AtlasBufferSize;
};

class FontAsset : public Asset {
public:
    FontAsset();
    ~FontAsset();

    FontMetaData MetaData;

    Texture& GetFontAtlas();

    CharInfo GetCharacter(int idx);
protected:
    void Deserialize(std::ifstream& file) override;
    void Serialize(std::ofstream& file) override;
private:
    Texture m_Texture;
    bool CreatedTexture = false;

    CharInfo* m_Characters = nullptr;
};