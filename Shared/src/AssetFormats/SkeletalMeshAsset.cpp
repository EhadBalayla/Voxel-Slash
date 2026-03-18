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

    std::vector<SkeletalVertex> verticies(MetaData.verticiesSize / sizeof(SkeletalVertex));
    std::vector<uint32_t> indicies(MetaData.indiciesSize / sizeof(uint32_t));

    file.read(reinterpret_cast<char*>(verticies.data()), MetaData.verticiesSize);
    file.read(reinterpret_cast<char*>(indicies.data()), MetaData.indiciesSize);

    mesh.Create(verticies.data(), MetaData.verticiesSize, indicies.data(), MetaData.indiciesSize);
    CreatedMesh = true;
}
void SkeletalMeshAsset::Serialize(std::ofstream& file) {
    file.write(reinterpret_cast<char*>(&MetaData), sizeof(SkeletalMeshMetadata));
}

void SkeletalMeshAsset::Render(VkCommandBuffer cmd, VkPipelineLayout layout, glm::mat4 mtx) {
    vkCmdPushConstants(cmd, layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &mtx);
    VkDeviceSize offsets[] = { 0 };
    VkBuffer buffer = mesh.GetBuffer();
    vkCmdBindVertexBuffers(cmd, 0, 1, &buffer, offsets);
    vkCmdBindIndexBuffer(cmd, buffer, mesh.indiciesOffset, VK_INDEX_TYPE_UINT32);
    vkCmdDrawIndexed(cmd, MetaData.indiciesSize / sizeof(uint32_t), 1, 0, 0, 0);
}