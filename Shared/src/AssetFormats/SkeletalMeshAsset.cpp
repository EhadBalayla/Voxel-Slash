#include "SkeletalMeshAsset.h"
#include <fstream>
#include <vector>

#include "../VertexStruct.h"

SkeletalMeshAsset::SkeletalMeshAsset() {
    header.type = AssetType::SkeletalMeshAsset;
}
SkeletalMeshAsset::~SkeletalMeshAsset() {
    if(CreatedMesh) mesh.Delete();
}

void SkeletalMeshAsset::Deserialize(std::ifstream& file) {
    file.read(reinterpret_cast<char*>(&MetaData), sizeof(SkeletalMeshMetadata));

    std::vector<SkeletalVertex> verticies;
    std::vector<uint32_t> indicies;

    verticies.resize(MetaData.verticiesSize / sizeof(SkeletalVertex));
    indicies.resize(MetaData.indiciesSize / sizeof(uint32_t));

    file.read(reinterpret_cast<char*>(verticies.data()), MetaData.verticiesSize);
    file.read(reinterpret_cast<char*>(indicies.data()), MetaData.indiciesSize);

    mesh.Create(verticies.data(), MetaData.verticiesSize, indicies.data(), MetaData.indiciesSize);
    CreatedMesh = true;
}
void SkeletalMeshAsset::Serialize(std::ofstream& file) {
    file.write(reinterpret_cast<char*>(&MetaData), sizeof(SkeletalMeshMetadata));
}

void SkeletalMeshAsset::Render(VkCommandBuffer cmd, glm::mat4 mtx) {
    
}