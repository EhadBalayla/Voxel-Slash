#include "ClientChunk.h"
#include "../core managers/app.h"

#include <glm/gtc/matrix_transform.hpp>

enum class Face {
    Top,
	Bottom,
	Left,
	Right,
	Front,
	Back
};

uint32_t GetFaceM(glm::ivec3 pos, uint8_t texIndex, uint8_t faceDir) {
    uint32_t ret = 0;

    ret |= faceDir << 23;
    ret |= texIndex << 15;
    ret |= pos.y << 10;
    ret |= pos.z << 5;
    ret |= pos.x;

    return ret;
}
void AddFaceM(glm::ivec3 pos, Face face, uint8_t texOffset, std::vector<uint32_t>& faces) {
    faces.push_back(GetFaceM(pos, texOffset, static_cast<uint8_t>(face)));
    return;
}


void ClientChunk::Render() {
    if(HasOpaque) {
        ChunkRenderer& renderer = GApp->m_ChunkRenderer;

        int LODFactor = 1;//GetLODSize(LOD);
        //glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(ChunkX * Chunk_Length * LODFactor, ChunkY * Chunk_Length * LODFactor, ChunkZ * Chunk_Length * LODFactor));
        glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(ChunkX * 32 * LODFactor, ChunkY * 32 * LODFactor, ChunkZ * 32 * LODFactor));
        model = glm::scale(model, glm::vec3(static_cast<float>(LODFactor)));
        
        renderer.SetTrans(model);

        VkDescriptorSet sets[] = { renderer.GetChunksSet(GContext->currentFrame), mesh.opaqueMeshBuffer.GetDescriptorSet(GContext->currentFrame) };
        uint32_t setsCount = 2;
        vkCmdBindDescriptorSets(GRenderer->GetFrameCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, renderer.GetChunksPipelineLayout(), 0, setsCount, sets, 0, nullptr);
        vkCmdDraw(GRenderer->GetFrameCommandBuffer(), mesh.opaqueCount * 6, 1, 0, 0);
    }
}


void ClientChunk::GenerateMeshData() {
    uint32_t indexOffset = 0;

    for(int x = 0; x < 32; x++) {
        for(int y = 0; y < 32; y++) {
            for(int z = 0; z < 32; z++) {
                BlockType type = m_Blocks[IndexAt(x, y, z)];
                if(type == BlockType::Air) continue;

                glm::ivec3 blockPos(x, y, z);

                auto IsAir = [&](int dx, int dy, int dz) -> bool {
                    int nx, ny, nz;
                    nx = x + dx, ny = y + dy, nz = z + dz;
                    if (nx < 0) {
                        BlockType b = GApp->m_MPWorld->m_ClientChunkManager.GetChunk(glm::i64vec3(ChunkX - 1, ChunkY, ChunkZ))->m_Blocks[IndexAt(32 - 1, ny, nz)];
                        return b == BlockType::Air;
                    }
                    else if (nx >= 32) {
                        BlockType b = GApp->m_MPWorld->m_ClientChunkManager.GetChunk(glm::i64vec3(ChunkX + 1, ChunkY, ChunkZ))->m_Blocks[IndexAt(0, ny, nz)];
                        return b == BlockType::Air;
                    }
                    else if (ny < 0) {
                        BlockType b = GApp->m_MPWorld->m_ClientChunkManager.GetChunk(glm::i64vec3(ChunkX, ChunkY - 1, ChunkZ))->m_Blocks[IndexAt(nx, 32 - 1, nz)];
                        return b == BlockType::Air;
                    }
                    else if (ny >= 32) {
                        BlockType b = GApp->m_MPWorld->m_ClientChunkManager.GetChunk(glm::i64vec3(ChunkX, ChunkY + 1, ChunkZ))->m_Blocks[IndexAt(nx, 0, nz)];
                        return b == BlockType::Air;
                    }
                    else if (nz < 0) {
                        BlockType b = GApp->m_MPWorld->m_ClientChunkManager.GetChunk(glm::i64vec3(ChunkX, ChunkY, ChunkZ - 1))->m_Blocks[IndexAt(nx, ny, 32 - 1)];
                        return b == BlockType::Air;
                    }
                    else if (nz >= 32) {
                        BlockType b = GApp->m_MPWorld->m_ClientChunkManager.GetChunk(glm::i64vec3(ChunkX, ChunkY, ChunkZ + 1))->m_Blocks[IndexAt(nx, ny, 0)];
                        return b == BlockType::Air;
                    }

                    
                    return m_Blocks[IndexAt(nx, ny, nz)] == BlockType::Air;
                    };
                
                BlockData bd = GApp->BlockRegistery[type];
                if(IsAir(0, 0, -1)) AddFaceM(blockPos, Face::Back, bd.uvs.backUV, meshData.opaqueFaces);
                if(IsAir(0, 0, 1)) AddFaceM(blockPos, Face::Front, bd.uvs.frontUV, meshData.opaqueFaces);
                if(IsAir(-1, 0, 0)) AddFaceM(blockPos, Face::Left, bd.uvs.leftUV, meshData.opaqueFaces);
                if(IsAir(1, 0, 0)) AddFaceM(blockPos, Face::Right, bd.uvs.rightUV, meshData.opaqueFaces);
                if(IsAir(0, 1, 0)) AddFaceM(blockPos, Face::Top, bd.uvs.topUV, meshData.opaqueFaces);
                if(IsAir(0, -1, 0)) AddFaceM(blockPos, Face::Bottom, bd.uvs.bottomUV, meshData.opaqueFaces);
            }
        }
    }
}
void ClientChunk::UploadMeshData() {
    if(meshData.opaqueFaces.size() > 0) {
        // Upload to GPU
        mesh.opaqueMeshBuffer.Update(meshData.opaqueFaces.data(), meshData.opaqueFaces.size() * sizeof(uint32_t));
        mesh.opaqueCount = meshData.opaqueFaces.size();

        meshData.opaqueFaces.clear();
        std::vector<uint32_t>().swap(meshData.opaqueFaces);

        HasOpaque = true;
    } else HasOpaque = false;
}