#pragma once
#include <cstdint>

enum class BlockType : uint8_t {
    Air = 0,
    Stone = 1,
    Grass = 2,
    Dirt = 3,
    Sand = 4,

    Water = 69,
};

struct BlockUV {
    uint8_t topUV;
    uint8_t bottomUV;
    uint8_t leftUV;
    uint8_t rightUV;
    uint8_t backUV;
    uint8_t frontUV;
};

struct BlockData {
    BlockUV uvs;
};