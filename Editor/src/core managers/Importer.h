#pragma once
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <vector>

class Importer {
public:
    void ImportTexture(const char* path, const char* importPath);
    void ImportFont(const char* path, const char* importPath);
    void ImportSkeletalMesh(const char* folderPath);
    void ImportSound(const char* folderPath);

    void TraverseModelFile(const char* path, const char* folderPath);
    bool IsCachingModel();
    void FreeModelCache();
private:
    //caching assimp's shit
    void ProcessNode(aiNode* node, const aiScene* scene);
    void ProcessAnimations(const aiScene* scene);

    Assimp::Importer importer;

    const aiScene* cachedScene = nullptr;
    std::vector<aiNode*> cachedStaticMeshes;
    std::vector<aiNode*> cachedSkeletalMeshes;
    std::vector<aiAnimation*> cachedAnimations;
};