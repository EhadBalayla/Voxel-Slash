#pragma once
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

class Importer {
public:
    void ImportTexture(const char* folderPath);
    void ImportSkeletalMesh(const char* folderPath);
    void ImportSound(const char* folderPath);

    void TraverseModelFile(const char* path);
    void FreeModelCache();
private:
    Assimp::Importer importer;
};