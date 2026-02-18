#pragma once
#include "FastNoiseLight.h"

class Chunk;
class ChunkGenerator {
public:
    ChunkGenerator();

    void GenerateChunk(Chunk* c);
private:
    FastNoiseLite BiomeNoise;


    FastNoiseLite Plains1;
    FastNoiseLite Plains2;
    FastNoiseLite PlainsSelector;

    FastNoiseLite IceSpikes1;
    FastNoiseLite IceSpikes2;
    FastNoiseLite IceSpikesSelector;

    FastNoiseLite Mesa1;
    FastNoiseLite Mesa2;
    FastNoiseLite Mesa3;
    FastNoiseLite Mesa4;

    FastNoiseLite MountainBase;


    int MakePlainsHeight(int WorldX, int WorldZ);
    int MakeIceSpikesHeight(int WorldX, int WorldZ);
    int MakeMesaHeight(int WorldX, int WorldZ);
    int MakeMountainPeaks(int WorldX, int WorldZ);
};