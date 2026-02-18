#include "ChunkGenerator.h"
#include "../../Chunk.h"

#include <time.h>

ChunkGenerator::ChunkGenerator() {
    BiomeNoise.SetNoiseType(FastNoiseLite::NoiseType_Cellular);
    BiomeNoise.SetSeed(time(0));
    BiomeNoise.SetFrequency(0.005f);

    BiomeNoise.SetFractalType(FastNoiseLite::FractalType_FBm);
    BiomeNoise.SetFractalGain(0.0f);

    BiomeNoise.SetCellularDistanceFunction(FastNoiseLite::CellularDistanceFunction_Euclidean);
    BiomeNoise.SetCellularReturnType(FastNoiseLite::CellularReturnType_CellValue);
    BiomeNoise.SetCellularJitter(1.3f);




    Plains1.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    Plains1.SetSeed(69420);
    Plains1.SetFrequency(0.007f);

    Plains1.SetFractalType(FastNoiseLite::FractalType_FBm);
    Plains1.SetFractalOctaves(4);
    Plains1.SetFractalLacunarity(1.95);
    Plains1.SetFractalGain(0.48f);
    Plains1.SetFractalWeightedStrength(0.28f);


    Plains2.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
    Plains2.SetSeed(69420);
    Plains2.SetFrequency(0.016f);

    Plains2.SetFractalType(FastNoiseLite::FractalType_FBm);
    Plains2.SetFractalOctaves(5);
    Plains2.SetFractalLacunarity(1.43);
    Plains2.SetFractalGain(1.59);
    Plains2.SetFractalWeightedStrength(3.48f);


    PlainsSelector.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2S);
    PlainsSelector.SetSeed(69420);
    PlainsSelector.SetFrequency(0.003f);

    PlainsSelector.SetFractalType(FastNoiseLite::FractalType_Ridged);
    PlainsSelector.SetFractalOctaves(2);
    PlainsSelector.SetFractalLacunarity(0.78f);
    PlainsSelector.SetFractalGain(1.0f);
    PlainsSelector.SetFractalWeightedStrength(9.28f);





    IceSpikes1.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2S);
    IceSpikes1.SetSeed(69420);
    IceSpikes1.SetFrequency(0.01f);

    IceSpikes1.SetFractalType(FastNoiseLite::FractalType_Ridged);
    IceSpikes1.SetFractalOctaves(4);
    IceSpikes1.SetFractalLacunarity(1.59f);
    IceSpikes1.SetFractalGain(1.98f);
    IceSpikes1.SetFractalWeightedStrength(2.93f);


    IceSpikes2.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2S);
    IceSpikes2.SetSeed(69420);
    IceSpikes2.SetFrequency(0.032f);

    IceSpikes2.SetFractalType(FastNoiseLite::FractalType_Ridged);
    IceSpikes2.SetFractalOctaves(2);
    IceSpikes2.SetFractalLacunarity(1.24f);
    IceSpikes2.SetFractalGain(3.19f);
    IceSpikes2.SetFractalWeightedStrength(2.81f);


    IceSpikesSelector.SetNoiseType(FastNoiseLite::NoiseType_Cellular);
    IceSpikesSelector.SetSeed(69420);
    IceSpikesSelector.SetFrequency(0.032f);

    IceSpikesSelector.SetFractalType(FastNoiseLite::FractalType_Ridged);
    IceSpikesSelector.SetFractalOctaves(4);
    IceSpikesSelector.SetFractalLacunarity(1.01f);
    IceSpikesSelector.SetFractalGain(1.95f);
    IceSpikesSelector.SetFractalWeightedStrength(3.0f);




    Mesa1.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    Mesa1.SetSeed(69420);
    Mesa1.SetFrequency(0.002f);

    Mesa1.SetFractalType(FastNoiseLite::FractalType_FBm);
    Mesa1.SetFractalOctaves(3);
    Mesa1.SetFractalLacunarity(1.4f);
    Mesa1.SetFractalGain(1.6f);
    Mesa1.SetFractalWeightedStrength(0.36f);

    Mesa2.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    Mesa2.SetSeed(69420);
    Mesa2.SetFrequency(0.003f);

    Mesa2.SetFractalType(FastNoiseLite::FractalType_Ridged);
    Mesa2.SetFractalOctaves(5);
    Mesa2.SetFractalLacunarity(2.32f);
    Mesa2.SetFractalGain(1.85f);
    Mesa2.SetFractalWeightedStrength(0.33f);

    Mesa3.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
    Mesa3.SetSeed(69420);
    Mesa3.SetFrequency(0.015f);

    Mesa3.SetFractalType(FastNoiseLite::FractalType_FBm);
    Mesa3.SetFractalOctaves(4);
    Mesa3.SetFractalLacunarity(2.22f);
    Mesa3.SetFractalGain(0.53f);
    Mesa3.SetFractalWeightedStrength(0.26f);

    Mesa4.SetNoiseType(FastNoiseLite::NoiseType_Cellular);
    Mesa4.SetCellularReturnType(FastNoiseLite::CellularReturnType_Distance2Sub);
    Mesa4.SetFrequency(0.0016f);




    MountainBase.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    MountainBase.SetSeed(69420);           // different seed than mesas
    MountainBase.SetFrequency(0.0001f);    // low frequency for large mountains

    MountainBase.SetFractalType(FastNoiseLite::FractalType_Ridged);
    MountainBase.SetFractalOctaves(6);
    MountainBase.SetFractalLacunarity(2.0f);
    MountainBase.SetFractalGain(1.2f);
    MountainBase.SetFractalWeightedStrength(0.5f);
}
float lerp(float a, float b, float t) {
    return a + t * (b - a);
}
void ChunkGenerator::GenerateChunk(Chunk* c) {
    for(int x = 0; x < Chunk_Length; x++) {
        for(int z = 0; z < Chunk_Length; z++) {
            
            int LODFactor = GetLODSize(c->LOD);
            int WorldX = x * LODFactor + c->ChunkX * Chunk_Length * LODFactor;
            int WorldZ = z * LODFactor + c->ChunkZ * Chunk_Length * LODFactor;

            //float Biome = BiomeNoise.GetNoise(static_cast<float>(WorldX), static_cast<float>(WorldZ));
            int FinalY;
            //if(Biome < 0.5)
              //  FinalY = MakeIceSpikesHeight(WorldX, WorldZ);
            //else
                //FinalY = MakePlainsHeight(WorldX, WorldZ);
            FinalY = MakeMountainPeaks(WorldX, WorldZ);
            //FinalY = MakeMesaHeight(WorldX, WorldZ);
            //FinalY = MakeMountainPeaks(WorldX, WorldZ);

            for(int y = 0; y < Chunk_Length; y++) {
                int WorldY = y * LODFactor + c->ChunkY * Chunk_Length * LODFactor;

                if(WorldY == FinalY) {c->m_Blocks[IndexAt(x, y, z)] = BlockType::Grass; c->HasAnything = true; }
                else if(WorldY >= FinalY - 4 && WorldY < FinalY) {c->m_Blocks[IndexAt(x, y, z)] = BlockType::Dirt; c->HasAnything = true; }
                else if(WorldY < FinalY - 4) {c->m_Blocks[IndexAt(x, y, z)] = BlockType::Stone; c->HasAnything = true; }
            }
        }
    }
}


int ChunkGenerator::MakePlainsHeight(int WorldX, int WorldZ) {
    float n1 = Plains2.GetNoise(static_cast<float>(WorldX), static_cast<float>(WorldZ)) * 30;
    float n2 = Plains1.GetNoise(static_cast<float>(WorldX), static_cast<float>(WorldZ)) * 20;
    float a = PlainsSelector.GetNoise(static_cast<float>(WorldX), static_cast<float>(WorldZ));

    return lerp(n1, n2, a);
}
int ChunkGenerator::MakeIceSpikesHeight(int WorldX, int WorldZ) {
    float n1 = std::pow(IceSpikes1.GetNoise(static_cast<float>(WorldX), static_cast<float>(WorldZ)), 2) * 25;
    float n2 = IceSpikes2.GetNoise(static_cast<float>(WorldX), static_cast<float>(WorldZ)) * 4 + 3;
    float a = IceSpikesSelector.GetNoise(static_cast<float>(WorldX), static_cast<float>(WorldZ));

    return lerp(n2, n1, a);
}
int ChunkGenerator::MakeMesaHeight(int WorldX, int WorldZ) {
    float WarpX = Mesa2.GetNoise(static_cast<float>(WorldX) / 3.8f, static_cast<float>(WorldZ) / 10.7f);
    float WarpZ = Mesa2.GetNoise(static_cast<float>(WorldX) / 8.2f, static_cast<float>(WorldZ) / 4.5f);

    const float mesaScale = 125.0f;
    float h = (1 - Mesa1.GetNoise(static_cast<float>(WorldX) / 5.0f + WarpX * 50.0f, static_cast<float>(WorldZ) / 5.0f + WarpZ * 50.0f)) * mesaScale - 115;

    const float step = 15.0f;

    float mesaHeight = std::floor(h / step + 0.5f) * step + Mesa3.GetNoise(static_cast<float>(WorldX), static_cast<float>(WorldZ)) * 15;

    float canyon = Mesa4.GetNoise(static_cast<float>(WorldX), static_cast<float>(WorldZ));

    if (canyon < -0.1f) {
        float depth = (-0.1f - canyon) * 50.0f;
        mesaHeight -= depth;
    }   

    return mesaHeight;
}
int ChunkGenerator::MakeMountainPeaks(int WorldX, int WorldZ) {
    float warpX = Mesa2.GetNoise(static_cast<float>(WorldX), static_cast<float>(WorldZ)) * 2.0f;
    float warpZ = Mesa2.GetNoise(static_cast<float>(WorldX), static_cast<float>(WorldZ)) * 2.0f;

    const float mountainScale = 300.0f;
    float mountainHeight = MountainBase.GetNoise(static_cast<float>(WorldX) / 2.0f + warpX, static_cast<float>(WorldZ) / 2.0f + warpZ) * mountainScale;



    return mountainHeight;
}