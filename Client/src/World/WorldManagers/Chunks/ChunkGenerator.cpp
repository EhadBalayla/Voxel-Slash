#include "ChunkGenerator.h"
#include "../../Chunk.h"

#include <algorithm>
#include <ctime>

#include <iostream>

//helper functions
float smoothstep(float edge0, float edge1, float x) {
    float t = std::clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
}
float lerp(float a, float b, float t) {
    return a + t * (b - a);
}

ChunkGenerator::ChunkGenerator() : ChunkGenerator(static_cast<int64_t>(std::time(nullptr))) {

}
ChunkGenerator::ChunkGenerator(int64_t seed) {
    std::cout << seed << std::endl;

    //the noise for where there is a continent and where there is ocean
    continentalNoise.SetSeed(seed);
    continentalNoise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    continentalNoise.SetFrequency(0.001f);
    continentalNoise.SetFractalOctaves(5);
    continentalNoise.SetFractalGain(0.42f);
    continentalNoise.SetFractalLacunarity(2.0f);
    
    //all noises for a continent
    landSelectorNoise.SetSeed(seed + 1);
    landSelectorNoise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    landSelectorNoise.SetFrequency(0.0005f);
    landSelectorNoise.SetFractalOctaves(3);
    landSelectorNoise.SetFractalGain(0.3f);
    landSelectorNoise.SetFractalLacunarity(2.0f);

    flatNoise.SetSeed(seed + 2);
    flatNoise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    flatNoise.SetFrequency(0.015f);
    flatNoise.SetFractalOctaves(6);
    flatNoise.SetFractalGain(0.5f);
    flatNoise.SetFractalLacunarity(2.5f);

    hillyNoise.SetSeed(seed + 3);
    hillyNoise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    hillyNoise.SetFrequency(0.005f);
    hillyNoise.SetFractalOctaves(6);
    hillyNoise.SetFractalGain(0.55f);
    hillyNoise.SetFractalLacunarity(2.0f);

    mountainNoise.SetSeed(seed);
    mountainNoise.SetNoiseType(FastNoiseLite::NoiseType_Cellular);
    mountainNoise.SetFrequency(0.002f);
    mountainNoise.SetFractalOctaves(5);
    mountainNoise.SetFractalGain(0.5f);
    mountainNoise.SetFractalLacunarity(2.0f);


    //detail noises
    detailNoise.SetSeed(seed);
    detailNoise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    detailNoise.SetFractalOctaves(6);
    detailNoise.SetFrequency(0.05f);
    detailNoise.SetFractalLacunarity(1.7f);
    detailNoise.SetFractalGain(0.4f);
}
void ChunkGenerator::GenerateChunk(Chunk* c) {
    int LODSize = GetLODSize(c->LOD);

    for(int x = 0; x < Chunk_Length; x++) {
        int WorldX = x * LODSize + c->ChunkX * Chunk_Length * LODSize;

        for(int z = 0; z < Chunk_Length; z++) {
            int WorldZ = z * LODSize + c->ChunkZ * Chunk_Length * LODSize;

            float Height = 0.0f;
            float continental = continentalNoise.GetNoise((float)WorldX, (float)WorldZ) * -4.0f;

            Height = continental;

            float flatFactor = 0.0f;
            float hillFactor = 0.0f;
            float mountainFactor = 0.0f;

            float n1 = flatNoise.GetNoise((float)WorldX, (float)WorldZ);
            float n2 = hillyNoise.GetNoise((float)WorldX, (float)WorldZ);
            float n3 = mountainNoise.GetNoise((float)WorldX, (float)WorldZ);

            float flat = std::pow(n1, 5.0f) * -5.0f + 0.5f;
            float hilly = (std::pow((n2 + 1.0f) * -0.5f, 2.0) + 0.25f) * 30.0f;
            float mountainous = ((n3 + 1.0f) * 0.5f) * 600.0f + 30.0f + n1 * 10.0f;

            float landSelector = (landSelectorNoise.GetNoise((float)WorldX, (float)WorldZ) + 1.0f) * 0.5f;

            flatFactor = std::clamp((0.33f - landSelector) / 0.33f, 0.0f, 1.0f);
            hillFactor = std::clamp((landSelector - 0.0f) / 0.33f, 0.0f, 1.0f) * std::clamp((0.66f - landSelector) / 0.33f, 0.0f, 1.0f);
            mountainFactor = std::clamp((landSelector - 0.66f) / 0.34f, 0.0f, 1.0f);
                
            Height += flat * flatFactor + hilly * hillFactor + mountainous * mountainFactor;

            for(int y = 0; y < Chunk_Length; y++) {
                int WorldY = y * LODSize + c->ChunkY * Chunk_Length * LODSize;

                int idx = IndexAt(x, y, z);


                float Density = Height - WorldY;

                /*float microAmplitude = 0.0f;
                microAmplitude += flatFactor * 0.5f;
                microAmplitude += hillFactor * 1.5f;
                microAmplitude += mountainFactor * 3.5f;
                
                float detail = detailNoise.GetNoise((float)WorldX * 1.5f, (float)WorldY * 3.5f, (float)WorldZ * 1.5f) * microAmplitude;
                
                Density += detail;*/

                if(Density > 0.0f) {
                    c->m_Blocks[idx] = BlockType::Stone;
                    c->HasAnything = true;
                } else {
                    if(WorldY <= 0) {
                        c->m_Blocks[idx] = BlockType::Water;
                        c->HasAnything = true;
                    } else {
                        c->m_Blocks[idx] = BlockType::Air;
                    }
                }
            }
        }
    }
}
void ChunkGenerator::ReplaceBlocks(Chunk* c) {
    /*for(int x = 0; x < Chunk_Length; x++) {
        for(int z = 0; z < Chunk_Length; z++) {
            int GrassCountXZ = -1; //counting blocks down
            for(int y = Chunk_Length - 1; y >= 0; y--) {
                int idx = IndexAt(x, y, z);
                    
                if(c->m_Blocks[idx] != BlockType::Air && c->m_Blocks[idx] != BlockType::Water) {
                    if(y == Chunk_Length - 1) {
                        c->m_Blocks[idx] = BlockType::Grass;
                        c->HasAnything = true;
                        GrassCountXZ = 0;
                    }
                    else {
                        if(c->m_Blocks[IndexAt(x, y + 1, z)] == BlockType::Air) {
                            c->m_Blocks[idx] = BlockType::Grass;
                            c->HasAnything = true;
                            GrassCountXZ = 0;
                        }
                        else {
                            if(GrassCountXZ > -1 && GrassCountXZ < 2) { c->m_Blocks[idx] = BlockType::Dirt;   GrassCountXZ++; }
                            else { c->m_Blocks[idx] = BlockType::Stone; };
                        }
                    }
                }
            }
        }
    }*/
}
void ChunkGenerator::CarveCaves(Chunk* c) {

}