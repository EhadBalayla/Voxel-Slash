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
    //land noises
    FastNoiseLite landSelectorNoise;
    FastNoiseLite flatNoise;
    FastNoiseLite hillyNoise;
    FastNoiseLite mountainNoise;

    //detail noise
    FastNoiseLite detailNoise;
};