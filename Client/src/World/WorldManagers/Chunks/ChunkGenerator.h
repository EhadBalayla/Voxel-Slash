#pragma once
#include "FastNoiseLight.h"
#include <cstdint>

class Chunk;
class ChunkGenerator {
public:
    ChunkGenerator();
    ChunkGenerator(int64_t seed);

    void GenerateChunk(Chunk* c);
    void ReplaceBlocks(Chunk* c);
    void CarveCaves(Chunk* c);
private:
    FastNoiseLite masterNoise; // Slow, huge scale (0.0001)
    FastNoiseLite mountainNoise;      // Medium scale (0.01)
    FastNoiseLite detailNoise;           // 3D noise for overhangs (0.02)
};