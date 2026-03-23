#pragma once
#include "TransformAsset.h"
#include "../MeshBuffer.h"

struct SkeletalMeshMetadata {
    size_t offset;
    size_t verticiesSize;
    size_t indiciesSize;
};

class SkeletalMeshAsset : public TransformAsset {
public:
    SkeletalMeshAsset();
    ~SkeletalMeshAsset();

    SkeletalMeshMetadata MetaData;

    void Render(VkCommandBuffer cmd, VkPipelineLayout layout, glm::mat4 mtx) override;
protected:
    void Deserialize(std::ifstream& file) override;
    void Serialize(std::ofstream& file) override;
private:
    MeshBuffer mesh;
    bool CreatedMesh = false;
};