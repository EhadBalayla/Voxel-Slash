#include "Chunk.h"
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

uint32_t GetFace(glm::ivec3 pos, uint8_t texIndex, uint8_t faceDir) {
    uint32_t ret = 0;

    ret |= faceDir << 23;
    ret |= texIndex << 15;
    ret |= pos.y << 10;
    ret |= pos.z << 5;
    ret |= pos.x;

    return ret;
}
void AddFace(glm::ivec3 pos, Face face, uint8_t texOffset, std::vector<uint32_t>& faces) {
    faces.push_back(GetFace(pos, texOffset, static_cast<uint8_t>(face)));
    return;
}


void Chunk::Render() {
    if(HasOpaque) {
        ChunkRenderer& renderer = GApp->m_ChunkRenderer;

        int LODFactor = GetLODSize(LOD);
        glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(ChunkX * Chunk_Length * LODFactor, ChunkY * Chunk_Length * LODFactor, ChunkZ * Chunk_Length * LODFactor));
        model = glm::scale(model, glm::vec3(static_cast<float>(LODFactor)));
        
        renderer.SetTrans(model);

        VkDescriptorSet sets[] = { renderer.GetChunksSet(GContext->currentFrame), mesh.opaqueMeshBuffer.GetDescriptorSet(GContext->currentFrame) };
        uint32_t setsCount = 2;
        vkCmdBindDescriptorSets(GRenderer->GetFrameCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, renderer.GetChunksPipelineLayout(), 0, setsCount, sets, 0, nullptr);
        vkCmdDraw(GRenderer->GetFrameCommandBuffer(), mesh.opaqueCount * 6, 1, 0, 0);
    }
}


void Chunk::GenerateMeshData() {
    if(!HasAnything) return;

    Chunk* NegX = GApp->m_World->GetChunkManager().GetChunkProvider().GetChunk(glm::ivec3(ChunkX - 1, ChunkY, ChunkZ), LOD);
    Chunk* PosX = GApp->m_World->GetChunkManager().GetChunkProvider().GetChunk(glm::ivec3(ChunkX + 1, ChunkY, ChunkZ), LOD);
    Chunk* NegY = GApp->m_World->GetChunkManager().GetChunkProvider().GetChunk(glm::ivec3(ChunkX, ChunkY - 1, ChunkZ), LOD);
    Chunk* PosY = GApp->m_World->GetChunkManager().GetChunkProvider().GetChunk(glm::ivec3(ChunkX, ChunkY + 1, ChunkZ), LOD);
    Chunk* NegZ = GApp->m_World->GetChunkManager().GetChunkProvider().GetChunk(glm::ivec3(ChunkX, ChunkY, ChunkZ - 1), LOD);
    Chunk* PosZ = GApp->m_World->GetChunkManager().GetChunkProvider().GetChunk(glm::ivec3(ChunkX, ChunkY, ChunkZ + 1), LOD);

    for(int x = 0; x < Chunk_Length; x++) {
        for(int y = 0; y < Chunk_Length; y++) {
            for(int z = 0; z < Chunk_Length; z++) {
                BlockType type = m_Blocks[IndexAt(x, y, z)];
                if(type == BlockType::Air) continue;

                glm::ivec3 blockPos(x, y, z);

                auto IsAir = [&](int dx, int dy, int dz) -> bool {
                    int nx, ny, nz;
                    nx = x + dx, ny = y + dy, nz = z + dz;
                    if (nx < 0) {
                        BlockType b = NegX->m_Blocks[IndexAt(Chunk_Length - 1, ny, nz)];
                        return b == BlockType::Air;
                    }
                    else if (nx >= 32) {
                        BlockType b = PosX->m_Blocks[IndexAt(0, ny, nz)];
                        return b == BlockType::Air;
                    }
                    else if (ny < 0) {
                        BlockType b = NegY->m_Blocks[IndexAt(nx, Chunk_Length - 1, nz)];
                        return b == BlockType::Air;
                    }
                    else if (ny >= 32) {
                        BlockType b = PosY->m_Blocks[IndexAt(nx, 0, nz)];
                        return b == BlockType::Air;
                    }
                    else if (nz < 0) {
                        BlockType b = NegZ->m_Blocks[IndexAt(nx, ny, Chunk_Length - 1)];
                        return b == BlockType::Air;
                    }
                    else if (nz >= 32) {
                        BlockType b = PosZ->m_Blocks[IndexAt(nx, ny, 0)];
                        return b == BlockType::Air;
                    }

                    
                    return m_Blocks[IndexAt(nx, ny, nz)] == BlockType::Air;
                    };
                
                BlockData bd = GApp->BlockRegistery[type];
                if(IsAir(0, 0, -1)) AddFace(blockPos, Face::Back, bd.uvs.backUV, meshData.opaqueFaces);
                if(IsAir(0, 0, 1)) AddFace(blockPos, Face::Front, bd.uvs.frontUV, meshData.opaqueFaces);
                if(IsAir(-1, 0, 0)) AddFace(blockPos, Face::Left, bd.uvs.leftUV, meshData.opaqueFaces);
                if(IsAir(1, 0, 0)) AddFace(blockPos, Face::Right, bd.uvs.rightUV, meshData.opaqueFaces);
                if(IsAir(0, 1, 0)) AddFace(blockPos, Face::Top, bd.uvs.topUV, meshData.opaqueFaces);
                if(IsAir(0, -1, 0)) AddFace(blockPos, Face::Bottom, bd.uvs.bottomUV, meshData.opaqueFaces);
            }
        }
    }
}
void Chunk::UploadMeshData() {
    if(meshData.opaqueFaces.size() > 0) {
        // Upload to GPU
        mesh.opaqueMeshBuffer.Update(meshData.opaqueFaces.data(), meshData.opaqueFaces.size() * sizeof(uint32_t));
        mesh.opaqueCount = meshData.opaqueFaces.size();

        meshData.opaqueFaces.clear();
        std::vector<uint32_t>().swap(meshData.opaqueFaces);

        HasOpaque = true;
    } else HasOpaque = false;
}



void Chunk::DeleteMeshObjects() {
    if(HasOpaque) mesh.opaqueMeshBuffer.Delete();
}


glm::vec3 Chunk::GetMin() {
    return glm::vec3(ChunkX * Chunk_Length * GetLODSize(LOD), 
    ChunkY * Chunk_Length * GetLODSize(LOD), 
    ChunkZ * Chunk_Length * GetLODSize(LOD));
}
glm::vec3 Chunk::GetMax() {
    return glm::vec3(ChunkX * Chunk_Length * GetLODSize(LOD) + Chunk_Length * GetLODSize(LOD), 
                    ChunkY * Chunk_Length * GetLODSize(LOD) + Chunk_Length * GetLODSize(LOD), 
                    ChunkZ * Chunk_Length * GetLODSize(LOD) + Chunk_Length * GetLODSize(LOD));
}
ChunkMeshData& Chunk::GetMeshData() {
    return meshData;
}