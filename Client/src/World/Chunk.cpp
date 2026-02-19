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

uint32_t GetVertex(glm::ivec3 pos, int blockCorner, int texCorner, uint8_t texOffset, uint8_t faceID) {
    uint32_t ret = 0;
    
    ret |= (faceID & 7) << 28;
    ret |= texOffset << 20;
    ret |= texCorner << 18;
    ret |= blockCorner << 15;
    ret |= (pos.y << 10);
    ret |= (pos.z << 5);
    ret |= pos.x;

    return ret;
}
void AddFace(glm::ivec3 pos, Face face, uint32_t& indexOffset, uint8_t texOffset, std::vector<uint32_t>& verticies, std::vector<uint32_t>& indicies) {
    uint32_t v1, v2, v3, v4;
    uint8_t faceID = static_cast<uint8_t>(face);

    switch(face) {
        case Face::Top:
        v1 = GetVertex(pos, 4, 0, texOffset, faceID);
        v2 = GetVertex(pos, 5, 1, texOffset, faceID);
        v3 = GetVertex(pos, 6, 2, texOffset, faceID);
        v4 = GetVertex(pos, 7, 3, texOffset, faceID);
        break;
        case Face::Bottom:
        v1 = GetVertex(pos, 0, 3, texOffset, faceID);
        v2 = GetVertex(pos, 3, 2, texOffset, faceID);
        v3 = GetVertex(pos, 2, 1, texOffset, faceID);
        v4 = GetVertex(pos, 1, 0, texOffset, faceID);
        break;
        case Face::Left:
        v1 = GetVertex(pos, 0, 3, texOffset, faceID);
        v2 = GetVertex(pos, 4, 0, texOffset, faceID);
        v3 = GetVertex(pos, 7, 1, texOffset, faceID);
        v4 = GetVertex(pos, 3, 2, texOffset, faceID);
        break;
        case Face::Right:
        v1 = GetVertex(pos, 1, 2, texOffset, faceID);
        v2 = GetVertex(pos, 2, 3, texOffset, faceID);
        v3 = GetVertex(pos, 6, 0, texOffset, faceID);
        v4 = GetVertex(pos, 5, 1, texOffset, faceID);
        break;
        case Face::Front:
        v1 = GetVertex(pos, 3, 3, texOffset, faceID);
        v2 = GetVertex(pos, 7, 0, texOffset, faceID);
        v3 = GetVertex(pos, 6, 1, texOffset, faceID);
        v4 = GetVertex(pos, 2, 2, texOffset, faceID);
        break; 
        case Face::Back:
        v1 = GetVertex(pos, 0, 2, texOffset, faceID);
        v2 = GetVertex(pos, 1, 3, texOffset, faceID);
        v3 = GetVertex(pos, 5, 0, texOffset, faceID);
        v4 = GetVertex(pos, 4, 1, texOffset, faceID);
        break; 
    }

    verticies.push_back(v1);
    verticies.push_back(v2);
    verticies.push_back(v3);
    verticies.push_back(v4);

    indicies.push_back(indexOffset + 0);
    indicies.push_back(indexOffset + 1);
    indicies.push_back(indexOffset + 2);
    indicies.push_back(indexOffset + 2);
    indicies.push_back(indexOffset + 3);
    indicies.push_back(indexOffset + 0);

    indexOffset += 4;
}


void Chunk::Render() {
    if(HasOpaque) {
        Renderer& renderer = GApp->m_Renderer;

        int LODFactor = GetLODSize(LOD);
        glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(ChunkX * Chunk_Length * LODFactor, ChunkY * Chunk_Length * LODFactor, ChunkZ * Chunk_Length * LODFactor));
        model = glm::scale(model, glm::vec3(static_cast<float>(LODFactor)));
        
        renderer.SetTrans(model);

        VkDeviceSize offset[] = {0};
        vkCmdBindVertexBuffers(renderer.GetFrameCommandBuffer(), 0, 1, &mesh.opaqueMeshBuffer.GetBuffer(), offset);
        vkCmdBindIndexBuffer(renderer.GetFrameCommandBuffer(), mesh.opaqueMeshBuffer.GetBuffer(), mesh.opaqueMeshBuffer.indiciesOffset, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(renderer.GetFrameCommandBuffer(), mesh.opaqueCount, 1, 0, 0, 0);
    }
}


void Chunk::GenerateMeshData() {
    if(!HasAnything) return;

    uint32_t indexOffset = 0;

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
                        BlockType b = neighbors[0]->m_Blocks[IndexAt(Chunk_Length - 1, ny, nz)];
                        return b == BlockType::Air;
                    }
                    else if (nx >= Chunk_Length) {
                        BlockType b = neighbors[1]->m_Blocks[IndexAt(0, ny, nz)];
                        return b == BlockType::Air;
                    }
                    else if (ny < 0) {
                        BlockType b = neighbors[2]->m_Blocks[IndexAt(nx, Chunk_Length - 1, nz)];
                        return b == BlockType::Air;
                    }
                    else if (ny >= Chunk_Length) {
                        BlockType b = neighbors[3]->m_Blocks[IndexAt(nx, 0, nz)];
                        return b == BlockType::Air;
                    }
                    else if (nz < 0) {
                        BlockType b = neighbors[4]->m_Blocks[IndexAt(nx, ny, Chunk_Length - 1)];
                        return b == BlockType::Air;
                    }
                    else if (nz >= Chunk_Length) {
                        BlockType b = neighbors[5]->m_Blocks[IndexAt(nx, ny, 0)];
                        return b == BlockType::Air;
                    }

                    
                    return m_Blocks[IndexAt(nx, ny, nz)] == BlockType::Air;
                    };
                
                BlockData bd = GApp->BlockRegistery[type];
                if(IsAir(0, 0, -1)) AddFace(blockPos, Face::Back, indexOffset, bd.uvs.backUV, meshData.opaqueVerticies, meshData.opaqueIndicies);
                if(IsAir(0, 0, 1)) AddFace(blockPos, Face::Front, indexOffset, bd.uvs.frontUV, meshData.opaqueVerticies, meshData.opaqueIndicies);
                if(IsAir(-1, 0, 0)) AddFace(blockPos, Face::Left, indexOffset, bd.uvs.leftUV, meshData.opaqueVerticies, meshData.opaqueIndicies);
                if(IsAir(1, 0, 0)) AddFace(blockPos, Face::Right, indexOffset, bd.uvs.rightUV, meshData.opaqueVerticies, meshData.opaqueIndicies);
                if(IsAir(0, 1, 0)) AddFace(blockPos, Face::Top, indexOffset, bd.uvs.topUV, meshData.opaqueVerticies, meshData.opaqueIndicies);
                if(IsAir(0, -1, 0)) AddFace(blockPos, Face::Bottom, indexOffset, bd.uvs.bottomUV, meshData.opaqueVerticies, meshData.opaqueIndicies);
            }
        }
    }

    neighbors[0]->referenceCount--;
    neighbors[1]->referenceCount--;
    neighbors[2]->referenceCount--;
    neighbors[3]->referenceCount--;
    neighbors[4]->referenceCount--;
    neighbors[5]->referenceCount--;
}
void Chunk::UploadMeshData() {
    if(meshData.opaqueVerticies.size() > 0) {
        // Upload to GPU
        mesh.opaqueMeshBuffer.Update(meshData.opaqueVerticies.data(), meshData.opaqueVerticies.size() * sizeof(uint32_t), meshData.opaqueIndicies.data(), meshData.opaqueIndicies.size() * sizeof(uint32_t));

        mesh.opaqueCount = meshData.opaqueIndicies.size();

        meshData.opaqueVerticies.clear();
        meshData.opaqueIndicies.clear();
    
        std::vector<uint32_t>().swap(meshData.opaqueVerticies);
        std::vector<uint32_t>().swap(meshData.opaqueIndicies);

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