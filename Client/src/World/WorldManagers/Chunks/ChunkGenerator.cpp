#include "ChunkGenerator.h"
#include "../../Chunk.h"

#include <algorithm>
#include <ctime>

//helper functions
float smoothstep(float edge0, float edge1, float x) {
    float t = std::clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
}

ChunkGenerator::ChunkGenerator() : ChunkGenerator(static_cast<int64_t>(std::time(nullptr))) {

}
ChunkGenerator::ChunkGenerator(int64_t seed) {
    masterNoise.SetSeed(seed);
    masterNoise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    masterNoise.SetFrequency(0.001f); 

    // 2. MOUNTAINS: Ridged noise for sharp peaks
    mountainNoise.SetSeed(seed + 1);
    mountainNoise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    mountainNoise.SetFractalType(FastNoiseLite::FractalType_Ridged);
    mountainNoise.SetFractalOctaves(4);
    mountainNoise.SetFrequency(0.005f);

    // 3. DETAIL: Small bumps for plains
    detailNoise.SetSeed(seed + 2);
    detailNoise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    detailNoise.SetFrequency(0.02f);
}
void ChunkGenerator::GenerateChunk(Chunk* c) {
    int LODSize = GetLODSize(c->LOD);
    float Density = 0.0f;

    for(int x = 0; x < Chunk_Length; x++) {
        for(int z = 0; z < Chunk_Length; z++) {
            int WorldX = x * LODSize + c->ChunkX * Chunk_Length * LODSize;
            int WorldZ = z * LODSize + c->ChunkZ * Chunk_Length * LODSize;

            float Height = masterNoise.GetNoise((float) WorldX, (float)WorldZ) * 10.0f - 5.0f;

            for(int y = 0; y < Chunk_Length; y++) {
                float WorldY = (float)(y * LODSize + c->ChunkY * Chunk_Length * LODSize);
                int index = IndexAt(x, y, z);

                if(WorldY <= Height) {
                    c->m_Blocks[index] = BlockType::Stone;
                    c->HasAnything = true;
                }
            }
        }
    }
}