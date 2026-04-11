#pragma once
#include "FastNoiseLight.h"
#include <cstdint>

class Chunk;
class ChunkManager;
class ChunkGenerator {
public:
    ChunkGenerator(ChunkManager* manager);
    ChunkGenerator(int64_t seed, ChunkManager* manager);

    void GenerateChunk(Chunk* c);
    void ReplaceBlocks(Chunk* c);
    void CarveCaves(Chunk* c);

    void Populate(Chunk* c);
private:
    ChunkManager* owningManager;

    FastNoiseLite continentalNoise;
    //land noises
    FastNoiseLite landSelectorNoise;
    FastNoiseLite flatNoise;
    FastNoiseLite hillyNoise;
    FastNoiseLite mountainNoise;

    //detail noise
    FastNoiseLite detailNoise;

    int64_t WorldSeed;
};