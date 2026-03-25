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
    FastNoiseLite continentalNoise;
    FastNoiseLite flatNoise;
    FastNoiseLite hillyNoise;
    FastNoiseLite mountainNoise;
    FastNoiseLite landSelectorNoise;
    FastNoiseLite noise1;
    FastNoiseLite noise2;
};